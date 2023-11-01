// Código lecturas de etiquetas RFID y controlar un servomotor, simulando el control de una puerta.

// Librerías a utilizar
#include <SPI.h>
#include <MFRC522.h>
#include "Servo.h"


// Definir pines
#define RST_PIN D3
#define SS_PIN D4
#define SERVO_PIN 4

// Crear objeto de tiopo MFRC522, para hacer las lecturas con la librería
MFRC522 reader(SS_PIN, RST_PIN);

// Crear objeto tipo servo.
Servo servo;

// Lista de direcciones registradas, usadas para comprobar que la tarjeta es válida
// En la aplicación final, los usuarios podrían ser modificados desde alguna interface.
String userCode[2] = {"BA-08-11-20", "FF-C4-DA-34"};
const int userQuantity = 2;

// Variables para controlar servo.
const int openTime = 5000; // Cerrar el servo después de esta cantidad de tiempo
const int openAngle = -180;
const int closeAngle = 180;

void setup()
{
  // Inicializar conexión serial y esperar hasta que la comunicación sea exitosa.
  Serial.begin(9600);

  // Esperar a que el serial esté disponible.
  while (!Serial)
  {
  }
  Serial.println("Comunicación serial inicializada.");
  
  servo.attach(SERVO_PIN);
  servo.write(closeAngle);
  
  Serial.println("Servo inicializado.");
  
  SPI.begin();

  // Inicializar chip de MFRC522
  reader.PCD_Init();
  delay(4); // Esperar 4 segundos después de inicializar.

  Serial.println("Setup finalizado");
}

void loop()
{
  // Checar si se detectó una nueva tarjeta. En caso de que no, reiniciar el loop.
  if (!reader.PICC_IsNewCardPresent())
  {
    return;
  }
  
  // Checar que la lectura haya sido exitosa. Si no fue exitosa reiniciar el loop.
  if (!reader.PICC_ReadCardSerial())
  {
    return;
  }

  Serial.println("Lectura de tarjeta exitosa");

  // Leer el serial para obtener la detección

  String reading = "";
  for (int x = 0; x < reader.uid.size; x++)
  {
    // If it is less than 10, we add zero
    if (reader.uid.uidByte[x] < 0x10)
    {
      reading += "0";
    }
    // Pasar lectura de byter a hexadecimal
    reading += String(reader.uid.uidByte[x], HEX);
    
    // Separar bytes leídos con guiones.
    if (x + 1 != reader.uid.size)
    {
      reading += "-";
    }
  }
  // Hacer todo el string a mayúsculas, dar formato a entradas.
  reading.toUpperCase();

  Serial.println("Lectura: " + reading); // Imprimir lectura

  // Validar lectura y mover actuador en función a eso.
  if (validReading(reading)) {
    Serial.println("Tarjeta aprobada, abriendo puerta.");
    openServo(openTime); // Abrir servo
  } else {
    Serial.println("Tarjeta no reconocida, registre la tarjeta.");
  }

  // Salir del estado de autenticación de lectura.
  // Usado para que se puedan establecer nuevas detecciones con el sensor.
  reader.PICC_HaltA();
  reader.PCD_StopCrypto1();

}

// Regresa verdadero si el usuario se encuentra registrado
bool validReading(String reading){
  for(int i = 0; i < userQuantity; i++){
    if (reading == userCode[i])
      return true;
  }

  return false; // Si llega a este punto fue porque no se encontró el usuario
}

// Abre el servo por la cantidad de tiempo especificada
void openServo(int duration){
  
  servo.write(openAngle);
  delay(duration);
  servo.write(closeAngle); 
}
