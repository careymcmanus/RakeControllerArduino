
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

    unsigned long timeInterval;
    unsigned long pauseTime;
    unsigned long previousTime = 0;

    OutPinArray *drivePins;

    rakePrg *prgmData;
    uint8_t prgNo;

    MotorState *cStateData;
    MotorState tempState;
    uint8_t numSts;
    uint8_t cState;
    uint8_t sState;

    bool mtrStpd;
    bool controllerActive;

    void updateStateData()
    {
        numSts = prgmData[prgNo].numSts;
        cStateData = prgmData[prgNo].states;
    }
    void printBinary(uint8_t number)
    {
        for (int i = 0; i < 8; i++)
        {
            Serial.print(bitRead(number, i));
        }
        Serial.println();
    }
    void printSetState(MotorState state)
    {
        Serial.print("Motor State ");
        Serial.print(state.sName);
        Serial.println(" set with the following properties");
        Serial.print("Motor Speed:");
        Serial.print(state.mSpeed);
        Serial.print(" Time:");
        Serial.print(state.sTime);
        Serial.print(" Direction:");
        Serial.print(state.mDir);
        Serial.print(" Gate:");
        Serial.println(state.gate);
    }

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
                toggleGateState();
                break;
            default:
                break;
            }
        }
    }

public:
    MotorController(OutPinArray *drvPins, BtnProc *bP, CmdProc *cP, rakePrg *prgms);

    /*
     * Functions for processing incoming commands
     */

    void controllerInit();
    void interruptInit();
    void interruptUpdate(int value);

    void mainStateLoop();
    void consumeFlags();
    void setMotorState(bool unpausing);
    void startProgram();
    void stopProgram();
    void stopMotor();
    void startMotor();
    void jogStart(int state);
    void jogStop();
    void drive();
    void iterateState();
    void updateState();
    void printStates();
    void toggleGate();
    void toggleGateState();
    void stopState();

    void setState();
    void getStates();
    void getCurrent();
};
#endif
