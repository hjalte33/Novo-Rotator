
#ifndef lcd_control_h
#define lcd_control_h

#include <Arduino.h>
#include "DFRobot_LCD.h"

#define LCDUPDATEFREQUENCY 1000

#define LCD_Menu_Info 0
#define LCD_Menu_StartStop 1
#define LCD_Menu_ProgramSelect 2
#define LCD_Selected_Info 3
#define LCD_Selected_ProgramSelect_P0 4
#define LCD_Selected_ProgramSelect_P1 5
#define LCD_Selected_ProgramSelect_P2 6
#define LCD_Selected_ProgramSelect_P3 7
#define LCD_Selected_ProgramSelect_P4 8
#define LCD_Selected_ProgramSelect_P5 9
#define LCD_Selected_P0_RPM 10
#define LCD_Selected_P0_Time 11

extern DFRobot_LCD lcd;

void lcdInit();
void btnsInit();
void lcdStartMenu();
void btnLeftPress();
void btnCenterPress();
void btnRightPress();
void btnLeftUpdate();
void btnCenterUpdate();
void btnRightUpdate();

#endif