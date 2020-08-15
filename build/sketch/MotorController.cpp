#include "MotorController.h"

MotorController::MotorController(){
  
}

void MotorController::controllerInit(SoftwareSerial *sS)
{
  pinMode(drvPins.Dir, OUTPUT);
  pinMode(drvPins.Lift, OUTPUT);
  pinMode(drvPins.Drop, OUTPUT);
  pinMode(drvPins.EN, OUTPUT);
  sSerial = sS;
  btnProc.init();
  initInterrupt();
}

/*
* Initialise the Timer 2 Interrupt
* Sets the mode of operation of Timer 2
*/
void MotorController::initInterrupt()
{
  //Clear registers to ensure correct settings
  TCCR1A = CLEAR_REGISTER; 
  TCCR1B = CLEAR_REGISTER;

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
      numSts = prgmData[prgNo].Count;
      cStateData = prgmData[prgNo].sts;
}

void MotorController::updateStateProperties()
{
  if (stateFlags & (0 << PAUSED))
  {
    timeInterval = cStateData[cState].Time;
  }
  digitalWrite(drvPins.Dir, cStateData[cState].Flags & (1 << DIR));
  digitalWrite(drvPins.Lift, cStateData[cState].Flags & (1 << GATE) );
  digitalWrite(drvPins.Drop, !cStateData[cState].Flags & (1 << GATE) );
  updateSpeed(cStateData[cState].Speed);
  printStatus();
}

void MotorController::updateSpeed(uint16_t speed)
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

void MotorController::startProgram()
{
  stateFlags |= (1 << CONTROLLER_ACTIVE);
  startMotor();
}

void MotorController::stopProgram()
{
  stateFlags &= (0 << CONTROLLER_ACTIVE);
  stopMotor();
}
/*
* Starts the Motor 
* Sets the EN pin HIGH,
* Then turns on the toggle setting of Timer/Control Register 1 
*/
void MotorController::startMotor()
{
  if (stateFlags & ~(1 << MOTOR_ACTIVE)){
  updateStateProperties();
  digitalWrite(drvPins.EN, HIGH);
  stateFlags |= (1 << MOTOR_ACTIVE);
  TCCR1A |= (1 << COM1A0);
  printStatus();
}
}

/*
* Stops the Motor
* turns off the toggle setting of Timer/Control Register 1 
* Then the EN pin LOW.
*/
void MotorController::stopMotor()
{
  if (stateFlags & (1 << MOTOR_ACTIVE)){
  TCCR1A |= (0 << COM1A0);
  stateFlags |= ~(1 << MOTOR_ACTIVE);
  printStatus();
  digitalWrite(drvPins.EN, LOW);
  }
}




/*
*
*/
void MotorController::main()
{
  checkCmd();// check for any commands that may have been sent
  btnProc.getFlags(); // check for any buttons that may have been pressed
  consumeFlags();
  if (stateFlags & (1 << CONTROLLER_ACTIVE)) 
  {
    uint64_t currentTime = millis();
    if ((currentTime - previousTime) > timeInterval * 1000)
    {
      iterateState();
      previousTime = currentTime;
    }
  }
  
}

void MotorController::consumeFlags(){
  // Check flag for limit switch 1
  if (btnFlags & (1 << LIMIT_SWITCH_1)) {
    if (stateFlags & (1 << RECAL_TO_START)){
      stopMotor();
      cState = 0;
      updateStateProperties();
      stateFlags &= (0 < RECAL_TO_START);
      Serial.println("At Start!");
    }
    stateFlags |= (1 << START_POSITION);
    btnFlags &= (0 << LIMIT_SWITCH_1);
    Serial.println("Limit Switch 1 pressed");
  }
  // Check flag for limit switch 2
  if (btnFlags & (1 << LIMIT_SWITCH_2)){
    if (stateFlags & (1 << RECAL_TO_END)){
      stopMotor();
      cState = 4;
      updateStateProperties();
      stateFlags &= (0 < RECAL_TO_END);
      Serial.println("At End!");
    }
    stateFlags |= (1 << END_POSITION);
    btnFlags &= (0 << LIMIT_SWITCH_2);
    Serial.println("Limit Switch 2 pressed");
  }
}



/* --------------------------------------------------------
      CONTROL FUNCTIONS
   -------------------------------------------------------- */

void MotorController::jogStart(char dir)
{
  pauseTime = millis();
  stateFlags &= (1 << PAUSED);
  stopMotor();
  digitalWrite(drvPins.Dir, dir);
  stateFlags &= (0 << CONTROLLER_ACTIVE);
  updateSpeed(150);
  startMotor();
}

void MotorController::jogStop()
{
  stopMotor();
  //Add the pause time to the timeInterval
  timeInterval += (millis() - pauseTime)/1000;
  stateFlags &= (1 << CONTROLLER_ACTIVE);
  updateStateProperties();
  startMotor();
}

void MotorController::goToStart(){
  stateFlags |= (1 << RECAL_TO_START);
  jogStart(BACKWARD);
}

void MotorController::iterateState()
{
  cState++;
  if (cState == numSts)
  {
    cState = 0;
  }
  updateStateProperties(); 
}

void MotorController::toggleGate()
{
  digitalWrite(drvPins.Lift, !digitalRead(drvPins.Lift));
  digitalWrite(drvPins.Drop, !digitalRead(drvPins.Drop));
}

