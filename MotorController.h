/*
    MotorController.h - Library for controlling 
    motor the South Salt Bath Rake
    Created by Carey McManus
*/
#ifndef MotorController_h
#define MotorController_h
#endif

#include "Arduino.h"
#include <ArduinoSTL.h>

typedef struct{
  int mSpeed;
  int sTime;
  bool mDir;
  bool gate;
  String sName;
} MotorState;


class MotorController {
    private:
        byte pulsePos, dirPos, gateLift, gateDrop;
        MotorState motorStates[3];
        int currentState;
        bool controllerActive;
        
       

    public:
    MotorController(int pulsePos, int dirPos, int gateLift, int gateDrop);

    void controllerInit();
    void interruptInit();
    void interruptUpdate(int value);

    void setMotorState();
    void stopMotor();
    void startMotor();
    void jogStart(bool mDirection);
    void jogStop();
    void drive();

    void toggleGate();
    
    void setState(String stateName, int stateSpeed);
    void setState(String stateName, int stateSpeed, bool motorDirection, bool gateState);
    void getStates();
    
};
