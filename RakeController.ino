#include "MotorController.h"
#include <ArduinoJson.h>
#include <ArduinoSTL.h>

StaticJsonDocument<200> doc;

const byte numChars = 128;
MotorController controller = MotorController(12, 13, 5, 6);
char receivedChars[numChars];
boolean newData = false;

void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

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



void processJson(char input[]) {
  DeserializationError error = deserializeJson(doc, input);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    newData = false;
    return;
  }
}

void processCommand(JsonVariant command) {
  Serial.println("Processing Command... ");


  if (command["set"])
  {
    Serial.println("Set Command");
    if (command["set"]["mdir"].isNull() || command["set"]["gState"].isNull()){
      
    controller.setState(command["set"]["name"], command["set"]["speed"].as<int>());
    }
  }
  else if (command["get"])
  {
    Serial.println("Get Command");
    controller.getStates();
  }
  else if (command["jog"])
  {
    Serial.println("Jog Command");
  }
  else if (command["start"])
  {
    Serial.println("Start Command");
  }
  else if (!command["on"].isNull()) {
  }
  else
  {
    Serial.println("Invalid Command");
  }

}


void getCommand() {
  if (newData == true) {
    // If serial command starts with a bracket then process command as a Json Command

    if (receivedChars[0] == '{') {
      Serial.print("This is a Json Object : ");

      processJson(receivedChars);

      JsonVariant Command = doc.as<JsonVariant>();
      processCommand(Command);
    } else {
      Serial.println("Invalid Command ... ");
    }

    newData = false;
  }
}





void setup() {
  

  Serial.begin(9600);
  controller.controllerInit();
  controller.stopMotor();
}

void loop() {
  recvWithStartEndMarkers();
  getCommand();

}


ISR(TIMER1_COMPA_vect) {
  //interrupt commands for TIMER 1 here
  controller.drive();
}
