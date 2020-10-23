#ifndef MACROS_H
#define MACROS_H

#define CLEAR_REGISTER 0x00

#define CLOCK 16
#define RPM_CON 75075
#define PRESCALER 8
#define COUNTER_MULTIPLIER ((CLOCK * RPM_CON) / PRESCALER)

#define CONTROLLER_ACTIVE 0
#define MOTOR_ACTIVE      1
#define PAUSED            2
#define FIRST_ON          3

#define LIMIT_SWITCH_1    0
#define LIMIT_SWITCH_2    1
#define RESET_BTN         2
#define ON_SWITCH         3



#define STATE_NO_NOT_VALID 3
#define SPEED_NOT_VALID 0
#define TIME_NOT_VALID 1
#define STATE_FLAGS_NOT_VALID 2

/*
* MACROS FOR RAKE DIRECTION
*/
#define BACKWARD 0x00
#define FORWARD 0xFF

/*
 * MACROS FOR COMMANDS
 */

//CMD Metadata

#define NUM_CHARS         16
#define START_MARK        '<'
#define END_MARK          '>'
#define RCV_IN_PROG       0
#define NEW_MSG           1

//CMD Numbers
 #define STOP_CMD         0
 #define START_CMD        1
 #define SEND_STATES_CMD  2
 #define SEND_CRNT_CMD    3
 #define SEND_START_CMD   4
 #define SEND_END_CMD     5
 #define FWD_RUN_CMD      6
 #define BCK_RUN_CMD      7
 #define STOP_RUN_CMD     8
 #define TOGGLE_GATE_CMD  9
 #define SET_STATE_CMD    10

// CMD DATA BYTES
#define STATE_NUM_BYTE 1
#define STATE_SPEED_BYTE 2
#define STATE_TIME_BYTE 4
#define STATE_FLAGS_BYTE 5

#endif 
