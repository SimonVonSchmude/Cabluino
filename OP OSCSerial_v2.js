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

    // sendTrigger = op.inTriggerButton("Send"),
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
// sendTrigger.onTriggered = send;
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
    if (autoConnect) {
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

let decodeMessage = function (msg) {
    if (sending) { return; }

    if (msg == 170) {
        send();
        return;
    }

    const uint8Array = new Uint8Array(msg);
    let oscMessage, jsonMessage;

    try {
        oscMessage = osc.readBundle(uint8Array, 0);
    }
    catch (err) {
        // op.log("SLIP message couldn't be converted to OSC: ", err);
        return;
    }
    try {
        jsonMessage = convertToJson(oscMessage);
        outObj.setRef(jsonMessage);
    }
    catch (err) {
        // op.log("OSC message couldn't be converted to JSON: ", err);
        return;
    }
};

let logError = function (msg) {
    op.log("error: " + msg);
};

let decoder = new slip.Decoder({
    "onMessage": decodeMessage,
    "onError": logError,
    "maxMessageSize": 209715200,
    "bufferSize": 2048
});

// –––––––––

async function connect(isNewPort, target = null) {
    if (connected) {
        op.log("Already connected");
        return;
    }

    if (!("serial" in navigator)) {
        op.log("This only works in Chrome!");
        return;
    }

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
        op.log("Connection failed:", err);
        setConnectState(false);
        return;
    }

    port.addEventListener("disconnect", (event) => {
        op.log("Your board disconnected!");
        op.log("Closing Port...");
        closePort();
    });

    setConnectState(true);
    op.log("Connected to port!");

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
            op.log("Failed to read data:", err);
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
            op.log("Error canceling reader:", err);
        }
    }

    if (port) {
        try {
            await port.close();
            port = null; // Set to null to avoid reusing an old port
            setConnectState(false);
            op.log("Closed port!");
        }
        catch (err) {
            op.log("Error closing port:", err);
        }
    }
}

async function forgetAllPorts() {
    await closePort();

    const ports = await navigator.serial.getPorts();

    if (ports.length === 0) {
        op.log("No ports to forget!");
        return;
    }

    try {
        for (const port of ports) {
            await port.forget();
        }

        op.log("Forgot all ports!");
    }
    catch (err) {
        op.log("Error forgetting ports:", err);
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
        op.log("Error while sending message:", err);
    }
    finally {
        writer.releaseLock();
        writerLocked = false;

        sending = false;
    }
}

function inferType(value) {
    if (Number.isInteger(value)) {
        return "i";
    }
    else if (typeof value === "number") {
        return "f";
    }
    else if (typeof value === "string") {
        return "s";
    }
    op.setUiError("", "Unsupported value type in input object! Must be integer, float or string.");
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
