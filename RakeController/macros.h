#ifndef MACROS_H
#define MACROS_H

#define CLEAR_REGISTER 0x00

#define CLOCK 16
#define RPM_CON 75075
#define PRESCALER 8
#define COUNTER_MULTIPLIER ((CLOCK * RPM_CON) / PRESCALER)

#define CONTROLLER_ACTIVE 0
#define MOTOR_ACTIVE 1
#define PAUSED 2
#define START_POSITION 3
#define END_POSITION 4
#define RECAL_TO_START 5
#define RECAL_TO_END 6

#define LIMIT_SWITCH_1 0
#define LIMIT_SWITCH_2 1

#define NUM_CHARS 16
#define START_MARK '<'
#define END_MARK '>'
#define RCV_IN_PROG 0
#define NEW_MSG 1

#define STATE_NO_NOT_VALID 3
#define SPEED_NOT_VALID 0
#define TIME_NOT_VALID 1
#define STATE_FLAGS_NOT_VALID 2

#define STATE_NUM_BYTE 1
#define STATE_SPEED_BYTE 2
#define STATE_TIME_BYTE 4
#define STATE_FLAGS_BYTE 5

#define BACKWARD 0x00
#define FORWARD 0xFF

#define END FORWARD
#define START BACKWARD

/* ---------------------------- 
 *  ANSII CHARACTER CODES
 ------------------------------ */
#define ZERO  48
#define ONE   49
#define TWO   50
#define THREE 51
#define FOUR  52
#define FIVE  53
#define SIX   54
#define SEVEN 55
#define EIGHT 56
#define NINE  57
#define A     65

#endif 
