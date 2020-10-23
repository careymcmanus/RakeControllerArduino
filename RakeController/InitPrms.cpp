#include "InitPrms.h"


MotorState mainPrg[6] = {{.Time=6,  .Speed=40,  .Flags = (0 << DIR) | (0 << GATE)},
                         {.Time=125, .Speed=200, .Flags = (0 << DIR) | (0 << GATE)}, 
                         {.Time=340, .Speed=25,  .Flags = (0 << DIR) | (0 << GATE)},
                         {.Time=250, .Speed=40,  .Flags = (0 << DIR) | (0 << GATE)},
                         {.Time=64,  .Speed=100, .Flags = (1 << DIR) | (0 << GATE)},
                         {.Time=187, .Speed=200, .Flags = (1 << DIR) | (1 << GATE)}};

MotorState scndPrg[2] = {{.Time=200, .Speed=200, .Flags = (0 << DIR) | (0 << GATE)},
                         {.Time=200, .Speed=200, .Flags = (1 << DIR) | (0 << GATE)}};

rakePrg prgms[2] = {{.Name="Main",  .Count=6 , .sts=mainPrg},
                    {.Name="Second", .Count=2, .sts=scndPrg}};

OutPinArray drvPins = {.Drv=13, .Dir=12, .Lift=8}; 

Btn btnData[3] = {{.pin=6,.b_acc=0,.b_val=0},
                  {.pin=10,.b_acc=0,.b_val=0},
                  {.pin=11,.b_acc=0,.b_val=0}};

BtnSet btnSet  = {.Count=3, .btnData=btnData};
