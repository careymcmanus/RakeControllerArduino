#ifndef BTNPROC_H
#define BTNPROC_H

#include "MotorState.h";

class BtnProc
{
private:
  BtnSet *btnSet;
  uint8_t cSetNo = 0;
  Btn *cBtnData;
  uint8_t numBtns;
  uint8_t btns = 0x00;
  uint8_t *buttonFlags;
  uint8_t lastButtons = 0x00;

  void printMemAddress()
  {
    Serial.print("Btn Proc Data Adress: ");
    Serial.println((int)cBtnData);
  }

  void printButtons()
  {
    Serial.print("Number of Buttons: ");
    Serial.println(numBtns);
    for (int i = 0; i < numBtns; i++)
    {
      Serial.print("Button: ");
      Serial.print(cBtnData[i].btnName);
      Serial.print(" : ");
      Serial.print("Assigned Pin: ");
      Serial.println(cBtnData[i].pin);
    }
  }

  void initBtns()
  {
    if (numBtns > 0)
    {
      for (int i = 0; i < numBtns; i++)
      {
        pinMode(cBtnData[i].pin, INPUT_PULLUP);
      }
      printButtons();
    }
  }

  void updateBtnSet()
  {
    cBtnData = btnSet[cSetNo].btnData;
    numBtns = btnSet[cSetNo].numBtns;
    printButtons();
  }

  void filterButtonN8(int i)
  {
    cBtnData[i].b_acc <<= 1; // shift out oldest state, make new state a zero
    bool pinState = digitalRead(cBtnData[i].pin);
    if (!pinState)
    {
      cBtnData[i].b_acc |= 1;
      if (cBtnData[i].b_acc == 255)
      {
        cBtnData[i].b_val = 1;
      }
      else
      {
        cBtnData[i].b_val = 0;
      }
    }
  }

  void filterAllButtons()
  {
    btns = 0x00;
    for (int i = numBtns - 1; i >= 0; i--)
    {
      filterButtonN8(i);
      btns = (btns << 1) | cBtnData[i].b_val;
    }
  }

public:
  BtnProc(BtnSet *bData);
  void changeBtnSet();
  void initProc(uint8_t *bFlags);
  void getFlags();
};

#endif
