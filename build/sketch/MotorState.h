#ifndef MOTORSTATE_H
#define MOTORSTATE_H

#include <stdint.h>
#include "macros.h"

#define DIR 0
#define GATE 3

struct MotorState{
  uint16_t Time;
  uint16_t Speed;
  uint8_t Flags; //contains flags for Motor Direction And Gate State
};

struct rakePrg{
    char *Name;
    uint8_t Count;
    MotorState *sts;
};

struct OutPinArray{
  uint8_t EN;
  uint8_t Dir;
  uint8_t Lift;
  uint8_t Drop;
};

struct Btn {
  uint8_t pin; // Pin Number for btn
  uint8_t b_acc; // the button accumulator
  uint8_t b_val; // the boolean button val
};

struct BtnSet{
    uint8_t Count;
    struct Btn *btnData;
};


#endif
