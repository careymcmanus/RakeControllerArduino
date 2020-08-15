#include <Arduino.h>
#line 1 "c:\\Users\\carey\\Documents\\CMConsulting\\SouthSalt\\RakeControllerArduino\\RakeController\\RakeController.ino"
#include "MotorController.h"

/*
* Initialise the MotorController which handles all the logic for the 
* startProgram
*/
SoftwareSerial sSerial(2,3);
MotorController controller = MotorController();

#line 10 "c:\\Users\\carey\\Documents\\CMConsulting\\SouthSalt\\RakeControllerArduino\\RakeController\\RakeController.ino"
void setup();
#line 17 "c:\\Users\\carey\\Documents\\CMConsulting\\SouthSalt\\RakeControllerArduino\\RakeController\\RakeController.ino"
void loop();
#line 10 "c:\\Users\\carey\\Documents\\CMConsulting\\SouthSalt\\RakeControllerArduino\\RakeController\\RakeController.ino"
void setup() {
  Serial.begin(115200);
  sSerial.begin(9600);
  controller.controllerInit(&sSerial);
  Serial.println("Controller Initiated: ");
}

void loop() {
  controller.main();
}

