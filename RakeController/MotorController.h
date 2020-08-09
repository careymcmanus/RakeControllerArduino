
#ifndef MotorController_h
#define MotorController_h
/*
    MotorController.h - Library for controlling 
    motor the South Salt Bath Rake
    Created by Carey McManus
*/

#include "MotorState.h"
#include "CmdProc.h"
#include "BtnProc.h"

#define CLEAR_REGISTER 0b00000000

const uint8_t CLOCK = 16;
const uint8_t PRESCALER = 8;
const unsigned long RPM_CON = 75075;
const unsigned long COUNTER_MULTIPLIER = (CLOCK * RPM_CON) / PRESCALER;

class MotorController
{
private:
    CmdProc *cmdProc;

    BtnProc *btnProc;
    uint8_t btnFlags =0x00;

    uint64_t timeInterval;
    uint64_t pauseTime;
    uint64_t previousTime = 0;
    bool paused = false;

    OutPinArray *drivePins;

    rakePrg *prgmData;
    uint8_t prgNo;

    MotorState *cStateData;
    MotorState tempState;
    uint8_t numSts;
    uint8_t cState;

    bool mtrStpd;
    bool controllerActive;


    void MotorController::getCommand()
    {
        int cmd = 0;

        cmdProc->getCmd(&cmd);
        if (cmd != 0)
        {
            switch (cmd)
            {
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
                break;
            default:
                break;
            }
        }
    }

public:
    MotorController(OutPinArray *drvPins, BtnProc *bP, CmdProc *cP, rakePrg *prgms);

    //Initialiser Functions
    void controllerInit();
    void initInterrupt();

    //Update Functions
    void updateStateData();
    void drvSpdUPdt(uint16_t speed);
    void updtStateProps();
    
    //Start/Stop Functions
    void startProgram();
    void stopProgram();
    void startMotor();
    void stopMotor();

    //Main calling functions
    void main();
    void consumeFlags();

    //Control Functions
    void jogStart(bool dir);
    void jogStop();
    void iterateState();
    void toggleGate();

    //Cmd Interface Functions
    void setState();
    void getStates();
    void getCurrent();

    //Serial Monitor Functions
    void printStatus();
    void printSetState(uint8_t stateNum);
};
#endif
