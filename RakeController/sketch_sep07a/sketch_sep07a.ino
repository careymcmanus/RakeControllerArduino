#include <SoftwareSerial.h>

#define NUM_CHARS 16
#define START_MARK '<'
#define END_MARK '>'
#define RCV_IN_PROG 0
#define NEW_MSG 1

char cmdFlags = 0x00;
char rcvMsg[NUM_CHARS];

SoftwareSerial sSerial(2,3);

int checkMsg(){
  static uint8_t index = 0;
  while (sSerial.available() > 0){
    char rc = sSerial.read();
    if (cmdFlags & (1 << RCV_IN_PROG)){
      if (rc == END_MARK){
        rcvMsg[index] = '\0';
        cmdFlags &= ~(1 << RCV_IN_PROG);
        cmdFlags |= (1 << NEW_MSG);
        index = 0;
        return 1;
      } else {
        rcvMsg[index] = rc;
        index++;
        if (index >= NUM_CHARS){
          cmdFlags &=~(1 << RCV_IN_PROG);
          return -1;
        }
      }
    } else if (rc == START_MARK){
      cmdFlags |= (1 << RCV_IN_PROG);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  sSerial.begin(9600);
}

void loop() {
  checkMsg();
  if(cmdFlags &= (1<< NEW_MSG)){
    Serial.write(rcvMsg); 
    Serial.println();
    Serial.println(rcvMsg);
    cmdFlags &= ~(1<<NEW_MSG);
  }
}
