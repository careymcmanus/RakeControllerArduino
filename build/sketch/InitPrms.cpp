#include "InitPrms.h"


MotorState mainPrg[6] = {{.Time=10,  .Speed=40,  .Flags = (1 << DIR) | (0 << GATE)},
                         {.Time=119, .Speed=200, .Flags = (1 << DIR) | (0 << GATE)}, 
                         {.Time=496, .Speed=25,  .Flags = (1 << DIR) | (0 << GATE)},
                         {.Time=230, .Speed=40,  .Flags = (1 << DIR) | (0 << GATE)},
                         {.Time=64,  .Speed=100, .Flags = (0 << DIR) | (0 << GATE)},
                         {.Time=197, .Speed=200, .Flags = (0 << DIR) | (1 << GATE)}};

MotorState scndPrg[2] = {{.Time=200, .Speed=200, .Flags = (0 << DIR) | (0 << GATE)},
                         {.Time=200, .Speed=200, .Flags = (1 << DIR) | (0 << GATE)}};

rakePrg prgms[2] = {{.Name="Main",  .Count=6 , .sts=mainPrg},
                    {.Name="Second", .Count=2, .sts=scndPrg}};

OutPinArray drvPins = {.EN=13, .Dir=12, .Lift=10, .Drop=9}; 

Btn btnData[2] = {{.pin=4,.b_acc=0,.b_val=0},
                  {.pin=5,.b_acc=0,.b_val=0}};

BtnSet btnSet  = {.Count=2, .btnData=btnData};
