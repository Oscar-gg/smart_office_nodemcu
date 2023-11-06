/* Client code for RFID sensor

Sends: codes interpreted by RFID sensor, only when the sensor has a lecture.

Recieves: whether or not to open the servo, which represents opening the door.

The NodeMCU doesn't make the decision of opening the servo locally as more specific logic
can be implemented using data found in the database.

*/

#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
#include "Servo.h"

// Define pins
#define RST_PIN D3
#define SS_PIN D4
#define SERVO_PIN 4

MFRC522 reader(SS_PIN, RST_PIN);

// Define servo to open the door.
Servo servo;

// Control variable for servo
const int openTime = 5000; // Close servo after the time has passed.
const int openAngle = -180;
const int closeAngle = 180;

const char *ssid = "";     // Enter SSID
const char *password = ""; // Enter Password

// Enter server adress
const char *websockets_connection_string = "";

using namespace websockets;

WebsocketsClient client;

// Function to specify the type of device to the server
// Used by the server to identify the device
// The server matches the client's id with the specified type
void setClientType()
{
    DynamicJsonDocument doc(1024);

    doc["action"] = "deviceType"; // Defines endpoint to call
    doc["type"] = "RFID";         // Used to identify several devices with the same purpose
    doc["name"] = "rfidzone1";    // Unique, used to identify devices of the same type.

    char output[200];
    serializeJson(doc, output);
    client.send(output);
}

void sendRFIDResponse(String lecture)
{
    Serial.print("Detected lecture: ");
    Serial.println(lecture);
    
    DynamicJsonDocument doc(1024);

    doc["action"] = "recieveData";
    doc["dataType"] = "RFID";
    doc["data"] = lecture;

    char output[200];
    serializeJson(doc, output);
    client.send(output);
}

// Execute when recieving a message
void onMessageCallback(WebsocketsMessage message)
{
    // Serial.print("Got Message: ");
    // Serial.println(message.data());

    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, message.data());

    // Checar si hubo un error
    if (err)
    {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(err.c_str());
        Serial.println("Aborted action");
        return;
    }
    else
    {
        // Imprimir el mensaje
        serializeJson(doc, Serial);
        Serial.println();
    }

    String action = doc["action"].as<String>();

    Serial.print("Action: ");
    Serial.println(action);

    if (action == "")
    {
        Serial.println("Error: no action was detected.");
        Serial.println("Aborted action");
    }
    else if (action == "servo")
    {
        // Read open property
        int open = doc["open"].as<int>() | 0;
        Serial.print("Value recieved for open property:");
        Serial.println(open);

        if (open == 1)
        {
            openServo(openTime);
        }
    }
    else
    {
        Serial.print("Warning: the recieved action has no implementation: ");
        Serial.println(action);
    }
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
    
    while (!Serial)
    {
    }

    delay(2000);

    Serial.println("Serial communication initialized.");

    servo.attach(SERVO_PIN);
    servo.write(closeAngle);

    Serial.println("Servo initialized.");

    SPI.begin();

    // Initialize MFRC522
    reader.PCD_Init();

    Serial.println("Sensor and actuator initialization done.");

    // Connect to wifi
    WiFi.begin(ssid, password);

    // Wait until device is connected
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

    // Set up with the server
    setClientType();

    // Send a ping
    client.ping();

    Serial.println("Setup has been finished");
}

void loop()
{

    // Listen for events
    client.poll();

    // RFID code:

    if (!reader.PICC_IsNewCardPresent())
    {
        return;
    }

    // Check if lecture was successful, else exit loop
    if (!reader.PICC_ReadCardSerial())
    {
        return;
    }

    Serial.println("RFID readed card successfuly");

    String reading = "";
    for (int x = 0; x < reader.uid.size; x++)
    {
        // If it is less than 10, we add zero
        if (reader.uid.uidByte[x] < 0x10)
        {
            reading += "0";
        }
        // Convert lecture from byte to hexadecimal
        reading += String(reader.uid.uidByte[x], HEX);

        // Separate bytes with dashes.
        if (x + 1 != reader.uid.size)
        {
            reading += "-";
        }
    }
    // Make string uppercase (for formatting)
    reading.toUpperCase();

    // Send lecture to server
    sendRFIDResponse(reading);
    delay(5000); // Wait to avoid sending lots of requests.
}

// Open servo, considering the specified variables
void openServo(int duration)
{
    servo.write(openAngle);
    delay(duration);
    servo.write(closeAngle);
}
