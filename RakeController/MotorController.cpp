#include "MotorController.h"

MotorController::MotorController(int pulsePos, int dirPos, int gateLift, int gateDrop) {
  this->pulsePos = pulsePos;
  this->dirPos = dirPos;
  this->gateLift = gateLift;
  this->gateDrop = gateDrop;
}

void MotorController::controllerInit() {

  pinMode(pulsePos, OUTPUT);
  pinMode(dirPos, OUTPUT);
  pinMode(gateLift, OUTPUT);
  pinMode(gateDrop, OUTPUT);


  motorStates[0] = (MotorState) {
    10, 40, false, false, "state1"
  };
  motorStates[1] = (MotorState) {
    119, 200, false, false, "state2"
  };
  motorStates[2] = (MotorState) {
    496, 25, false, false, "state3"
  };
  motorStates[3] = (MotorState) {
    230, 40, false, false, "state4"
  };
  motorStates[4] = (MotorState) {
    64, 100, true, false, "state5"
  };
  motorStates[5] = (MotorState) {
    197, 200, true, true, "state6"
  };
  motorStates[6] = (MotorState) {
    180, 200, false, false, "Forward"
  };
  motorStates[7] = (MotorState) {
    180, 200, true, false, "Backward"
  };
  
  currentState = 0;
  cmdProc.initSerial();
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
  if (motorStopped) {
    startMotor();
  }
  if (speedRPM > 0) {
    long counter = (COUNTER_MULTIPLIER / speedRPM) - 1;
    OCR1A = counter;
  } else {
    stopMotor();
  }
}

void MotorController::mainStateLoop() {

  if (controllerActive) {
    unsigned long currentTime = millis();
    unsigned long dTime = (currentTime - previousTime);
    if (dTime > timeInterval * 1000) {
      previousTime = currentTime;
      iterateState();
    }
  }
}

void MotorController::getCommand() {
  uint8_t cmd = cmdProc.getCmd();
    switch (cmd) {
      case 48:
        Serial.println("Stop Program");
        break;
      case 49:
        Serial.println("Start Program");
        break;
      case 50:
        Serial.println("Get States");
        getStates();
        break;
      case 51:
        Serial.println("Get Current");
        getCurrent();
        break;
      case 52:
        Serial.println("Fwd Jog");
        jogStart(6);
        break;
      case 53:
        Serial.println("Back Jog");
        jogStart(7);
        break;
      case 54:
        Serial.println("Stop Jog");
        jogStop();
        break;
      case 55:
        Serial.println("Set State");
        setState();
        break;
      case 56:
        Serial.println("Change Gate");
        toggleGateState();
        break;
      default:
        break;
    }
  }

void MotorController::toggleGateState() {
  Serial.println("Toggle Gate State");
  motorStates[currentState].gate = !motorStates[currentState].gate;
  setMotorState(false);
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
  //TODO Redo this to work by setting the motor controller enable pin low
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  motorStopped = true;
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
  //TODO Rewrite to set motor controller enable pin HIGH
  sei();
  motorStopped = false;
  Serial.println("motor on");
}

void MotorController::jogStart(int state) {
  pauseTime = millis();
  controllerActive = false;
  savedState = currentState;
  currentState = state;
  setMotorState(false);
  startMotor();
}

void MotorController::jogStop() {
  uint64_t pausedTime = (millis() - pauseTime) / 1000; // /1000 to convert to seconds
  timeInterval += (pausedTime);
  Serial.print("paused time: ");
  Serial.print((long) pausedTime);
  Serial.print(" New Time Interval: ");
  Serial.println(timeInterval);
  currentState = savedState;
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
  digitalWrite(pulsePos, !digitalRead(pulsePos));
}

void MotorController::iterateState() {
  currentState++;
  if (currentState == numberStates) {
    currentState = 0;
  }
  getCurrent();
  setMotorState(false);
}

void MotorController::toggleGate() {
  digitalWrite(gateLift, !digitalRead(gateLift));
  digitalWrite(gateDrop, !digitalRead(gateDrop));
}

void MotorController::setState() {
    MotorState toSetState = cmdProc.getMotorState();
    for (int i = 0; i < numberStates; i++) {
      if (motorStates[i].sName == toSetState.sName) {
        motorStates[i].mSpeed = toSetState.mSpeed;
        motorStates[i].sTime = toSetState.sName;
        motorStates[i].mDir = toSetState.mDir;
        motorStates[i].gate = toSetState.gate;

        printSetState(motorStates[i]);
        return;
      }
    }

}

void MotorController::stopState() {

}

void MotorController::getCurrent() {
  String message = "<{\"current\":" + String(currentState) + "}>";
  cmdProc.sendCmd(message);
}

void MotorController::getStates() {
  String msg = "<{\"states\":[";
  for (int i = 0; i < numberStates; i++) {
    msg += "{\"name\":" + String(motorStates[i].sName) +
              ",\"speed\":" + String(motorStates[i].mSpeed) +
              ",\"time\":" + String(motorStates[i].sTime) +
              ",\"direction\":" + String(motorStates[i].mDir) +
              ",\"gate\":" + String(motorStates[i].gate) + "}";
    if (i < numberStates - 1) {
      msg += ",";
    }
  }
  msg += "]}>";
  Serial.print("Message: ");
  Serial.println(msg);
  cmdProc.sendCmd(msg);
}
