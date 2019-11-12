[1mdiff --git a/RakeController/MotorController.cpp b/RakeController/MotorController.cpp[m
[1mindex 613ef71..0285238 100644[m
[1m--- a/RakeController/MotorController.cpp[m
[1m+++ b/RakeController/MotorController.cpp[m
[36m@@ -1,9 +1,9 @@[m
 #include "./MotorController.h"[m
 [m
[31m-const int CLOCK = 16;[m
[31m-const int PRESCALER = 128;[m
[31m-const unsigned long RPM_CON = 150150;[m
[31m-const int COUNTER_MULTIPLIER = (CLOCK*RPM_CON)/PRESCALER;[m
[32m+[m[32mconst unsigned long CLOCK = 16;[m
[32m+[m[32mconst int PRESCALER = 8;[m
[32m+[m[32mconst unsigned long RPM_CON = 75075;[m
[32m+[m[32mconst unsigned long COUNTER_MULTIPLIER = (CLOCK*RPM_CON)/PRESCALER;[m
 [m
 MotorController::MotorController(int pulsePos, int dirPos, int gateLift, int gateDrop) {[m
   this->pulsePos = pulsePos;[m
[36m@@ -26,10 +26,10 @@[m [mvoid MotorController::controllerInit() {[m
     3, 50, false, false, "state1"[m
   };[m
   motorStates[1] = (MotorState) {[m
[31m-    20, 140, false, false, "state2"[m
[32m+[m[32m    20, 180, false, false, "state2"[m
   };[m
   motorStates[2] = (MotorState) {[m
[31m-    133, 130, false, false, "state3"[m
[32m+[m[32m    133, 200, false, false, "state3"[m
   };[m
   motorStates[3] = (MotorState) {[m
     1014, 25, false, false, "state4"[m
[36m@@ -51,21 +51,19 @@[m [mvoid MotorController::interruptInit() {[m
   TCCR1A = 0;[m
   TCCR1B = 0;[m
   TCNT1  = 0;[m
[31m-[m
[31m-  //Set the compare match register[m
[31m-  OCR1A = ((CLOCK*RPM_CON)/(2*PRESCALER*motorStates[currentState].mSpeed))-1;; // = 16000000 / (1 * 13333.333333333334) - 1 (must be <65536)[m
[32m+[m[32m  interruptUpdate(motorStates[currentState].mSpeed);[m
   // turn on CTC mode[m
   TCCR1B |= (1 << WGM12);[m
   // Set CS12, CS11 and CS10 bits for 1 prescaler[m
[31m-  TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10); //Prescaler for compare match register set to 128[m
[32m+[m[32m  TCCR1B |= (0 << CS12) | ( 1<< CS11) | (0 << CS10); //Prescaler for compare match register set to 128[m
   // enable timer compare interrupt[m
   TIMSK1 |= (1 << OCIE1A);[m
   sei(); //allow interrupts[m
 }[m
 [m
[31m-//TODO Work out speed conversion values RPM->pulse frequency->timer counter value[m
[32m+[m
 void MotorController::interruptUpdate(int speedRPM) {[m
[31m-  int counter = (COUNTER_MULTIPLIER/(2*speedRPM));[m
[32m+[m[32m  long counter = (COUNTER_MULTIPLIER/speedRPM)-1;[m
   Serial.print("Speed: ");[m
   Serial.print(speedRPM);[m
   Serial.print(" Counter: ");[m
[36m@@ -122,8 +120,7 @@[m [mvoid MotorController::startProgram() {[m
 [m
 void MotorController::startMotor() {[m
   TCCR1B |= (1 << WGM12);[m
[31m-  // Set CS12, CS11 and CS10 bits for 1 prescaler[m
[31m-  TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10);[m
[32m+[m[41m  [m
   Serial.println("motor on");[m
 }[m
 [m
[36m@@ -133,7 +130,7 @@[m [mvoid MotorController::jogStart(bool mDirection) {[m
   Serial.print("current time interval: ");[m
   Serial.println(timeInterval);[m
   digitalWrite(dirPos, mDirection);[m
[31m-  interruptUpdate(5000);[m
[32m+[m[32m  interruptUpdate(180);[m
   startMotor();[m
 }[m
 [m
[36m@@ -144,10 +141,10 @@[m [mvoid MotorController::jogStop() {[m
   Serial.print(pausedTime);[m
   Serial.print(" New Time Interval: ");[m
   Serial.println(timeInterval);[m
[31m-  stopMotor();[m
[31m-  startMotor();[m
[32m+[m
   controllerActive = true;[m
   setMotorState(true);[m
[32m+[m[32m   startMotor();[m
 }[m
 [m
 void MotorController::drive() {[m
[36m@@ -158,14 +155,10 @@[m [mvoid MotorController::drive() {[m
 }[m
 [m
 void MotorController::iterateState() {[m
[31m-  Serial.print("Current state: ");[m
[31m-  Serial.print(motorStates[currentState].sName);[m
   currentState++;[m
   if (currentState == numberStates) {[m
     currentState = 0;[m
   }[m
[31m-  Serial.print(" Changing State to: ");[m
[31m-  Serial.println(motorStates[currentState].sName);[m
   setMotorState(false);[m
 }[m
 [m
[1mdiff --git a/RakeController/RakeController.ino b/RakeController/RakeController.ino[m
[1mindex 3fcd741..c13f691 100644[m
[1m--- a/RakeController/RakeController.ino[m
[1m+++ b/RakeController/RakeController.ino[m
[36m@@ -74,9 +74,11 @@[m [mvoid processCommand(JsonVariant command) {[m
   }[m
   else if (command["jog"])[m
   {[m
[31m-    Serial.println("Jog Command");[m
[32m+[m[32m    Serial.println("Jog");[m
     if (command["jog"] == "fwd"){[m
       controller.jogStart(true);[m
[32m+[m[32m    } else if (command["jog"] == "bwd") {[m
[32m+[m[32m      controller.jogStart(false);[m
     } else if (command["jog"] == "stop"){[m
       controller.jogStop();[m
     }[m
[36m@@ -124,7 +126,6 @@[m [mvoid setup() {[m
 [m
   Serial.begin(9600);[m
   controller.controllerInit();[m
[31m-  controller.stopMotor();[m
   controller.startProgram();[m
 }[m
 [m
