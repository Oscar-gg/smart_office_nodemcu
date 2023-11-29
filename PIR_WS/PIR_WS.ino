#include "config.h"
#include <Servo.h>
#include <Websockets.h>
#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

using namespace websockets;

Websockets wsClient(config::websockets_connection_string);

const int pirPin = D1; // Pin al que está conectado el sensor PIR
const int ledPin = D2; // Pin al que está conectado el LED para indicar movimiento

// Dont send data if less time has passed
const long int SEND_TIME_INTERVAL = 30000;
unsigned long int lastAlarmSent = 0;

// Execute when recieving a message
void onMessageCallback(WebsocketsMessage message)
{
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
  else if (action == "E")
  {
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

  pinMode(pirPin, INPUT);  // declare sensor as input
  pinMode(ledPin, OUTPUT); // declare LED as output

  Serial.println("PIR and LED initialized.");

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
  wsClient.initialize("Alarm", "alarmzone1");

  // Send a ping
  wsClient.ping();

  Serial.println("Setup has been finished");
}

void loop()
{
  // Listen for events
  wsClient.poll();

  long state = digitalRead(pirPin);
  if (state == HIGH)
  {
    digitalWrite(ledPin, HIGH);
    Serial.println("Motion detected!");
    delay(1000);

    // Update status if more than 30s have passed
    if (millis() - lastAlarmSent < SEND_TIME_INTERVAL)
    {
      return;
    }
    wsClient.sendResponse("triggered", "movement");
    lastAlarmSent = millis();
  }
  else
  {
    digitalWrite(ledPin, LOW);
    Serial.println("Motion absent!");
    delay(1000);
  }
}
