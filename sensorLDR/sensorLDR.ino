#include <Servo.h>

// Valores del LDR: 0-1023
const int ldrPin = A0;   //Pin del LDR
const int encendido = 600;  //Valor mínimo por el cual movemos el Servo

Servo myservo;
int servoPIN = 2; // d4 en node mcu
int pos = 0;//Posición del myservo
int v;//Lectura del LDR

 
void setup() {
   myservo.attach(servoPIN);//Servo en pin D4
   Serial.begin(9600);
}

// Test Servo only
//void loop(){
//  Serial.println("IN LOOP");
//  myservo.write(90);
//  delay(500);
//  myservo.write(180);
//  delay(500);
//  myservo.write(0);
//  delay(500);
//}
 
void loop() {
  v = analogRead(ldrPin);
  Serial.print("Lectura LDR = "); // Regresa el valor de la intencidad de la luz (0-1024)
  Serial.println(v);
   
  
   if (v > encendido) {
      myservo.write(180);
      // Serial.print("La luz esta encendida.");
      // Serial.println(); // Imprime una línea en blanco en el monitor serial
      // Serial.print("La cortina esta abajo.");
      // Serial.println(); // Imprime una línea en blanco en el monitor serial

      
   }
   else{
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