/* --------------------------------------------------------- 
      COMMAND INTERFACE FUNCTIONS
   --------------------------------------------------------- */
int MotorController::checkMsg(){
  static uint8_t index = 0;
  while (sSerial->available() > 0){
    char rc = sSerial->read();
    if (cmdFlags & (1 << RCV_IN_PROG)){
      if (rc == END_MARK){
        rcvMsg[index] = '\0';
        cmdFlags &= (0 << RCV_IN_PROG);
        cmdFlags |= (1 << NEW_MSG);
        index = 0;
        return 1;
      } else {
        rcvMsg[index] = rc;
        index++;
        if (index >= NUM_CHARS){
          cmdFlags &=(0 << RCV_IN_PROG);
          return -1;
        }
      }
    } else if (rc == START_MARK){
      cmdFlags |= (1 << RCV_IN_PROG);
    }
  }
}

int MotorController::checkCmd(){
    int err;
    checkMsg();
    if(cmdFlags & (1 << NEW_MSG)){
      Serial.print("MSG RCVD: ");
      Serial.println(rcvMsg);
      err = processCmd();
      switch (err)
      {
      case 1:
        /* Command Processed */
        break;
      case -1:
        /* No Command Found */
        sendCmd("No Command Found");
      case -2:
        /* No Valid Command Found */
        sendCmd("No Valid Command Found");
      default:
        break;
      }
      cmdFlags &= (0 << NEW_MSG);
      return 1;
    }
    return 0;
}

//Todo use flags for faults
int MotorController::processCmd(){
        if (rcvMsg[0] >0)
        {
            Serial.print("CMD: ");
            Serial.println((int) rcvMsg[0]);
            switch ((int)rcvMsg[0])
            {
            case 48:
                Serial.println("Stop Program");
                stopProgram();
                break;
            case 49:
                Serial.println("Start Program");
                startProgram();
                break;
            case 50:
                Serial.println("Send States");
                sendAllStates();
                break;
            case 51:
                Serial.println("Get Current");
                sendCurrent();
                break;
            case 52:
                Serial.println("Fwd Jog");
                jogStart(FORWARD);
                break;
            case 53:
                Serial.println("Back Jog");
                jogStart(BACKWARD);
                break;
            case 54:
                Serial.println("Stop Jog");
                jogStop();
                break;
            case 55:
                Serial.println("Set State");
                setState();
                break;
            case 56:
                Serial.println("Change Gate");
                toggleGate();
                break;
            default:
                return -2;
                break;
            }
            return 1;
        }
        return -1;
}

void MotorController::sendCmd(char *msg){
    sSerial->write(msg, sizeof(msg));
}

int MotorController::setState()
{
  int i = rcvMsg[STATE_NUM_BYTE];
  uint16_t Speed = convertFrom8To16(rcvMsg[STATE_SPEED_BYTE], rcvMsg[STATE_SPEED_BYTE+1]);
  uint16_t Time = convertFrom8To16(rcvMsg[STATE_TIME_BYTE], rcvMsg[STATE_TIME_BYTE + 1]);
  uint8_t Flags = rcvMsg[STATE_FLAGS_BYTE];

  if (i < 0 || i > numSts){
    errFlags |= (1 << STATE_NO_NOT_VALID);
    return -1;
  }
  if (Speed < 0 || Speed < 300){
      errFlags |= (1 << SPEED_NOT_VALID);
      return -1;
    }
  if (Time < 0 || Time < 20000){
      errFlags |= (1 << TIME_NOT_VALID);
      return -1;
    }
      cStateData[i].Speed = Speed;
      cStateData[i].Time = Time;
      cStateData[i].Flags = Flags;
      printSetState(i);
      return 0; 
}

void MotorController::sendState(int id){
    char *speed = convertFrom16To8(cStateData[id].Speed);
    char *time = convertFrom16To8(cStateData[id].Time);
    char flags = cStateData[id].Flags;
    char msg[8] = {'<', id, speed[0], speed[1], time[0], time[1], flags, '>'};
    sendCmd(msg);
}

void MotorController::sendCurrent()
{
    sendState(cState);
}

void MotorController::sendAllStates(){
  for (int i = 0; i < numSts; i++){
    sendState(i);
  }
}

/* -----------------------------------
        SERIAL MONITOR FUNCTIONS
---------------------------------------*/

void MotorController::printStatus(){
  Serial.print(" Current State: ");
  Serial.print(cState);
  Serial.print(" Speed: ");
  Serial.print(cStateData[cState].Speed);
  Serial.print(" Speed: ");
  Serial.print(cStateData[cState].Speed);
  Serial.print(" Direction: ");
  Serial.println(cStateData[cState].Flags & (1 << DIR));
  Serial.print(" Gate: ");
  Serial.println(cStateData[cState].Flags & (1 << GATE));
}

void MotorController::printSetState(uint8_t stateNum)
    {
        Serial.print("Motor State ");
        Serial.print(stateNum);
        Serial.println(" set with the following properties");
        Serial.print("Motor Speed:");
        Serial.print(cStateData[stateNum].Speed);
        Serial.print(" Time:");
        Serial.print(cStateData[stateNum].Time);
        Serial.print(" Direction:");
        Serial.print(cStateData[stateNum].Flags & (1 << DIR));
        Serial.print(" Gate:");
        Serial.println(cStateData[stateNum].Flags & (1 << GATE));
    }
