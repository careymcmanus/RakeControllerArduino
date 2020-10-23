#include "MotorController.h"

/*
* Initialise the MotorController which handles all the logic for the 
* startProgram
*/
SoftwareSerial sSerial(2,3);
MotorController controller = MotorController();

void setup() {
  Serial.begin(9600);
  controller.controllerInit(&sSerial);
  Serial.println("Controller Initiated: ");
}

void loop() {
  controller.main();
}

ISR(TIMER1_COMPA_vect){

  controller.drive();
}

