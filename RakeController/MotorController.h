
#ifndef MotorController_h
#define MotorController_h
/*
    MotorController.h - Library for controlling 
    motor the South Salt Bath Rake
    Created by Carey McManus
*/

#include <Arduino.h>
#include "MotorState.h"
#include "CmdProc.h"

const unsigned long CLOCK = 16;
const int PRESCALER = 8;
const unsigned long RPM_CON = 75075;
const unsigned long COUNTER_MULTIPLIER = (CLOCK * RPM_CON) / PRESCALER;

class MotorController
{
private:
    CmdProc cmdProc = CmdProc();
    unsigned long timeInterval;
    unsigned long pauseTime;
    unsigned long previousTime = 0;
    static const int numberStates = 6;
    byte pulsePos, dirPos, gateLift, gateDrop;
    MotorState motorStates[numberStates + 2];
    int currentState;
    int savedState;
    bool motorStopped;
    bool controllerActive;

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

public:
    MotorController(int pulsePos, int dirPos, int gateLift, int gateDrop);

    /*
     * Functions for processing incoming commands
     */

    void getCommand();

    void controllerInit();
    void interruptInit();
    void interruptUpdate(int value);

    void mainStateLoop();

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