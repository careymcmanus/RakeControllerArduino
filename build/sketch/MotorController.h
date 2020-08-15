
#ifndef MotorController_h
#define MotorController_h

#include <SoftwareSerial.h>
#include <Arduino.h>

#include "utilities.h"
#include "InitPrms.h"
#include "BtnProc.h"
#include "MotorState.h"

/*
    MotorController.h - Library for controlling
    motor the South Salt Bath Rake
    Created by Carey McManus
*/

class MotorController
{
  private:

    uint64_t timeInterval;
    uint64_t pauseTime;
    uint64_t previousTime = 0;

    rakePrg *prgmData = prgms;
    uint8_t prgNo = 0;

    MotorState *cStateData = prgmData[prgNo].sts;
    uint8_t numSts = prgmData[prgNo].Count;;
    uint8_t cState = 0;

    char rcvMsg[NUM_CHARS];
    char errFlags = CLEAR_REGISTER;
    char cmdFlags = CLEAR_REGISTER;
    char btnFlags = CLEAR_REGISTER;
    char stateFlags = CLEAR_REGISTER;

    BtnProc btnProc = BtnProc(&btnSet, &btnFlags);
    SoftwareSerial *sSerial;

    char *convertFrom16To8(uint16_t dataAll) {
      static char arrayData[2] = { 0x00, 0x00 };

      *(arrayData) = (dataAll >> 8) & 0x00FF;
      arrayData[1] = dataAll & 0x00FF;
      return arrayData;
    }

    uint16_t convertFrom8To16(char dataFirst, char dataSecond) {
      uint16_t dataBoth = 0x0000;

      dataBoth = dataFirst;
      dataBoth = dataBoth << 8;
      dataBoth |= dataSecond;
      return dataBoth;
    }

    void printBinary(uint8_t number) {
      for (int i = 0; i < 8; i++) {
        Serial.print(bitRead(number, i));
      }
      Serial.println();
    }

  public:
    MotorController();

    //Initialiser Functions
    void controllerInit(SoftwareSerial *sS);
    void initInterrupt();

    //Update Functions
    void updateStateData();
    void updateStateProperties();
    void updateSpeed(uint16_t speed);


    //Start/Stop Functions
    void startProgram();
    void stopProgram();
    void startMotor();
    void stopMotor();

    //Main calling functions
    void main();
    void consumeFlags();

    //Control Functions
    void jogStart(char dir);
    void jogStop();
    void goToStart();
    void goToEnd();
    void iterateState();
    void toggleGate();

    //Cmd Interface Functions
    int checkMsg();
    int checkCmd();
    void sendCmd(char *msg);
    int processCmd();
    int setState();
    void sendState(int id);
    void sendAllStates();
    void sendCurrent();

    //Serial Monitor Functions
    void printStatus();
    void printSetState(uint8_t stateNum);
};
#endif
