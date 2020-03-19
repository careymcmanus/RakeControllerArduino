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

SoftwareSerial nanoSerial(4,5); //RX, TX
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
        stopProgram();
        break;
      case 49:
        Serial.println("Start Program");
        startProgram();
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
    newData = false;
  }
}

void MotorController::controllerInit() {
  mySerial.begin(9600);
  nanoSerial.begin(9600);
  
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
  controllerActive = true;
}


void MotorController::main() {

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


void MotorController::toggleGateState() {
  Serial.println("Toggle Gate State");
  motorStates[currentState].gate = !motorStates[currentState].gate;
  setMotorState(false);
}

/*
 * Converts a integer value into a four digit string 
 * ie 100 would be represented as 0100 
 *    25 would be represented as 0025
 */
String MotorController::convertSpeed(int value){
  String output = String(value);
  String extra = "";
  if (value < 10){
    extra = "000";
  } else if (value < 100) {
    extra = "00";
  } else if (value < 1000) {
    extra = "0";
  }
  output = extra + output;
  return output;
}

void MotorController::setMotorState(bool unpausing) {
  MotorState state = motorStates[currentState];
  if (!unpausing) {
    timeInterval = state.sTime;
  }
  String nanoCommand = "<2" + convertSpeed(state.mSpeed) + String(state.mDir) + ">";
  nanoSerial.println(nanoCommand);
  digitalWrite(gateLift, motorStates[currentState].gate);
  digitalWrite(gateDrop, !motorStates[currentState].gate);
}

void MotorController::stopMotor() {
  motorStopped = true;
  Serial.println("motor off");
  nanoSerial.println("<0>");
 
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
  motorStopped = false;
  Serial.println("motor on");
  nanoSerial.println("<1>");
}

void MotorController::jogStart(int state) {
  pauseTime = millis();
  controllerActive = false;
  savedState = currentState;
  currentState = state;
  setMotorState(false);
  Serial.print("current time interval: ");
  Serial.println(timeInterval);
  startMotor();
}

void MotorController::jogStop() {
  unsigned long pausedTime = (millis() - pauseTime) / 1000; // /1000 to convert to seconds
  timeInterval += (pausedTime);
  Serial.print("paused time: ");
  Serial.print(pausedTime);
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

void MotorController::stopState() {

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
