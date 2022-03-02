
#ifndef lcd_control_h
#define lcd_control_h

#include <Arduino.h>
#include "DFRobot_LCD.h"

//Frequencies
#define LCDUPDATEFREQ 2500 //number of milisec between LCD updates
#define BUTTONUPDATEFREQ 300 //number of millisecs between button readings

//LCD menu screen states
#define LCD_Menu_Info 0
#define LCD_Menu_StartStop 1
#define LCD_Menu_ProgramSelect 2
#define LCD_Selected_Info 3
#define LCD_Selected_ProgramSelect_P0 20
#define LCD_Selected_ProgramSelect_P1 21
#define LCD_Selected_ProgramSelect_P2 22
#define LCD_Selected_ProgramSelect_P3 23
#define LCD_Selected_ProgramSelect_P4 24
#define LCD_Selected_ProgramSelect_P5 25
#define LCD_Selected_P0_RPM 200
#define LCD_Selected_P0_Time 201
#define LCD_Starting 10
#define LCD_Stopping 11

//Start/stop states
#define LCD_SS_Unselected 0
#define LCD_SS_SelectedIdle 1
#define LCD_SS_SelectedActive 2

//External variables
extern DFRobot_LCD lcd;
extern long timeElapsed;

void lcdInit();
void btnsInit();
void lcdStartMenu();
void ISR_btnLeftPress();
void ISR_btnCenterPress();
void ISR_btnRightPress();
void btnLeftUpdate();
void btnCenterUpdate();
void btnRightUpdate();
void lcdUpdate();

#endif