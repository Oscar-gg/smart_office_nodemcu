#include "config.h"
#include <Websockets.h>
#include <Servo.h>
#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

// Valores del LDR: 0-1023
const int ldrPin = A0;     // Pin del LDR
const int encendido = 600; // Valor mínimo por el cual movemos el Servo

Servo myservo;
int servoPIN = 2; // d4 en node mcu
int pos = 0;      // Posición del myservo
int v;            // Lectura del LDR

using namespace websockets;
Websockets wsClient(config::websockets_connection_string);

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
    else if (action == "getLight")
    {
        String encendido = isLightOn();
        String sessionId = doc["id"].as<String>() | "";

        wsClient.sendResponse(encendido, "light", sessionId);
    }
    else
    {
        Serial.print("Warning: the recieved action has no implementation: ");
        Serial.println(action);
    }
}

void setup()
{

    Serial.begin(config::serialBaud);

    while (!Serial)
    {
    }

    delay(2000);

    Serial.println("Serial communication initialized.");

    myservo.attach(servoPIN); // Servo en pin D4
    Serial.println("Servo initialized.");

    // Connect to wifi
    WiFi.begin(config::ssid, config::password);

    // Wait until device is connected
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print("Wifi not connected. Status: ");
        Serial.println(WiFi.status());
        delay(500);
    }

    Serial.println("Wifi has been connected");

    // run callback when messages are received
    wsClient.onMessage(onMessageCallback);
    wsClient.initialize("light", "lightzone1");

    // Send a ping
    wsClient.ping();

    Serial.println("Setup has been finished");
}

// Test Servo only
// void loop(){
//  Serial.println("IN LOOP");
//  myservo.write(90);
//  delay(500);
//  myservo.write(180);
//  delay(500);
//  myservo.write(0);
//  delay(500);
//}

String isLightOn(){
    int light = getLight();
    Serial.print("Light level: ");
    Serial.println(light);
    if (light > encendido){
        return "Encendido";
    } else {
        return "Apagado";
    }
}

int getLight(){
    return analogRead(ldrPin);
}

void loop()
{
    // Listen for events
    wsClient.poll();

    v = analogRead(ldrPin);
    Serial.print("Lectura LDR = "); // Regresa el valor de la intencidad de la luz (0-1024)
    Serial.println(v);


    if (v > encendido)
    {
        myservo.write(180);
        // Serial.print("La luz esta encendida.");
        // Serial.println(); // Imprime una línea en blanco en el monitor serial
        // Serial.print("La cortina esta abajo.");
        // Serial.println(); // Imprime una línea en blanco en el monitor serial
    }
    else
    {
        myservo.write(-180);
        // Serial.print("La luz esta apagada.");
        // Serial.println(); // Imprime una línea en blanco en el monitor serial
        // Serial.print("La cortina esta arriba.");
        // Serial.println(); // Imprime una línea en blanco en el monitor serial
    }
    //  Serial.println(); // Imprime una línea en blanco en el monitor serial
    //  Serial.println(); // Imprime una línea en blanco en el monitor serial

    delay(1000);
}
