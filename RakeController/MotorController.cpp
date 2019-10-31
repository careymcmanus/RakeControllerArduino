#include "./MotorController.h"


MotorController::MotorController(int pulsePos, int dirPos, int gateLift, int gateDrop) {
  this->pulsePos = pulsePos;
  this->dirPos = dirPos;
  this->gateLift = gateLift;
  this->gateDrop = gateDrop;

  init();
}

void MotorController::controllerInit() {

  pinMode(pulsePos, OUTPUT);
  pinMode(dirPos, OUTPUT);
  pinMode(gateLift, OUTPUT);
  pinMode(gateDrop, OUTPUT);


  motorStates[0] = (MotorState) {
    3, 50, false, false, "state1"
  };
  motorStates[1] = (MotorState) {
    20, 140, false, false, "state2"
  };
  motorStates[2] = (MotorState) {
    25, 130, false, false, "state3"
  };
  motorStates[3] = (MotorState) {
    15, 25, false, false, "state4"
  };
  motorStates[4] = (MotorState) {
    40, 100, true, false, "state5"
  };
  motorStates[5] = (MotorState) {
    24, 200, true, true, "state6"
  };
  currentState = 0;

  interruptInit();
  controllerActive = true;
}

void MotorController::interruptInit() {
  //Initialize Timer1
  cli(); //Disables all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  //Set the compare match register
  OCR1A = 1000; // = 16000000 / (1 * 13333.333333333334) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 1 prescaler
  TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); //allow interrupts
}

//TODO Work out speed conversion values RPM->pulse frequency->timer counter value
void MotorController::interruptUpdate(int value) {
  OCR1A = value * 100;
}

void MotorController::mainStateLoop() {

  if (controllerActive) {
    unsigned long currentTime = millis();
    unsigned long dTime = (currentTime - previousTime) / 1000;
    if (dTime > timeInterval) {
      previousTime = currentTime;
      Serial.print("Time Period: ");
      Serial.println(dTime);
      iterateState();
    }

  }

}

void MotorController::setMotorState(bool unpausing) {
  if (!unpausing) {
    timeInterval = motorStates[currentState].sTime;
  }


  interruptUpdate(motorStates[currentState].mSpeed);
  digitalWrite(dirPos, motorStates[currentState].mDir);
  digitalWrite(gateLift, motorStates[currentState].gate);
  digitalWrite(gateDrop, !motorStates[currentState].gate);
  Serial.print("Current State: ");
  Serial.print(motorStates[currentState].sName);
  Serial.print(" : Motor Speed: ");
  Serial.println(motorStates[currentState].mSpeed);
}

void MotorController::stopMotor() {
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  Serial.println("motor off");
  digitalWrite(pulsePos, LOW);
}
void MotorController::stopProgram() {
  controllerActive = false;
  stopMotor();
}

void MotorController::startProgram() {
  controllerActive = true;
  startMotor();
}

void MotorController::startMotor() {
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 1 prescaler
  TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10);
  Serial.println("motor on");
}

void MotorController::jogStart(bool mDirection) {
  pauseTime = millis();
  controllerActive = false;
  Serial.print("current time interval: ");
  Serial.println(timeInterval);
  digitalWrite(dirPos, mDirection);
  interruptUpdate(5000);
  startMotor();
}

void MotorController::jogStop() {
  unsigned long pausedTime = (millis() - pauseTime) / 1000; // /1000 to convert to seconds
  timeInterval += (pausedTime);
  Serial.print("paused time: ");
  Serial.print(pausedTime);
  Serial.print(" New Time Interval: ");
  Serial.println(timeInterval);
  stopMotor();
  startMotor();
  controllerActive = true;
  setMotorState(true);
}

void MotorController::drive() {
  digitalWrite(gateLift, motorStates[currentState].gate);
  digitalWrite(gateDrop, !motorStates[currentState].gate);
  digitalWrite(dirPos, motorStates[currentState].mDir);
  digitalWrite(pulsePos, !digitalRead(pulsePos));
}

void MotorController::iterateState() {
  Serial.print("Current state: ");
  Serial.print(motorStates[currentState].sName);
  currentState++;
  if (currentState == numberStates) {
    currentState = 0;
  }
  Serial.print(" Changing State to: ");
  Serial.println(motorStates[currentState].sName);
  setMotorState(false);
}

void MotorController::toggleGate() {
  digitalWrite(gateLift, !digitalRead(gateLift));
  digitalWrite(gateDrop, !digitalRead(gateDrop));
}

void MotorController::setState(String stateName, int stateSpeed) {
  for (int i = 0; i < numberStates; i++) {

    if (motorStates[i].sName == stateName) {
      Serial.println("setting motor state ");

      motorStates[i].mSpeed = stateSpeed;
      Serial.print("Motor State ");
      Serial.println(motorStates[i].sName);
      Serial.println(" set with the following properties");
      Serial.print("Motor Speed:");
      Serial.println(motorStates[i].mSpeed);
      return;
    }
  }
  Serial.println("State not found");
}

void MotorController::setState(String stateName, int stateSpeed, bool mDir, bool gate) {
  for (int i = 0; i < numberStates; i++) {

    if (motorStates[i].sName == stateName) {
      Serial.println("setting motor state ");

      motorStates[i].mSpeed = stateSpeed;
      motorStates[i].mDir = mDir;
      motorStates[i].gate = gate;
      Serial.print("Motor State ");
      Serial.println(motorStates[i].sName);
      Serial.println(" set with the following properties");
      Serial.print(motorStates[i].mSpeed);
      Serial.print(" : ");
      Serial.print(motorStates[i].mDir);
      Serial.print(" : ");
      Serial.println(motorStates[i].gate);
      return;
    }
  }
  Serial.println("State not found");
}

void MotorController::getStates() {
  Serial.print("{\"states\":[");

  for (int i = 0; i < numberStates; i++) {

    String stateString = "{\"name\":" + motorStates[i].sName +
                         ",\"speed\":" + motorStates[i].mSpeed +
                         ",\"time\":" + motorStates[i].sTime +
                         ",\"direction\":" + motorStates[i].mDir +
                         ",\"gate\":" + motorStates[i].gate + "}";
    Serial.print(stateString);
    if (i < numberStates - 1) {
      Serial.print(",");
    }


  }
  Serial.println("]}");
}
