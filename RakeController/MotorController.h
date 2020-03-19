/*
    MotorController.h - Library for controlling 
    motor the South Salt Bath Rake
    Created by Carey McManus
*/
#ifndef MotorController_h
#define MotorController_h
#endif

#include "Arduino.h"
#include <ArduinoJson.h>
#include <SoftwareSerial.h>



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
        MotorState motorStates[numberStates+2];
        int currentState;
        int savedState;
        bool motorStopped;
        bool controllerActive;
     
        
       

    public:
    MotorController(int pulsePos, int dirPos, int gateLift, int gateDrop);

    /*
     * Functions for processing incoming commands
     */
    void recvWithStartEndMarkers();
    void processJson(char input[]);
    void processCommand(JsonVariant command);
    void getCommand();

    void controllerInit();

    void main();

    String convertSpeed(int value);
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
