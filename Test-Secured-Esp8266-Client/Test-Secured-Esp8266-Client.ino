// Client code for AWS IoT Websockets

#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

const char *ssid = "";     // Enter SSID
const char *password = ""; // Enter Password


// Enter server adress
const char *websockets_connection_string = "wss://15yzzg78gk.execute-api.us-east-1.amazonaws.com/development/";

using namespace websockets;

WebsocketsClient client;

// Send data to server in JSON format
void serializeTest()
{
    DynamicJsonDocument doc(1024);

    doc["sensor"] = "gps";
    doc["time"] = 1351824120;
    doc["data"][0] = 48.756080;
    doc["data"][1] = 2.302038;

    char output[200];
    serializeJson(doc, output);
    Serial.println(output);
    client.send(output);
}

// Function to specify the type of device to the server
// Used by the server to identify the device
void setClientType()
{
}

// Execute when recieving a message
void onMessageCallback(WebsocketsMessage message)
{
    Serial.print("Got Message: ");
    Serial.println(message.data());

    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, message.data());

    // Checar si hubo un error
    if (err)
    {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.c_str());
    }
    else
    {
        // Imprimir el mensaje
        serializeJson(doc, Serial);
    }

    const char *action = doc["action"];

    Serial.print("\nAction: ");
    Serial.println(action);
}

// Listen for events and print to debug status
void onEventsCallback(WebsocketsEvent event, String data)
{
    if (event == WebsocketsEvent::ConnectionOpened)
    {
        Serial.println("Connnection Opened");
    }
    else if (event == WebsocketsEvent::ConnectionClosed)
    {
        Serial.println("Connnection Closed");
    }
    else if (event == WebsocketsEvent::GotPing)
    {
        Serial.println("Got a Ping!");
    }
    else if (event == WebsocketsEvent::GotPong)
    {
        Serial.println("Got a Pong!");
    }
}

void setup()
{
    Serial.begin(115200);

    // Connect to wifi
    WiFi.begin(ssid, password);

    // Wait until device iss connected
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print("Wifi not connected. Status: ");
        Serial.println(WiFi.status());
        delay(500);
    }

    Serial.println("Wifi has been connected");

    // run callback when messages are received
    client.onMessage(onMessageCallback);

    // run callback when events are occuring
    client.onEvent(onEventsCallback);

    // Connect to server
    client.connect(websockets_connection_string);

    setClientType();

    // Send a ping
    client.ping();
}

void loop()
{
    // Listen for events
    client.poll();
}
