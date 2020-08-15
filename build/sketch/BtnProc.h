#ifndef BTNPROC_H
#define BTNPROC_H

#include <Arduino.h>
#include "MotorState.h"
#include "InitPrms.h"


class BtnProc
{
private:
  BtnSet *btnData;
  int cSetNo = 0;

  Btn *cBtnData;
  int numBtns;

  char btns = 0x00;
  char *buttonFlags;
  char lastButtons = 0x00;

public:
  BtnProc(BtnSet *bData, char *bFlags);
  void changeBtnSet();
  void init();
  void filterButton(int i);
  void filterAllButtons();
  void getFlags();
  void printButtons();
};

#endif
