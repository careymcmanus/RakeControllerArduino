#include "CmdProc.h"

CmdProc::CmdProc(SoftwareSerial *ss, uint32_t baud){
  softSerial = ss;
  softSerial->begin(baud);
}

void CmdProc::sendCmd(String msg){
    Serial.print("CmdProc Msg: ");
    Serial.println(msg);
   softSerial->print(msg);
   softSerial->println();
}

/*
*   Checks to see if message data exists.
*   Extracts cmd value and assigns it to a pointer to cmd
*   Assigns cmd 0 if no data exists.
*   @param: cmd - pointer to an int
*/
void CmdProc::getCmd(int* cmd){
    recvMsg();
    if(newData){
        *cmd = rcvMsg[0];
        Serial.println(*cmd);
        newData = false;
        return;
    }
    *cmd = 0;
}

void CmdProc::getMotorState(MotorState *mState){
    DeserializationError error = deserializeJson(jDoc, (rcvMsg+2));
    *mState = MotorState(DEFAULT_MSTATE);
    if (error) {
        Serial.print(F("Deserialization Failed: "));
        Serial.println(error.c_str());
    }
    JsonObject cmdObj = jDoc.as<JsonObject>();

    if (cmdObj["name"]){
        char *stateName = cmdObj["name"];
        int mSpeed = cmdObj["speed"].as<int>();
        int sTime = cmdObj["time"].as<int>();
        bool mDir = cmdObj["dir"].as<bool>();
        bool sGate = cmdObj["gate"].as<bool>();
        *mState = {.sTime = sTime, .mSpeed = mSpeed, .mDir = mDir, .gate = sGate, .sName = stateName};
        return;
    }
    Serial.println("Error Proccessing Set Command: No State Name Found");
    
}
