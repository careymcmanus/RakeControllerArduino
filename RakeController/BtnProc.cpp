#include "BtnProc.h"

BtnProc::BtnProc(BtnSet *bData){
  btnSet = bData;
  cSetNo = 0;
  numBtns = btnSet[cSetNo].numBtns;
  cBtnData = btnSet[cSetNo].btnData; 

}


void BtnProc::initProc(uint8_t *bFlags){
  buttonFlags = bFlags;
  initBtns();
}

void BtnProc::changeBtnSet(){
  cSetNo++;
  if (cSetNo > 1){
    cSetNo = 0;
  }
  updateBtnSet();
}

void BtnProc::getFlags(){
  filterAllButtons();
  *buttonFlags |= btns & ~lastButtons;;
  lastButtons = btns;
}
