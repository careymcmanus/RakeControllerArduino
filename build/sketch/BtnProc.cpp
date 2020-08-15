#include "BtnProc.h"

BtnProc::BtnProc(BtnSet *bData, char *bFlags){
  btnData = bData;
  buttonFlags = bFlags;
  numBtns = btnData->Count;
  cBtnData = btnData->btnData; 
}

void BtnProc::init(){
  if (numBtns > 0)
    {
      for (int i = 0; i < numBtns; i++)
      {
        pinMode(cBtnData[i].pin, INPUT_PULLUP);
      }
      printButtons();
    }
}

  void BtnProc::filterButton(int i)
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

  
  void BtnProc::filterAllButtons()
  {
    btns = 0x00;
    for (int i = numBtns - 1; i >= 0; i--)
    {
      filterButton(i);
      btns = (btns << 1) | cBtnData[i].b_val;
    }
  }



void BtnProc::getFlags(){
  filterAllButtons();
  *buttonFlags |= btns & ~lastButtons;;
  lastButtons = btns;
}


void BtnProc::printButtons()
  {
    Serial.print("Number of Buttons: ");
    Serial.println(numBtns);
    for (int i = 0; i < numBtns; i++)
    {
      Serial.print("Button: ");
      Serial.print(i);
      Serial.print(" : ");
      Serial.print("Assigned Pin: ");
      Serial.println(cBtnData[i].pin);
    }
  }
