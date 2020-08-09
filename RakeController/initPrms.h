#ifndef INITPRMS_H
#define INITPRMS_H

#include "MotorState.h"

MotorState mainPrgmSts[6] = {{.sTime=10, .mSpeed=40,  .mDir=false, .gate=false, .sName="state1"},
                            {.sTime=119, .mSpeed=200, .mDir=false, .gate=false, .sName="state2"}, 
                            {.sTime=496, .mSpeed=25,  .mDir=false, .gate=false, .sName="state3"},
                            {.sTime=230, .mSpeed=40,  .mDir=false, .gate=false, .sName="state4"},
                            {.sTime=64,  .mSpeed=100, .mDir=true,  .gate=false, .sName="state5"},
                            {.sTime=197, .mSpeed=200, .mDir=true,  .gate=true,  .sName="state6"}};

MotorState scndPrgSts[2] = {{.sTime=200, .mSpeed=200, .mDir=false, .gate=false, .sName="fwdState"},
                            {.sTime=200, .mSpeed=200, .mDir=true , .gate=false, .sName="bckState"}};

rakePrg prgms[2] = {{.progName="Main_Program",  .numSts=6 , .states=mainPrgmSts},
                    {.progName="Second_Program",.numSts=2,.states=scndPrgSts}};

OutPinArray drvPins = {.plsPin=12, .dirPin=13, .gLiftPin=11, .gDropPin=10, .mEnable=11};

Btn btnData[2] = {{.pin=4,.b_acc=0.,.b_val=0, .btnName="Limit1"},
                  {.pin=5,.b_acc=0.,.b_val=0, .btnName="Limit2"}};

BtnSet btnSet[1] = {{.setName="Button Set 1",.numBtns=2, .btnData=btnData}};

 #endif