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
  // Enable all Interrupts 
  sei(); 
}


void MotorController::updateStateData(){
      numSts = prgmData[prgNo].numSts;
      cStateData = prgmData[prgNo].states;
}

void MotorController::updateStateProps()
{
  if (!paused)
  {
    timeInterval = cStateData[cState].sTime;
    paused = false;
  }
  digitalWrite(drivePins->dirPin, cStateData[cState].mDir);
  digitalWrite(drivePins->gLiftPin, cStateData[cState].gate);
  digitalWrite(drivePins->gDropPin, !cStateData[cState].gate);
  interruptUpdate(cStateData[cState].mSpeed);
  printStatus();
}

void MotorController::drvSpdUpdt(uint16_t speed)
{
    if (speed >0)
  { 
    OCR2A = (COUNTER_MULTIPLIER / speed) - 1;
  }
  else
  {
    stopMotor();
  }
}
/*
* Starts the Motor 
* Sets the mEnable pin HIGH,
* Then turns on the toggle setting of Timer/Control Register 1 
*/
void MotorController::startMotor()
{
  if (mtrStpd){
  digitalWrite(drivePins->mEnable, HIGH);
  mtrStpd = false;
  TCCR1A |= (1 << COM1A0);
  printStatus();
}
}

/*
* Stops the Motor
* turns off the toggle setting of Timer/Control Register 1 
* Then the mEnable pin LOW.
*/
void MotorController::stopMotor()
{
  //TODO Redo this to work by setting the motor controller enable pin low
  if (!mtrStpd)
  TCCR1A |= (0 << COM1A0);
  mtrStpd = true;
  printStatus();
  digitalWrite(drivePins->mEnable, LOW)
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





/*
*
*/
void MotorController::jogStart(uint8_t dir)
{
  pauseTime = millis();
  paused = true;
  stopMotor();
  controllerActive = false;
  drvSpdUpdt(150);
  startMotor();
}

/*
*
*/
void MotorController::jogStop()
{
  stopMotor();
  Serial.print(" Old Time Interval: ");
  Serial.print(timeInterval);
  //Add the pause time to the timeInterval
  timeInterval += (millis() - pauseTime)/1000;
  Serial.print(" --- New Time Interval: ");
  Serial.println(timeInterval);

  controllerActive = true;
  setState();
  startMotor();
}

void MotorController::iterateState()
{
  cState++;
  if (cState == numSts)
  {
    cState = 0;
  }
  getCurrent();
  setMotorState();
}

void MotorController::toggleGate()
{
  digitalWrite(drivePins->gLiftPin, !digitalRead(drivePins->gLiftPin));
  digitalWrite(drivePins->gDropPin, !digitalRead(drivePins->gDropPin));
}

/* --------------------------------------------------------- 
      COMMAND INTERFACE FUNCTIONS
   --------------------------------------------------------- */


void MotorController::setState()
{
  cmdProc->getMotorState(&tempState);

  for (int i = 0; i < numSts; i++)
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

void MotorController::getCurrent()
{
  String message = "<{\"current\":" + String(cState) + "}>";
  cmdProc->sendCmd(message);
}

void MotorController::getStates()
{
  String msg = "<{\"states\":[";
  for (int i = 0; i < numSts; i++)
  {
    msg += "{\"name\":" + String(cStateData[i].sName) +
           ",\"speed\":" + String(cStateData[i].mSpeed) +
           ",\"time\":" + String(cStateData[i].sTime) +
           ",\"direction\":" + String(cStateData[i].mDir) +
           ",\"gate\":" + String(cStateData[i].gate) + "}";
    if (i < numSts - 1)
    {
      msg += ",";
    }
  }
  msg += "]}>";
  Serial.print("Message: ");
  Serial.println(msg);
  cmdProc->sendCmd(msg);
}

/* -----------------------------------
        SERIAL MONITOR FUNCTIONS
---------------------------------------*/

void MotorController::printStatus(){
  Serial.print(" Current State: ");
  Serial.print(cStateData[cState].sName);
  Serial.print(" Speed: ");
  Serial.print(cStateData[cState].mSpeed);
  Serial.print(" Dir: ");
  Serial.print(cStateData[cState].mDir);
}

void MotorController::printSetState(uint8_t stateNum)
    {
        Serial.print("Motor State ");
        Serial.print(cStateData[stateNum].sName);
        Serial.println(" set with the following properties");
        Serial.print("Motor Speed:");
        Serial.print(cStateData[stateNum].mSpeed);
        Serial.print(" Time:");
        Serial.print(cStateData[stateNum].sTime);
        Serial.print(" Direction:");
        Serial.print(cStateData[stateNum].mDir);
        Serial.print(" Gate:");
        Serial.println(cStateData[stateNum].gate);
    }