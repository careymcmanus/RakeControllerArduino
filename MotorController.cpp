#include "./MotorController.h"

MotorController::MotorController(int pulsePos, int dirPos, int gateLift, int gateDrop){
    this->pulsePos = pulsePos;
    this->dirPos = dirPos;
    this->gateLift = gateLift;
    this->gateDrop = gateDrop;
    
    init();
}

void MotorController::controllerInit(){
    
      pinMode(pulsePos, OUTPUT);
      pinMode(dirPos, OUTPUT);
      pinMode(gateLift, OUTPUT);
      pinMode(gateDrop, OUTPUT);

        
    MotorState state1 = {1000, 10000, true, false, "state1"};
    MotorState state2 = {1000, 20000, true, false, "state2"};
    MotorState state3 = {1000, 30000, true, false, "state3"};
    
    Serial.println(state1.sName);
    Serial.println(state2.sName);
    Serial.println(state3.sName);
    
    motorStates[0] = state1;
    motorStates[1] = state2;
    motorStates[2] = state3;

    Serial.println(motorStates[2].sName);
      currentState = 0;

      interruptInit();
      controllerActive = true;
}

void MotorController::interruptInit(){
    //Initialize Timer1
      cli(); //Disables all interrupts
      TCCR1A = 0;
      TCCR1B = 0;
      TCNT1  = 0;

      //Set the compare match register
      OCR1A = 1000; // = 16000000 / (1 * 13333.333333333334) - 1 (must be <65536)
      // turn on CTC mode
      TCCR1B |= (1 << WGM12);
      // Set CS12, CS11 and CS10 bits for 1 prescaler
      TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10);
      // enable timer compare interrupt
      TIMSK1 |= (1 << OCIE1A);
      sei(); //allow interrupts
}

void MotorController::interruptUpdate(int value){
    OCR1A = value;
}

void MotorController::setMotorState() {
    interruptUpdate(motorStates[currentState].mSpeed);
    Serial.print("Current State: ");
    Serial.print(motorStates[currentState].sName);
    Serial.print(" : Motor Speed: ");
    Serial.println(motorStates[currentState].mSpeed);
}

void MotorController::stopMotor() {
    TCCR1A = 0;
      TCCR1B = 0;
      TCNT1  = 0;
      Serial.println("motor off");
      digitalWrite(pulsePos, LOW);
      controllerActive = false;
}

void MotorController::startMotor() {
      TCCR1B |= (1 << WGM12);
      // Set CS12, CS11 and CS10 bits for 1 prescaler
      TCCR1B |= (1 << CS12) | (0 << CS11) | (1 << CS10);
      Serial.println("motor on");
      controllerActive = true;
    }

void MotorController::jogStart(bool mDirection){
    digitalWrite(dirPos, mDirection);
    interruptUpdate(5000);
    startMotor();
}

void MotorController::jogStop(){
    stopMotor();
}

void MotorController::drive(){
    digitalWrite(gateLift, motorStates[currentState].gate);
      digitalWrite(gateDrop, !motorStates[currentState].gate);
      digitalWrite(dirPos, motorStates[currentState].mDir);
      digitalWrite(pulsePos, !digitalRead(pulsePos));
}

void MotorController::toggleGate(){
    digitalWrite(gateLift, !digitalRead(gateLift));
    digitalWrite(gateDrop, !digitalRead(gateDrop));
}

void MotorController::setState(String stateName, int stateSpeed){
     for (int i = 0; i < sizeof(motorStates); i++) {

        if (motorStates[i].sName == stateName) {
          Serial.println("setting motor state ");

          motorStates[i].mSpeed = stateSpeed;
          Serial.print("Motor State ");
          Serial.println(motorStates[i].sName);
          Serial.println(" set with the following properties");
          Serial.print("Motor Speed:");
          Serial.println(motorStates[i].mSpeed);
          return;
        }
     }
     Serial.println("State not found");
}

void MotorController::setState(String stateName, int stateSpeed, bool mDir, bool gate){
    for (int i = 0; i < sizeof(motorStates); i++) {

    if (motorStates[i].sName == stateName) {
      Serial.println("setting motor state ");

      motorStates[i].mSpeed = stateSpeed;
      motorStates[i].mDir = mDir;
      motorStates[i].gate = gate;
      Serial.print("Motor State ");
      Serial.println(motorStates[i].sName);
      Serial.println(" set with the following properties");
      Serial.print(motorStates[i].mSpeed);
      Serial.print(" : ");
      Serial.print(motorStates[i].mDir);
      Serial.print(" : ");
      Serial.println(motorStates[i].gate);
      return;
    }
  }
  Serial.println("State not found");
}

void MotorController::getStates(){
    String statelistString = "{\"states\":[";
    Serial.println(sizeof(motorStates));
    for (int i = 0; i < sizeof(motorStates); i++){
        
        String stateString = "{\"name\":" + motorStates[i].sName + 
                             ",\"speed\":" + motorStates[i].mSpeed +
                             ",\"time\":" + motorStates[i].sTime +
                             ",\"direction\":" + motorStates[i].mDir +
                             ",\"gate\":" + motorStates[i].gate + "}";
        Serial.println(stateString);
        
        
    }
    
    statelistString = statelistString + "]}";
    Serial.println(statelistString);
}
