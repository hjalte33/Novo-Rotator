#ifndef grbl_talker_h
#define grbl_talker_h

#define GRBL_SYNC_SPEED 200  // Hz

#define STATE_IDLE 0         // Idle
#define STATE_ALARM 1        // In alarm state. Locks out all g-code processes. Allows settings access.
#define STATE_CHECK_MODE 2   // G-code check mode. Locks out planner and motion only.
#define STATE_HOMING 3       // Performing homing cycle
#define STATE_CYCLE 4        // Cycle is running or motions are being executed.
#define STATE_HOLD 5         // Active feed hold
#define STATE_JOG 6          // Jogging mode.
#define STATE_SAFETY_DOOR 7  // Safety door is ajar. Feed holds and de-energizes system.
#define STATE_SLEEP 8        // Sleep state.

extern uint8_t grbl_state;
extern float grbl_rotations;
extern bool grbl_ok;

void grbl_init();
void grbl_reset();
void grbl_sync();
void _grbl_parser();

bool grbl_write(const char* c);

void grbl_go_mount();
void grbl_go_home();


#endif