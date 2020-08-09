#ifndef CMDPROC_H
#define CMDPROC_H



#include <Arduino.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

#include "MotorState.h"

#define DEFAULT_MSTATE {0,0,0,0, " "}

const uint8_t NUM_CHARS = 128;
const size_t CAPACITY = JSON_OBJECT_SIZE(5) + 40;

class CmdProc
{
public:
    CmdProc(SoftwareSerial *ss, uint32_t baud);
    void sendCmd(String cmd);
    void getMotorState(MotorState *mState);
    void getCmd(int* cmd);

private:
    SoftwareSerial *softSerial;
    StaticJsonDocument<CAPACITY> jDoc;
    bool rcvInProg = false;
    bool newData = false;
    uint8_t index = 0;
    char strtMark = '<';
    char endMark = '>';
    char rcvMsg[NUM_CHARS];

    void recvMsg()
     {
        
        while (softSerial->available() > 0 && newData == false)
        {
            char rc = softSerial->read();
            Serial.println(rc);
            if (rcvInProg)
            {
                if (rc == endMark)
                {
                    rcvMsg[index] = '\0';
                    rcvInProg = false;
                    index = 0;
                    newData = true;
                }
                else
                {
                    rcvMsg[index] = rc;
                    index++;
                    if (index >= NUM_CHARS)
                    {
                        index = NUM_CHARS - 1;
                    }
                }
            }
            else if (rc == strtMark)
            {
                rcvInProg = true;
            }
        }
     }


};

#endif
