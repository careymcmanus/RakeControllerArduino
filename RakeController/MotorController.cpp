#include "./MotorController.h"

const unsigned long CLOCK = 16;
const int PRESCALER = 8;
const unsigned long RPM_CON = 75075;
const unsigned long COUNTER_MULTIPLIER = (CLOCK*RPM_CON) / PRESCALER;

const byte numChars = 128;
char receivedChars[numChars];
boolean newData = false;
char stateMessage[200];

const size_t capacity = JSON_OBJECT_SIZE(5) + 40;
StaticJsonDocument<capacity> doc;

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


void MotorController::getCommand() {
  if (newData == true) {
    //USerial.println(receivedChars);
    char command = receivedChars[0];
    Serial.println(command);
    byte c = (byte)command;
    Serial.println(c);
    switch (c) {
      case 48:
        Serial.println("Stop Program");
        //stopProgram();
        break;
      case 49:
        Serial.println("Start Program");
        //startProgram();
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
        jogStart(true);
        break;
      case 53:
        Serial.println("Back Jog");
        jogStart(false);
        break;
      case 54:
        Serial.println("Stop Jog");
        jogStop();
        break;
      case 55:
        Serial.println("Set State");
        setState();
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
    if (dTime > timeInterval * 1000) {
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

void MotorController::setState() {

  char* command = receivedChars + 2;
  Serial.println(command);

  DeserializationError error = deserializeJson(doc, command);

  if (error) {
    Serial.print(F("Deserialization failed"));
    Serial.println(error.c_str());
    return;
  }
  JsonObject commandObj = doc.as<JsonObject>();

  if (commandObj["name"]) {
    String stateName = commandObj["name"].as<String>();
    Serial.print("Setting state ");
    Serial.println(stateName);
    for (int i = 0; i < numberStates; i++) {
      if (motorStates[i].sName == stateName) {
        motorStates[i].mSpeed = commandObj["speed"].as<int>();
        motorStates[i].sTime = commandObj["time"].as<int>();
        motorStates[i].mDir = commandObj["dir"].as<bool>();
        motorStates[i].gate = commandObj["gate"].as<bool>();
        Serial.print("Motor State ");
        Serial.print(motorStates[i].sName);
        Serial.println(" set with the following properties");
        Serial.print("Motor Speed:");
        Serial.print(motorStates[i].mSpeed);
        Serial.print(" Time:");
        Serial.print(motorStates[i].sTime);
        Serial.print(" Direction:");
        Serial.print(motorStates[i].mDir);
        Serial.print(" Gate:");
        Serial.println(motorStates[i].gate);
        return;
      }
    }
    Serial.println("State not found");
  } else {
    Serial.println("Error Processing Set Command - No State Name Found");
  }
  //
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
