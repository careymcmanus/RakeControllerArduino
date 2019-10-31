/*
    MotorController.h - Library for controlling 
    motor the South Salt Bath Rake
    Created by Carey McManus
*/
#ifndef MotorController_h
#define MotorController_h
#endif

#include "Arduino.h"

typedef struct{
  int sTime;
  int mSpeed;
  bool mDir;
  bool gate;
  String sName;
} MotorState;


class MotorController {
    private:
        unsigned long timeInterval;
        unsigned long pauseTime;
        unsigned long previousTime = 0;
        static const int numberStates = 6;
        byte pulsePos, dirPos, gateLift, gateDrop;
        MotorState motorStates[numberStates];
        int currentState;
        bool controllerActive;
        
       

    public:
    MotorController(int pulsePos, int dirPos, int gateLift, int gateDrop);

    void controllerInit();
    void interruptInit();
    void interruptUpdate(int value);

    void mainStateLoop();

    void setMotorState(bool unpausing);
    void startProgram();
    void stopProgram();
    void stopMotor();
    void startMotor();
    void jogStart(bool mDirection);
    void jogStop();
    void drive();
    void iterateState();
    void updateState();

    void toggleGate();
    
    void setState(String stateName, int stateSpeed);
    void setState(String stateName, int stateSpeed, bool motorDirection, bool gateState);
    void getStates();
    
};
