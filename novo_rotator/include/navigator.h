
#ifndef lcd_control_h
#define lcd_control_h

#include <Arduino.h>
#include "DFRobot_LCD.h"

//Frequencies
#define INFOUPDATEFREQ 2500 //number of milisec between LCD updates for info screen
#define SSUPDATEFREQ 4000 //number of milisec between LCD updates for starting/stopping screen
#define BUTTONUPDATEFREQ 300 //number of millisecs between button readings

//LCD menu screen states
#define LCD_Menu_Info                   0
#define LCD_Menu_StartStop              1
#define LCD_Menu_ProgramSelect          2
#define LCD_Selected_Info               3
#define LCD_Starting                    10
#define LCD_Stopping                    11
#define LCD_ProgramSelect_Return        20
#define LCD_ProgramSelect_P0            21
#define LCD_ProgramSelect_Pf            22
#define LCD_FILENAVIGATOR               220
#define LCD_FILENAVIGATOR_Return        221
#define LCD_Selected_P0_RPM             210
#define LCD_Selected_P0_Time            211

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

void draw_menu_info();
void draw_menu_program();
void draw_menu_start_stop();
void draw_menu_select_custom();
void draw_menu_select_file();
void draw_info_screen();
void draw_file_navigator();
void draw_stating_screen();
void draw_stopping_screen();
void draw_navigator_return();
void draw_selected_custom_rpm();
void draw_selected_custom_time();

bool btnLeftUpdate();
bool btnCenterUpdate();
bool btnRightUpdate();

void navigator_update();

void SD_init(int cs);
void count_files();
bool open_if_folder();

char* get_next_line();
void read_file();
void set_program(char* p);
char* get_next_file_name();
char* get_prev_file_name();
void get_return_path();
void get_forward_path();

#endif