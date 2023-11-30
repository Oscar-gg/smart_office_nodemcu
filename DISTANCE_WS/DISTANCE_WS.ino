#include "config.h"
#include <Servo.h>
#include <Websockets.h>
#include <ArduinoWebsockets.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

const int trigPin = 12; // Pin para el trigger del sensor ultrasónico
const int echoPin = 14; // Pin para la recepción de eco del sensor ultrasónico
const int ledPin = 5;   // Pin para el LED

#define SOUND_VELOCITY 0.034 // Velocidad del sonido en centímetros por microsegundo

long duration;    // Variable para almacenar la duración del eco
float distanceCm; // Variable para almacenar la distancia en centímetros

bool isLedOn = false;       // Variable para rastrear si el LED está encendido
unsigned long ledStartTime; // Variable para rastrear el tiempo de inicio del LED

using namespace websockets;

Websockets wsClient(config::websockets_connection_string);

const long int CHECK_DISTANCE_INTERVAL = 1000; // ms
unsigned long int lastTime = 0;

long int workTime = 0; // ms
const int distanceWorkingTime = 4;

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
    else if (action == "startWorkTime")
    {
        // Read open property
        workTime = 0;
    }
    else if (action == "endWorkTime")
    {
        String sessionId = doc["id"].as<String>();

        float ledElapsedTimeHours = (float)workTime / 3600000.0;
        wsClient.sendResponse(ledElapsedTimeHours, "workTime", sessionId);

        workTime = 0;
    }
    else
    {
        Serial.print("Warning: the recieved action has no implementation: ");
        Serial.println(action);
    }
}

void setup()
{
    Serial.begin(config::serialBaud); // Inicializar la comunicación serial
    while (!Serial)
    {
    }

    delay(2000);
    Serial.println("Serial communication initialized.");

    pinMode(trigPin, OUTPUT); // Establecer el pin del trigger como salida
    pinMode(echoPin, INPUT);  // Establecer el pin del echo como entrada
    pinMode(ledPin, OUTPUT);  // Establecer el pin del LED como salida

    Serial.println("Ultrasonic initialized.");

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
    wsClient.initialize("ultrasonic", "ultrasoniczone1");

    // Send a ping
    wsClient.ping();

    Serial.println("Setup has been finished");
}

float getDistance()
{
    digitalWrite(trigPin, LOW);  // Enviar un pulso bajo al pin del trigger
    delayMicroseconds(2);        // Esperar 2 microsegundos
    digitalWrite(trigPin, HIGH); // Enviar un pulso alto al pin del trigger
    delayMicroseconds(10);       // Esperar 10 microsegundos
    digitalWrite(trigPin, LOW);  // Resetear el pin del trigger

    duration = pulseIn(echoPin, HIGH);          // Medir la duración del eco
    distanceCm = duration * SOUND_VELOCITY / 2; // Calcular la distancia en centímetros
    return distanceCm;
}

void loop()
{
    // Listen for events
    wsClient.poll();

    if (millis() - lastTime > CHECK_DISTANCE_INTERVAL)
    {
        lastTime = millis();

        float distanceCm = getDistance();
        Serial.print("Distancia (cm): ");
        Serial.println(distanceCm); // Imprimir la distancia en centímetros en el puerto serie

        if (distanceCm <= distanceWorkingTime)
        {
            workTime += CHECK_DISTANCE_INTERVAL; // Sumar 1 segundo
            if (!isLedOn)
            {
                // Encender el LED y registrar el tiempo de inicio
                digitalWrite(ledPin, HIGH);

                isLedOn = true;
                ledStartTime = millis();
                Serial.println("Prender LED");
            }
        }
        else
        {
            if (isLedOn)
            {
                // Apagar el LED y calcular el tiempo transcurrido
                digitalWrite(ledPin, LOW);
                isLedOn = false;
                unsigned long ledEndTime = millis();
                unsigned long ledElapsedTime = ledEndTime - ledStartTime;

                // Convertir el tiempo en milisegundos a horas
                float ledElapsedTimeHours = (float)ledElapsedTime / 3600000.0;

                Serial.print("Tiempo del LED encendido (horas): ");
                Serial.println(ledElapsedTimeHours, 2); // Mostrar el tiempo en horas con 2 decimales
            }
        }
    }
}
