#include "MotorController.h"

/*
*
*/
void MotorController::main()
{
  checkCmd();         // check for any commands that may have been sent
  btnProc.getFlags(); // check for any buttons that may have been pressed
  consumeFlags();

  if (stateFlags & (1 << CONTROLLER_ACTIVE))
  {
    if (!(stateFlags & (1 << PAUSED)))
      {
      if (stateFlags & (1 << FIRST_ON))
      {
        Serial.println("Controller Active: ");
        digitalWrite(5, HIGH);
        updateStateProperties();
        startMotor();
        stateFlags &= ~(1 << FIRST_ON);
      }
      uint64_t currentTime = millis();
      if ((currentTime - previousTime) > timeInterval * 1000)
      {
        iterateState();
        previousTime = currentTime;
      }
    }
  }
  else
  {
    stopMotor();
    digitalWrite(5, LOW);
  }
}

void MotorController::consumeFlags()
{
  // Check flag for limit switch 1
  if (btnFlags & (1 << LIMIT_SWITCH_1))
  {
    Serial.println("LIMIT_SWITCH_1");
    reset();
    btnFlags &= ~(1 << LIMIT_SWITCH_1);
  }
  // Check flag for limit switch 2
  if (btnFlags & (1 << LIMIT_SWITCH_2))
  {
    Serial.println("LIMIT_SWITCH_2");
    overrunError();
    btnFlags &= ~(1 << LIMIT_SWITCH_2);
  }
  if (btnFlags & (1 << RESET_BTN))
  {
    Serial.println("RESET");
    sendToLimit(BACKWARD);
    btnFlags &= ~(1 << RESET_BTN);
  }
  if (stateFlags & (1 << CONTROLLER_ACTIVE))
  {
    if (!digitalRead(ON_SWITCH))
    {
      startProgram();
    }
  } else {
    if (digitalRead(ON_SWITCH))
    {
      stopProgram();
    }
  }
}

void MotorController::controllerInit(SoftwareSerial *sS)
{
  stateFlags |= (1 << FIRST_ON);
  
  pinMode(drvPins.Dir, OUTPUT);
  pinMode(drvPins.Lift, OUTPUT);
  pinMode(drvPins.Drv, OUTPUT);
  //TODO NO MAGIC NUMBERS
  pinMode(5, OUTPUT); // TODO PIN 5 is ON Indicator
  pinMode(4, OUTPUT); // TODO PIN 4 is OVERRUN Indicator
  pinMode(7, INPUT_PULLUP);
  sSerial = sS;
  btnProc.init();
  initInterrupt();
}

void MotorController::initInterrupt()
{
  if (stateFlags & ~(1 << MOTOR_ACTIVE))
  {
    stateFlags |= (1 << MOTOR_ACTIVE);
    cli();
    //Clear registers to ensure correct settings
    TCCR1A = CLEAR_REGISTER;
    TCCR1B = CLEAR_REGISTER;
    TCNT1 = CLEAR_REGISTER;
    // Set the Timer/Counter Control 2A Register
    // to Clear Timer on Compare match mode (CTC)
    TCCR1B |= (1 << WGM12);

    // Set the Timer/Counter Control 2B Register prescaler
    // counter will use count times of clk_i/o/8
    TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
    // Enable all Interrupts
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);
  }
}

void MotorController::updateStateData()
{
  numSts = prgmData[prgNo].Count;
  cStateData = prgmData[prgNo].sts;
}

void MotorController::updateStateProperties()
{
  stopMotor();
  if (stateFlags & ~(1 << PAUSED))
  {
    timeInterval = cStateData[cState].Time;
  }
  digitalWrite(drvPins.Dir, cStateData[cState].Flags & (1 << DIR));
  digitalWrite(drvPins.Lift, cStateData[cState].Flags & (1 << GATE));
  updateSpeed(cStateData[cState].Speed);
}

void MotorController::updateSpeed(uint16_t speed)
{
  long counter = (COUNTER_MULTIPLIER / speed) - 1;
  if (counter > 0)
  {
    OCR1A = counter;
  }
}
/* --------------------------------------------------------
      CONTROL FUNCTIONS
   -------------------------------------------------------- */

void MotorController::startProgram()
{
  stateFlags |= (1 << CONTROLLER_ACTIVE);
}

void MotorController::stopProgram()
{
  stateFlags &= ~(1 << CONTROLLER_ACTIVE);
  stopMotor();
}

void MotorController::startMotor()
{
  sei();
  printStatus();
}

void MotorController::stopMotor()
{
  cli();
}

void MotorController::drive()
{
  digitalWrite(drvPins.Drv, !digitalRead(drvPins.Drv));
}

void MotorController::jogStart(char dir)
{
  stopMotor();
  pauseTime = millis();
  stateFlags |= (1 << PAUSED);

  digitalWrite(drvPins.Dir, dir);
  updateSpeed(150);
  startMotor();
}

void MotorController::jogStop()
{
  stopMotor();
  //Add the pause time to the timeInterval
  timeInterval += (millis() - pauseTime) / 1000;
  stateFlags &= ~(1 << PAUSED);
  updateStateProperties();
}

