/*
This is very unoptimized and needs quite some fixes and cleanups!

by Simon von Schmude, 2024

Using:
slip.js -> https://github.com/colinbdclark/slip.js/
osc.js -> https://github.com/colinbdclark/osc.js/
*/

const
    baudRate = op.inValueSelect("Baudrate", [110, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 57600, 115200, 128000, 256000], 115200),
    // SLIPbufferSize = op.inInt("SLIP Buffer Size", 2048),

    connectNewTrigger = op.inTriggerButton("Connect to new device"),
    connectLastTrigger = op.inTriggerButton("Connect to previous device"),
    autoConnect = op.inValueBool("Reconnect automatically", false),

    sendObject = op.inObject("Data"),

    closeTrigger = op.inTriggerButton("Close"),
    forgetTrigger = op.inTriggerButton("Forget all devices"),

    outObj = op.outObject("Received Data"),
    connectState = op.outBoolNum("Connected", false),

    lastSentSize = op.outNumber("Size of last message sent");

let port, writer, reader, connected;
let keepReading = true;
let writerLocked = false;

let sending = false;

// SLIPbufferSize.onChange = closePort;
baudRate.onChange = closePort;
connectNewTrigger.onTriggered = newPort;
connectLastTrigger.onTriggered = lastPort;
closeTrigger.onTriggered = closePort;
forgetTrigger.onTriggered = forgetAllPorts;

function setConnectState(state) {
    connected = state;
    connectState.set(state);
}

function newPort() {
    connect(true);
}

function lastPort() {
    connect(false);
}

// –––––––––

navigator.serial.addEventListener("connect", (event) => {
    if (autoConnect.get()) {
        connect(false, event.target);
    }
});

function convertToJson(oscMessage) {
    const result = {};

    oscMessage.packets.forEach((packet) => {
        let value = packet.args[0]; // Assuming args always contain one value.
        let addressParts = packet.address.split("/").filter((part) => { return part !== ""; });
        let current = result;

        for (let i = 0; i < addressParts.length - 1; i++) {
            if (!current[addressParts[i]]) {
                current[addressParts[i]] = {};
            }
            current = current[addressParts[i]];
        }

        current[addressParts[addressParts.length - 1]] = value;
    });

    return result;
}

let logMessage = function (msg) {
    if (sending) { return; }

    if (msg == 170) {
        send();
        return;
    }

    // op.log("A SLIP message was received! Here is it: " + msg);
    // const uint8Array = new Uint8Array([47, 112, 111, 116, 0, 0, 0, 0, 44, 105, 0, 0, 0, 0, 3, 255]);

    const uint8Array = new Uint8Array(msg);
    let oscMessage, jsonMessage;

    try {
        oscMessage = osc.readBundle(uint8Array, 0);
    }
    catch (err) {
        // op.log("SLIP message couldn't be converted to OSC: ", err);
    }
    try {
        jsonMessage = convertToJson(oscMessage);
        outObj.setRef(jsonMessage);
    }
    catch (err) {
        // op.log("OSC message couldn't be converted to JSON: ", err);
    }
};

let logError = function (msg) {
    op.log("error: " + msg);
};

let decoder = new slip.Decoder({
    "onMessage": logMessage,
    "onError": logError,
    "maxMessageSize": 209715200,
    "bufferSize": 2048
});

// –––––––––

async function connect(isNewPort, target = null) {
    if (connected) {
        op.setUiError("connected", "Already connected", 1);
        //op.log("Already connected");
        return;
    }

    op.setUiError("connected", null);

    if (!("serial" in navigator)) {
        op.setUiError("chrome", "WebSerial only works in Chrome!", 2);
        //op.log("WebSerial only works in Chrome!");
        return;
    }

    op.setUiError("chrome", null);

    try {
        if (isNewPort) {
            port = await navigator.serial.requestPort();
        }
        else {
            const ports = await navigator.serial.getPorts();
            if (ports.length === 0) {
                throw "No saved ports available!";
            }
            if (target == null) {
                port = ports[ports.length - 1];
            }
            else {
                port = target;
            }
        }

        await port.open({ "baudRate": baudRate.get() });
    }
    catch (err) {
        // op.log("Connection failed: ", err);
        const errorMsg = "Connection failed: ".concat(err);
        op.setUiError("connection", errorMsg, 1);
        setConnectState(false);
        return;
    }

    op.setUiError("connection", null);

    port.addEventListener("disconnect", (event) => {
        // op.setUiError("disconnected", "Your Board disconnected, closing port...", 1);

        // op.log("Your board disconnected!");
        // op.log("Closing Port...");
        closePort();
    });

    setConnectState(true);

    op.setUiError("closedPort", null);
    op.setUiError("noPorts", null);
    op.setUiError("forgotPorts", null);
    op.setUiError("connectedPort", "Connected to port!", 0);
    //op.log("Connected to port!");

    keepReading = true;
    readUntilClosed();
}

