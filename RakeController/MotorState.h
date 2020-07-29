#ifndef MOTORSTATE_H
#define MOTORSTATE_H

#include <Arduino.h>

struct MotorState{
  int sTime;
  int mSpeed;
  bool mDir;
  bool gate;
  char *sName;
};

#endif

