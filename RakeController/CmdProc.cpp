#include "CmdProc.h"

CmdProc::CmdProc(SoftwareSerial *ss, uint32_t baud){
  softSerial = ss;
  softSerial->begin(baud);
}

void CmdProc::sendCmd(String msg){
   
}

uint8_t CmdProc::getCmd(){
    recvMsg();
    if(newData){
        int cmd = rcvMsg[0];
        newData = false;
        return cmd;
    }
}

MotorState CmdProc::getMotorState(){
    DeserializationError error = deserializeJson(jDoc, (rcvMsg+2));

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
        MotorState mState = {.sTime = sTime, .mSpeed = mSpeed, .mDir = mDir, .gate = sGate, .sName = stateName};
        return mState;
    }
    Serial.println("Error Proccessing Set Command: No State Name Found");
    
}
