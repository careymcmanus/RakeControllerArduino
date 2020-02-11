#include "MotorController.h"
#include <SoftwareSerial.h>

MotorController controller = MotorController(12, 13, 5, 6);


void setup() {
  Serial.begin(9600);
  controller.controllerInit();
  controller.startProgram();
}

void loop() {
  controller.recvWithStartEndMarkers();
  controller.getCommand();
  controller.mainStateLoop();

}


ISR(TIMER1_COMPA_vect) {
  //interrupt commands for TIMER 1 here
  controller.drive();
}