/*
 * Send Rake to the Limits of Systems
 * @param limit: which limit to send it
 * TODO: add in error messages
 */
void MotorController::sendToLimit(char limit)
{
  stopMotor();
  stateFlags |= (1 << PAUSED);
  digitalWrite(drvPins.Dir, limit);
  //TODO potentially have some sort of time delay on gate opening
  digitalWrite(drvPins.Gate, HIGH);
  updateSpeed(150);
  startMotor();
}

void MotorController::iterateState()
{
  stopMotor();
  cState++;
  if (cState == numSts)
  {
    cState = 0;
  }
  updateStateProperties();
  startMotor();
}

void MotorController::toggleGate()
{
  digitalWrite(drvPins.Lift, !digitalRead(drvPins.Lift));
}

/*
*
*/
void MotorController::reset()
{
  if (stateFlags & (1 << PAUSED)){
    stateFlags &= ~(1 << PAUSED);
  }
  cState = 0;
  updateStateProperties();
  startMotor();
}

/*
*
*/
void MotorController::overrunError()
{
  Serial.println("OVERRUN ERROR:");
  digitalWrite(4, HIGH);
  stopMotor();
}

/* --------------------------------------------------------- 
      COMMAND INTERFACE FUNCTIONS
   --------------------------------------------------------- */
int MotorController::checkMsg()
{
  static uint8_t index = 0;
  while (sSerial->available() > 0)
  {
    char rc = sSerial->read();
    if (cmdFlags & (1 << RCV_IN_PROG))
    {
      if (rc == END_MARK)
      {
        rcvMsg[index] = '\0';
        cmdFlags &= ~(1 << RCV_IN_PROG);
        cmdFlags |= (1 << NEW_MSG);
        index = 0;
        return 1;
      }
      else
      {
        rcvMsg[index] = rc;
        index++;
        if (index >= NUM_CHARS)
        {
          cmdFlags &= ~(1 << RCV_IN_PROG);
          return -1;
        }
      }
    }
    else if (rc == START_MARK)
    {
      cmdFlags |= (1 << RCV_IN_PROG);
    }
  }
}

/*
 * Calls checkMsg and then if New Msg  
 * Calls function to process Msg
 * TODO add in error Handling
 */
int MotorController::checkCmd()
{
  checkMsg();
  if (cmdFlags & (1 << NEW_MSG))
  {
    Serial.print("MSG RCVD: ");
    Serial.println(rcvMsg);
    processCmd();
    cmdFlags &= ~(1 << NEW_MSG);
    return 1;
  }
  return 0;
}
/*
 * Processes Cmds
 * TODO add in error codes
 */
int MotorController::processCmd()
{
  if (rcvMsg[0] > 0)
  {
    Serial.print("CMD: ");
    Serial.println((int)rcvMsg[0]);
    switch ((int)rcvMsg[0])
    {
    case STOP_CMD:
      Serial.println("Stop Program");
      stopProgram();
      break;
    case START_CMD:
      Serial.println("Start Program");
      startProgram();
      break;

    default:
      return -1;
      break;
    }
    return 1;
  }
  return -1;
}

void MotorController::sendCmd(char *msg)
{
  sSerial->write(msg, sizeof(msg));
}

int MotorController::setState()
{
  int i = rcvMsg[STATE_NUM_BYTE];
  uint16_t Speed = convertFrom8To16(rcvMsg[STATE_SPEED_BYTE], rcvMsg[STATE_SPEED_BYTE + 1]);
  uint16_t Time = convertFrom8To16(rcvMsg[STATE_TIME_BYTE], rcvMsg[STATE_TIME_BYTE + 1]);
  uint8_t Flags = rcvMsg[STATE_FLAGS_BYTE];

  if (i < 0 || i > numSts)
  {
    errFlags |= (1 << STATE_NO_NOT_VALID);
    return -1;
  }
  if (Speed < 0 || Speed < 300)
  {
    errFlags |= (1 << SPEED_NOT_VALID);
    return -1;
  }
  if (Time < 0 || Time < 20000)
  {
    errFlags |= (1 << TIME_NOT_VALID);
    return -1;
  }
  cStateData[i].Speed = Speed;
  cStateData[i].Time = Time;
  cStateData[i].Flags = Flags;
  printSetState(i);
  return 0;
}

void MotorController::sendState(int id)
{
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

void MotorController::sendAllStates()
{
  for (int i = 0; i < numSts; i++)
  {
    sendState(i);
  }
}

/* -----------------------------------
        SERIAL MONITOR FUNCTIONS
---------------------------------------*/

void MotorController::printStatus()
{
  Serial.print(" Current State: ");
  Serial.print(cState);
  Serial.print(" Time: ");
  Serial.print(cStateData[cState].Time);
  Serial.print(" Speed: ");
  Serial.print(cStateData[cState].Speed);
  Serial.print(" Direction: ");
  Serial.print(cStateData[cState].Flags & (1 << DIR));
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
