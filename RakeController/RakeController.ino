#include "MotorController.h"
#include "initPrms.h"
/*
* Initialise the Command Processor which handles
* Sending and recieving of messages over bluetooth
* module.
*/
SoftwareSerial ss(2,3);
CmdProc cP = CmdProc(&ss, 9600);
uint8_t btnFlags = 0x00;
BtnProc bP = BtnProc(btnSet);
/*
* Initialise the MotorController which handles all the logic for the 
* startProgram
*/
MotorController controller = MotorController(&drvPins, &bP, &cP, prgms);

void setup() {
  Serial.begin(19200);
  Serial.println("Setup:");
  controller.controllerInit();
  Serial.println("Controller Initiated: ");
  //controller.startProgram();
}

void loop() {
  controller.main();
}