async function readUntilClosed() {
    while (port.readable && keepReading) {
        reader = port.readable.getReader();
        try {
            while (true) {
                const { value, done } = await reader.read();
                if (done) {
                    // |reader| has been canceled.
                    break;
                }
                // Do something with |value|…
                decoder.decode(value);
            }
        }
        catch (err) {
            op.log("Failed to read data: ", err);
        }
        finally {
            reader.releaseLock();
        }
    }
}

async function closePort() {
    if (!connected) return;

    keepReading = false;

    if (reader) {
        try {
            await reader.cancel();
            reader.releaseLock();
            reader = null; // Set to null to avoid reusing an old reader
        }
        catch (err) {
            op.log("Error canceling reader: ", err);
        }
    }

    if (port) {
        try {
            await port.close();
            port = null; // Set to null to avoid reusing an old port
            setConnectState(false);
            op.setUiError("connectedPort", null);
            op.setUiError("noPorts", null);
            op.setUiError("forgotPorts", null);
            op.setUiError("closedPort", "Closed port!", 0);
            // op.log("Closed port!");
        }
        catch (err) {
            op.setUiError("closedPort", "Error closing port", 1);
            // op.log("Error closing port: ", err);
        }
    }
}

async function forgetAllPorts() {
    await closePort();

    const ports = await navigator.serial.getPorts();

    if (ports.length === 0) {
        op.setUiError("noPorts", "No ports to forget!", 1);
        //        op.log("No ports to forget!");
        return;
    }

    try {
        for (const port of ports) {
            await port.forget();
        }

        op.setUiError("forgotPorts", "Forgot all ports!", 0);
        // op.log("Forgot all ports!");
    }
    catch (err) {
        op.log("Error forgetting ports: ", err);
    }
}

async function send() {
    sending = true;

    if (!connected || writerLocked) { return; }

    try {
        const transformedJson = transformJsonToOsc(sendObject.get());
        const oscPacket = osc.writePacket(transformedJson);
        const slipEncoded = slip.encode(oscPacket);

        lastSentSize.set(slipEncoded.length);

        writer = port.writable.getWriter();
        writerLocked = true;
        await writer.write(slipEncoded);
    }
    catch (err) {
        // op.setUiError("sending", "Error while sending message!", 2);
        // op.log("Error while sending message: ", err);
    }
    finally {
        writer.releaseLock();
        writerLocked = false;

        sending = false;
    }
}

function inferType(value) {
    if (Number.isInteger(value)) {
        op.setUiError("dataType", null);
        return "i";
    }
    else if (typeof value === "number") {
        op.setUiError("dataType", null);
        return "f";
    }
    else if (typeof value === "string") {
        op.setUiError("dataType", null);
        return "s";
    }
    op.setUiError("dataType", "Unsupported value type in input object! Must be integer, float or string.", 1);
}

function transformJsonToOsc(json) {
    const packets = Object.keys(json).map((key) => {
        const value = json[key];

        // Check if the value is an array
        if (Array.isArray(value)) {
            return {
                "address": `/${key}`,
                "args": value.map((item) => {
                    return {
                        "type": inferType(item),
                        "value": item
                    };
                })
            };
        }

        // Handle individual value
        return {
            "address": `/${key}`,
            "args": [
                {
                    "type": inferType(value),
                    "value": value
                }
            ]
        };
    });

    return {
        "timeTag": osc.timeTag(0),
        "packets": packets
    };
}
