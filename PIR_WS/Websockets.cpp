#include "Websockets.h"

Websockets::Websockets()
{
}

void Websockets::initialize(String type, String name)
{
    // Connect to server
    client.connect(config::websockets_connection_string);
    client.onEvent(onEventsCallback);
    setClientType(type, name);
}

// Function to specify the type of device to the server
// Used by the server to identify the device
// The server matches the client's id with the specified type
void Websockets::setClientType(String type, String name)
{
    DynamicJsonDocument doc(1024);

    doc["action"] = "deviceType"; // Defines endpoint to call
    doc["type"] = type;           // Used to identify several devices with the same purpose
    doc["name"] = name;           // Unique, used to identify devices of the same type.

    char output[200];
    serializeJson(doc, output);
    client.send(output);
}

// Listen for events and print to debug status

void Websockets::ping()
{
    client.ping();
}
void Websockets::poll()
{
    client.poll();
}

void Websockets::sendStringResponse(String data, String dataType)
{
    Serial.print("Sending respnse:");
    Serial.println(data);
    DynamicJsonDocument doc(1024);

    doc["action"] = "recieveData";
    doc["dataType"] = dataType;
    doc["data"] = data;

    char output[200];
    serializeJson(doc, output);
    client.send(output);
}

void Websockets::sendIntResponse(int data, String dataType)
{
    Serial.print("Sending respnse:");
    Serial.println(data);
    DynamicJsonDocument doc(1024);

    doc["action"] = "recieveData";
    doc["dataType"] = dataType;
    doc["data"] = data;

    char output[200];
    serializeJson(doc, output);
    client.send(output);
}

void Websockets::sendFloatResponse(float data, String dataType)
{
    Serial.print("Sending respnse:");
    Serial.println(data);
    DynamicJsonDocument doc(1024);

    doc["action"] = "recieveData";
    doc["dataType"] = dataType;
    doc["data"] = data;

    char output[200];
    serializeJson(doc, output);
    client.send(output);
}

void Websockets::onMessage(const MessageCallback callback)
{
    this->client.onMessage(callback);
}
void Websockets::onMessage(const PartialMessageCallback callback)
{
    this->client.onMessage(callback);
}