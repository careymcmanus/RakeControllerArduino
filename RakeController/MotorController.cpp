#include "./MotorController.h"

const unsigned long CLOCK = 16;
const int PRESCALER = 8;
const unsigned long RPM_CON = 75075;
const unsigned long COUNTER_MULTIPLIER = (CLOCK*RPM_CON) / PRESCALER;

const byte numChars = 128;
char receivedChars[numChars];
boolean newData = false;
char stateMessage[200];

StaticJsonDocument<200> doc;
SoftwareSerial mySerial(2, 3); // RX, TX

MotorController::MotorController(int pulsePos, int dirPos, int gateLift, int gateDrop) {
  this->pulsePos = pulsePos;
  this->dirPos = dirPos;
  this->gateLift = gateLift;
  this->gateDrop = gateDrop;

}

void MotorController::recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (mySerial.available() > 0 && newData == false) {
    rc = mySerial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

void MotorController::processJson(char input[]) {
  DeserializationError error = deserializeJson(doc, input);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    newData = false;
    return;
  }
}



void MotorController::processCommand(JsonVariant command) {
  
}

void MotorController::getCommand() {
  if (newData == true) {
    Serial.println(receivedChars);
    byte command = receivedChars[0];
    Serial.println(command);
    switch(command){
      case 0:
        Serial.println("Stop Program");
        //stopProgram();
        break;
      case 1:
        Serial.println("Start Program");
        //startProgram();
        break;
      case 2:
        Serial.println("Get States");
        //getStates();
        break;
      case 3:
        Serial.println("Get Current");
        //getCurrent();
        break;
      case 4:
        Serial.println("Fwd Jog");
        //startJog(true);
        break;
      case 5:
        Serial.println("Back Jog");
        //startJog(false);
        break;
      case 6:
        Serial.println("Stop Jog");
        //stopJog();
        break;
      case 7:
        Serial.println("Set State");
        //setState();
        break;
      default:
        break;
    }
    newData = false;
  }
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
  mySerial.begin(9600);
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
    unsigned long dTime = (currentTime - previousTime);
    if (dTime > timeInterval*1000) {
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
  getCurrent();
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

void MotorController::getCurrent() {
  String message = "<{\"current\":" + String(currentState) + "}>";
  mySerial.println(message);
}

void MotorController::getStates() {

  mySerial.print("<{\"states\":[");
  for (int i = 0; i < numberStates; i++) {
    mySerial.print("{\"name\":" + motorStates[i].sName);
    mySerial.print(",\"speed\":" + String(motorStates[i].mSpeed));
    mySerial.print(",\"time\":" + String(motorStates[i].sTime));
    mySerial.print(",\"direction\":" + String(motorStates[i].mDir));
    mySerial.print(",\"gate\":" + String(motorStates[i].gate) + "}");
    if (i < numberStates - 1) {
      mySerial.print(",");
    }
  }
  mySerial.println("]}>");
}
