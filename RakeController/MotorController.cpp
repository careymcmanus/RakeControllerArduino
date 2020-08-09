#include "MotorController.h"


MotorController::MotorController(OutPinArray *drvPins,
                                 BtnProc *bP, CmdProc *cP, rakePrg *prgms)
{
  drivePins = drvPins;
  cmdProc = cP;
  btnProc = bP;
  prgmData = prgms; 
  prgNo = 0;
  numSts = prgmData[prgNo].numSts;
  cStateData = prgmData[prgNo].states;
  cState = 0;
  sState = 0;
  tempState = MotorState(DEFAULT_MSTATE);
}

void MotorController::controllerInit()
{

  pinMode(drivePins->plsPin, OUTPUT);
  pinMode(drivePins->dirPin, OUTPUT);
  pinMode(drivePins->gLiftPin, OUTPUT);
  pinMode(drivePins->gDropPin, OUTPUT);
  pinMode(drivePins->mEnable, OUTPUT);

  controllerActive = false;
  btnProc->initProc(&btnFlags);
  initInterrupt();
}

/*
* Initialise the Timer 2 Interrupt
* Sets the mode of operation of Timer 2
*/
void MotorController::initInterrupt()
{
  //Clear registers to ensure correct settings
  TCCR2A = CLEAR_REGISTER; 
  TCCR2B = CLEAR_REGISTER;

  // Set the Timer/Counter Control 2A Register
  // to Clear Timer on Compare match mode (CTC)
  TCCR1A |= (1 << WGM12);

  // Set the Timer/Counter Control 2B Register prescaler
  // counter will use count times of clk_i/o/8
  TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10); 
}


/*
*
*/
void MotorController::main()
{
  getCommand();// check for any commands that may have been sent
  btnProc->getFlags(); // check for any buttons that may have been pressed
  consumeFlags();
  if (controllerActive)
  {
    unsigned long currentTime = millis();
    unsigned long dTime = (currentTime - previousTime);
    if (dTime > timeInterval * 1000)
    {
      iterateState();
      previousTime = currentTime;
    }
  }
  
}

void MotorController::consumeFlags(){
  // Check flag for limit switch 1
  if (btnFlags & 1) {
    Serial.println("Limit Switch 1 pressed");
    btnFlags &= ~1;
  }
  // Check flag for limit switch 2
  if (btnFlags & (1 << 1)){
    Serial.println("Limit Switch 2 pressed");
    btnFlags &= ~(1 << 1);
  }
}



void MotorController::drive()
{
  // Set Timer/Counter Control 1A Register
  // COM2A bits to Toggle OCRA on compare match
  // (PIN 11 on Arduino UNO)
  TCCR1A |= (1 << COM2A0);
  sei();
}

void MotorController::drvSpdUpdt()
{
    if (cStateData[cState]->mSpeed > 0)
  {
    uint16_t counter = (COUNTER_MULTIPLIER / speedRPM) - 1;
    OCR2A = counter;
  }
  else
  {
    stopMotor;
  }
}

void MotorController::toggleGateState()
{
  Serial.println("Toggle Gate State");
  cStateData[cState].gate = !cStateData[cState].gate;
  setMotorState(false);
}

void MotorController::setMotorState(bool unpausing)
{
  if (!unpausing)
  {
    timeInterval = cStateData[cState].sTime;
  }
  digitalWrite(drivePins->dirPin, cStateData[cState].mDir);
  digitalWrite(drivePins->gLiftPin, cStateData[cState].gate);
  digitalWrite(drivePins->gDropPin, !cStateData[cState].gate);
  interruptUpdate(cStateData[cState].mSpeed);
  Serial.print("Current State: ");
  Serial.print(cStateData[cState].sName);
  Serial.print(" : Motor Speed: ");
  Serial.println(cStateData[cState].mSpeed);
}

void MotorController::stopMotor()
{
  //TODO Redo this to work by setting the motor controller enable pin low

  mtrStpd = true;
  Serial.println("motor off");
  digitalWrite(drivePins->plsPin, LOW);
}

void MotorController::stopProgram()
{
  controllerActive = false;
  stopMotor();
}

void MotorController::startProgram()
{
  controllerActive = true;
  startMotor();
}

void MotorController::startMotor()
{
  //TODO Rewrite to set motor controller enable pin HIGH
  sei();
  mtrStpd = false;
  Serial.println("motor on");
}

void MotorController::jogStart(int state)
{
  pauseTime = millis();
  controllerActive = false;
  sState = cState;
  cState = state;
  setMotorState(false);
  startMotor();
}

void MotorController::jogStop()
{
  uint64_t pausedTime = (millis() - pauseTime) / 1000; // /1000 to convert to seconds
  timeInterval += (pausedTime);
  Serial.print("paused time: ");
  Serial.print((long)pausedTime);
  Serial.print(" New Time Interval: ");
  Serial.println(timeInterval);
  cState = sState;
  controllerActive = true;
  setMotorState(true);
  startMotor();
}

void MotorController::printStates()
{
  for (int i = 0; i < 6; i++)
  {
    Serial.println(cStateData[i].sName);
  }
}


void MotorController::iterateState()
{
  cState++;
  if (cState == numSts)
  {
    cState = 0;
  }
  getCurrent();
  setMotorState(false);
}

void MotorController::toggleGate()
{
  digitalWrite(drivePins->gLiftPin, !digitalRead(drivePins->gLiftPin));
  digitalWrite(drivePins->gDropPin, !digitalRead(drivePins->gDropPin));
}

void MotorController::setState()
{
  cmdProc->getMotorState(&tempState);

  for (int i = 0; i < 6; i++)
  {
    if (cStateData[i].sName == tempState.sName)
    {
      cStateData[i].mSpeed = tempState.mSpeed;
      cStateData[i].sTime = tempState.sName;
      cStateData[i].mDir = tempState.mDir;
      cStateData[i].gate = tempState.gate;

      printSetState(cStateData[i]);
      return;
    }
  }
}

void MotorController::stopState()
{
}

void MotorController::getCurrent()
{
  String message = "<{\"current\":" + String(cState) + "}>";
  cmdProc->sendCmd(message);
}

void MotorController::getStates()
{
  String msg = "<{\"states\":[";
  for (int i = 0; i < 6; i++)
  {
    msg += "{\"name\":" + String(cStateData[i].sName) +
           ",\"speed\":" + String(cStateData[i].mSpeed) +
           ",\"time\":" + String(cStateData[i].sTime) +
           ",\"direction\":" + String(cStateData[i].mDir) +
           ",\"gate\":" + String(cStateData[i].gate) + "}";
    if (i < 6 - 1)
    {
      msg += ",";
    }
  }
  msg += "]}>";
  Serial.print("Message: ");
  Serial.println(msg);
  cmdProc->sendCmd(msg);
}
