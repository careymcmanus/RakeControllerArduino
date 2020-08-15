# 1 "c:\\Users\\carey\\Documents\\CMConsulting\\SouthSalt\\RakeControllerArduino\\RakeController\\RakeController.ino"
# 2 "c:\\Users\\carey\\Documents\\CMConsulting\\SouthSalt\\RakeControllerArduino\\RakeController\\RakeController.ino" 2

/*

* Initialise the MotorController which handles all the logic for the 

* startProgram

*/
# 7 "c:\\Users\\carey\\Documents\\CMConsulting\\SouthSalt\\RakeControllerArduino\\RakeController\\RakeController.ino"
SoftwareSerial sSerial(2,3);
MotorController controller = MotorController();

void setup() {
  Serial.begin(115200);
  sSerial.begin(9600);
  controller.controllerInit(&sSerial);
  Serial.println("Controller Initiated: ");
}

void loop() {
  controller.main();
}
