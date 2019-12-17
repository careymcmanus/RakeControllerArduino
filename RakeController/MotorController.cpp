#include "./MotorController.h"

const unsigned long CLOCK = 16;
const int PRESCALER = 8;
const unsigned long RPM_CON = 75075;
const unsigned long COUNTER_MULTIPLIER = (CLOCK*RPM_CON) / PRESCALER;

char stateMessage[200];

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
    20, 180, false, false, "state2"
  };
  motorStates[2] = (MotorState) {
    133, 200, false, false, "state3"
  };
  motorStates[3] = (MotorState) {
    1014, 25, false, false, "state4"
  };
  motorStates[4] = (MotorState) {
    65, 100, true, false, "state5"
  };
  motorStates[5] = (MotorState) {
    197, 200, true, true, "state6"
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
  interruptUpdate(motorStates[currentState].mSpeed);
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 1 prescaler
  TCCR1B |= (0 << CS12) | ( 1 << CS11) | (0 << CS10); //Prescaler for compare match register set to 128
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); //allow interrupts
}


void MotorController::interruptUpdate(int speedRPM) {
  long counter = (COUNTER_MULTIPLIER / speedRPM) - 1;
  Serial.print("Speed: ");
  Serial.print(speedRPM);
  Serial.print(" Counter: ");
  Serial.println(counter);
  OCR1A = counter;
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

  Serial.println("motor on");
}

void MotorController::jogStart(bool mDirection) {
  pauseTime = millis();
  controllerActive = false;
  Serial.print("current time interval: ");
  Serial.println(timeInterval);
  digitalWrite(dirPos, mDirection);
  interruptUpdate(180);
  startMotor();
}

void MotorController::jogStop() {
  unsigned long pausedTime = (millis() - pauseTime) / 1000; // /1000 to convert to seconds
  timeInterval += (pausedTime);
  Serial.print("paused time: ");
  Serial.print(pausedTime);
  Serial.print(" New Time Interval: ");
  Serial.println(timeInterval);

  controllerActive = true;
  setMotorState(true);
  startMotor();
}
void MotorController::printStates() {
  for (int i = 0; i < numberStates; i++) {
    Serial.println(motorStates[i].sName);
  }
}
void MotorController::drive() {
  digitalWrite(gateLift, motorStates[currentState].gate);
  digitalWrite(gateDrop, !motorStates[currentState].gate);
  digitalWrite(dirPos, motorStates[currentState].mDir);
  digitalWrite(pulsePos, !digitalRead(pulsePos));
}

void MotorController::iterateState() {
  currentState++;
  if (currentState == numberStates) {
    currentState = 0;
  }
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

void MotorController::getStates(SoftwareSerial mySerial) {

  mySerial.print("<{\"states\":[");
  for (int i = 0; i < 6; i++) {
    mySerial.print("{\"name\":" + motorStates[i].sName +
                         ",\"speed\":" + motorStates[i].mSpeed +
                         ",\"time\":" + motorStates[i].sTime +
                         ",\"direction\":" + motorStates[i].mDir +
                         ",\"gate\":" + motorStates[i].gate + "}");
    if(i < numberStates -1){
      mySerial.print(",");
    }

  }
  

  mySerial.println("]}>");
}
