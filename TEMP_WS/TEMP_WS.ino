#include "DHT.h"
#include "config.h"
#include "Websockets.h"
#include <Servo.h>
#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>

#define DHTTYPE DHT11
#define dht_dpin 0
DHT dht(dht_dpin, DHTTYPE);

#define RED_LED 14    // los pines al que esta conectado, D5
#define GREEN_LED 12  // D6
#define YELLOW_LED 13 // D7

const long int SEND_TIME_INTERVAL = 30000;
unsigned long int lastTime = 0;

float upperBound = 24.60;
float lowerBound = 18.0;

using namespace websockets;
Websockets wsClient;

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
  else if (action == "setPreferences")
  {
    // Read open property
    float _lowerBound = doc["lowerBound"].as<float>();
    float _upperBound = doc["upperBound"].as<float>();

    Serial.print("Value recieved for lower and upper bound: ");
    Serial.print(_lowerBound);
    Serial.print(", ");
    Serial.println(_upperBound);

    if (_lowerBound != 0)
    {
      lowerBound = _lowerBound;
    }
    if (_upperBound != 0)
    {
      upperBound = _upperBound;
    }
  }
  else if (action == "getHumidity")
  {
    wsClient.sendFloatResponse(getHumidity(), "humidity");
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

  dht.begin();
  Serial.println("Humidity and temperature sensor initialized.");

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);

  Serial.println("Initalized leds");

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
  wsClient.initialize("temperature", "temperaturezone1");

  // Send a ping
  wsClient.ping();

  Serial.println("Setup has been finished");
}

float getHumidity()
{
  return dht.readHumidity();
}

float getTemp()
{
  return dht.readTemperature();
}

String getStatus()
{
  float temp = getTemp();
  float humidty = getHumidity();
  if (!isnan(temp) && !isnan(humidty))
  {
    if (temp >= upperBound)
    {
      return "Calor";
    }
    else if (temp >= lowerBound && temp < upperBound)
    {
      return "Ideal";
    }
    else
    {
      return "Frio";
    }
  }
  else
  {
    return "Error";
  }
}

void ledsStatus(String status)
{
  if (status == "Calor")
  {
    // hace calor
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
  }
  else if (status == "Ideal")
  {
    // esta perf
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
  }
  else if (status == "Frio")
  {
    // hace frio
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
  }
  else if (status == "Error")
  {
    // error, apaga todo
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
  }
  else
  {
    Serial.print("Status no soportado: ");
    Serial.println(status);
  }
}

void logData()
{
  float t = getTemp();
  float h = getHumidity();
  String status = getStatus();
  ledsStatus(status);
  Serial.println("Status = ");
  Serial.print(status);
  Serial.print(", temperature = ");
  Serial.print(t);
  Serial.print(", humidity = ");
  Serial.println(h);
}

void loop()
{
  // Listen for events
  wsClient.poll();
  delay(200);
  logData();

  if (millis() - lastTime > SEND_TIME_INTERVAL)
  {
    lastTime = millis();
    wsClient.sendFloatResponse(getTemp(), "temperature");
  }
}
