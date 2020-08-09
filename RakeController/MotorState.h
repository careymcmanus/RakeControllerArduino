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

struct rakePrg{
    char *progName;
    uint8_t numSts;
    MotorState *states;
};

struct OutPinArray{
  uint8_t plsPin;
  uint8_t dirPin;
  uint8_t gLiftPin;
  uint8_t gDropPin;
  uint8_t mEnable;
};

struct Btn {
  uint8_t pin; // Pin Number for btn
  uint8_t b_acc; // the button accumulator
  uint8_t b_val; // the boolean button val
  char *btnName;
};

struct BtnSet{
    char *setName;
    uint8_t numBtns;
    struct Btn *btnData;
};

#endif

