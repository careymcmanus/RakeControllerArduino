#include "MotorController.h"
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX
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
    if (command["set"]["mdir"].isNull() || command["set"]["gState"].isNull()) {

      controller.setState(command["set"]["name"], command["set"]["speed"].as<int>());
    }
  }
  else if (command["get"])
  {
    Serial.println("Get Command");
    controller.getStates(mySerial);
  }
  else if (command["jog"])
  {
    Serial.println("Jog Command");
    controller.printStates();
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
    Serial.println(receivedChars);
    if (receivedChars[0] == '{') {
      Serial.println("This is a Json Object : ");
      Serial.println(receivedChars);
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

  mySerial.begin(9600);
  Serial.begin(9600);
  controller.controllerInit();
  controller.startProgram();
}

void loop() {
  recvWithStartEndMarkers();
  getCommand();
  controller.mainStateLoop();

}


ISR(TIMER1_COMPA_vect) {
  //interrupt commands for TIMER 1 here
  controller.drive();
}
