
#include <Arduino.h>
#include "../include/grbl_talker.h"

uint8_t grbl_state = 255;
float grbl_rotations;

char in_buff[64];
char out_buff[64];
bool grbl_ok = false;

unsigned long grbl_last_sync = 0;

void grbl_init(){
    Serial1.begin(115200);
    delay(500);
    grbl_sync();
}

void grbl_reset() {
    Serial1.println(0x18);
    delay(500);
    grbl_sync();
}

void grbl_sync(){
    unsigned long now = millis();
    if(now > grbl_last_sync+ GRBL_SYNC_SPEED){
        // ask grbl about status
        Serial1.print("?\n");
        delay(50);
        
        if (Serial1.available()) {
            Serial1.readBytesUntil('\n',in_buff,64);
            _grbl_parser();
        }else{
            return;
        }
        grbl_last_sync=now;
    }else{
        Serial1.print("\n");
        delay(50);
    }

    // read ok tag 
    if(Serial1.available()){
        Serial1.readBytesUntil('\n',in_buff,64);
        if(in_buff[0] == 'o' && in_buff[1] == 'k'){
            grbl_ok = true;
        }else{
            grbl_ok = false;
        }
        //Serial1.flush();
    }
}

void _grbl_parser() {
    // valid options for status = `Idle, Run, Hold, Jog, Alarm, Door, Check, Home, Sleep`
    switch (in_buff[1]) // skip first "<" symbol
    {
    case 'I': grbl_state = STATE_IDLE;           break;
    case 'R': grbl_state = STATE_CYCLE;          break;
    case 'J': grbl_state = STATE_JOG;            break;
    case 'A': grbl_state = STATE_ALARM;          break;
    case 'D': grbl_state = STATE_SAFETY_DOOR;    break;
    case 'C': grbl_state = STATE_CHECK_MODE;     break;
    case 'S': grbl_state = STATE_SLEEP;          break;
    case 'H': switch (in_buff[3]){
        case 'l': grbl_state = STATE_HOLD;       break;
        case 'm': grbl_state = STATE_HOMING;     break;
        }
    }

    // read out the x position = rotations
    uint8_t index_MPos = String(in_buff).indexOf("MPos:") + 5;
    grbl_rotations = atof(&in_buff[index_MPos]);
}

/**
 * @brief sends a command to grbl. Assumes grbl is ok
 * 
 * @param c command string to send
 */
bool grbl_write(const char* c) {
    // Before we send anything check the ok flag from grbl
    if (!grbl_ok) {return false;}

    // awake grbl if asleep or in alarm mode.
    if (grbl_state == STATE_SLEEP) {
        Serial1.println(0x18);
        delay(25);
        grbl_ok = true;
        return false;
    } else if (grbl_state == STATE_ALARM) {
        grbl_go_home();
        return false;
    }
    
    // send the buffer
    if (c[0] != 0) {
        Serial1.println(c);
        grbl_ok = false;
        delay(10);
        grbl_sync();
    }
    return true;
}

void grbl_go_home() {
    Serial1.println("$H");
    grbl_ok = false;
    delay(10);
    grbl_sync(); // extra call to sync to get the status update asap
}

void grbl_go_mount() {
    
}

