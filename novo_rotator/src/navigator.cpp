#include <SD.h>

#include "../include/navigator.h"

// Button pins
static const byte btnLeftPin = 5;
static const byte btnCenterPin = 6;
static const byte btnRightPin = 7;

unsigned long previousButtonMillisLeft = 0;    // time when button press last checked
unsigned long previousButtonMillisCenter = 0;  // time when button press last checked
unsigned long previousButtonMillisRight = 0;   // time when button press last checked

bool was_left_pressed = false;    // Button flag
bool was_center_pressed = false;  // Button flag
bool was_right_pressed = false;   // Button flag


// Menu state
byte lcdMenuState, lcdDisplayFlip, lcdStartingStopping, lcdContinuousUpdate;
String programSelected, programSelected_navigator, first_file;

// LCD variables
DFRobot_LCD lcd(16, 2);
unsigned long previousLcdMillis = 0;

// Symbol defs
byte symbolArrowRight[8] = {
    B00000,
    B00100,
    B00010,
    B11111,
    B00010,
    B00100,
    B00000,
    B00000};
byte symbolArrowLeft[8] = {
    B00000,
    B00100,
    B01000,
    B11111,
    B01000,
    B00100,
    B00000,
    B00000};
byte symbolArrowReturn[8] = {
    B11111,
    B00001,
    B00001,
    B00101,
    B01001,
    B11111,
    B01000,
    B00100};
// SD card program description variables
unsigned int selected_rpm = 30, selected_time = 150, custom_rpm = 0, custom_time = 0;
long timeElapsed = -1, timeInitiated = -1;

// SD card variables
const char loaded_program[64];
unsigned int file_index = 0;
unsigned int n_files = 0;
File root;
File entry;
char current_path[10][100], global_buff[256];
unsigned int current_path_count = 0;

// Freq
unsigned int lcd_curr_update_freq;

// Grbl
bool grbl_sendline = false;

void lcdInit() {
    // Initialize LCD
    lcd.init();
    lcd.customSymbol(3, symbolArrowReturn);
    lcd.customSymbol(2, symbolArrowRight);
    lcd.customSymbol(1, symbolArrowLeft);
    lcdMenuState = LCD_Menu_Info;
    lcdStartingStopping = LCD_SS_Unselected;
    lcdDisplayFlip = true;
    lcdContinuousUpdate = false;
    lcd_curr_update_freq = 999999;
    programSelected = String("       --       ");
    // Print a message to the LCD.
    /*
      lcd.setCursor(1, 0);
      lcd.print("Novo Rotator");
      lcd.setCursor(3, 1);
      lcd.print("FW: 0.0.1");
      delay(2500);
      lcd.clear();
      lcd.print("Starting...");
      lcd.setCursor(0, 1);
      lcd.print("Serial          ");
      */
}
void btnsInit() {
    pinMode(btnLeftPin, INPUT_PULLUP);
    pinMode(btnCenterPin, INPUT_PULLUP);
    pinMode(btnRightPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(btnLeftPin), ISR_btnLeftPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(btnCenterPin), ISR_btnCenterPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(btnRightPin), ISR_btnRightPress, FALLING);
}
void ISR_btnLeftPress() {
    if (millis() - previousButtonMillisLeft >= BUTTONUPDATEFREQ) {
        was_left_pressed = true;
        previousButtonMillisLeft = millis();
    }
}
void ISR_btnCenterPress() {
    if (millis() - previousButtonMillisCenter >= BUTTONUPDATEFREQ) {
        was_center_pressed = true;
        previousButtonMillisCenter = millis();
    }
}
void ISR_btnRightPress() {
    if (millis() - previousButtonMillisRight >= BUTTONUPDATEFREQ) {
        was_right_pressed = true;
        previousButtonMillisRight = millis();
    }
}

void draw_menu_info() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("      Info     ");
    lcd.write(2);
}
void draw_menu_program_select() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(1);
    lcd.print("    Program   ");
    lcd.setCursor(0, 1);
    lcd.print("     Select     ");
}
void draw_menu_start_stop() {
    if (lcdStartingStopping == LCD_SS_Unselected) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(1);
        lcd.print("  Start/Stop  ");
        lcd.write(2);
        lcd.setCursor(0, 1);
        lcd.print(programSelected);
    } else if (lcdStartingStopping == LCD_SS_SelectedIdle) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(1);
        lcd.print("     Start    ");
        lcd.write(2);
        lcd.setCursor(0, 1);
        lcd.print(programSelected);
    } else if (lcdStartingStopping == LCD_SS_SelectedActive || lcdStartingStopping == LCD_SS_SelectedPaused) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(1);
        lcd.print("     Stop     ");
        lcd.setCursor(0, 1);
        lcd.print(programSelected);
    }
}
void draw_menu_resume_pause()
{
    if (lcdStartingStopping == LCD_SS_SelectedPaused)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(1);
        lcd.print("    Resume    ");
        lcd.write(2);
        lcd.setCursor(0, 1);
        lcd.print(programSelected);
    }
    else if (lcdStartingStopping == LCD_SS_SelectedActive)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(1);
        lcd.print("     Pause    ");
        lcd.write(2);
        lcd.setCursor(0, 1);
        lcd.print(programSelected);
    }
}
void draw_menu_select_custom() {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(1);
        lcd.print("    Select    ");
        lcd.write(2);
        lcd.setCursor(0, 1);
        lcd.print("     Custom     ");
}
void draw_menu_select_file() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(1);
    lcd.print("    Select    ");
    lcd.setCursor(0, 1);
    lcd.print("      File      ");
}
void draw_info_screen(){
    String printString;
    if (lcdDisplayFlip == true) {
        lcd.clear();
        lcd.setCursor(0, 0);
        printString = String(" Current RPM:   ");
        lcd.print(printString);
        lcd.setCursor(0, 1);
        if (timeElapsed == -1) {
            printString = String("       --       ");
        } else {
            printString = String("      ") + String(selected_rpm) + String("       ");
        }
        lcd.print(printString);
        lcdDisplayFlip = false;
    } else {
        if (timeElapsed == -1) {
            printString = String("       --       ");
        } else {
            printString = String("     ") + String((millis() - timeElapsed) / 1000) + String("s/") + String(selected_time) + String("s ");
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(" Time elapsed:  ");
        lcd.setCursor(0, 1);
        lcd.print(printString);
        lcdDisplayFlip = true;
    }
}
void draw_file_navigator() {
    String printString = String(n_files) + String("   ") + String(file_index);
    Serial.println(printString);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(1);
    lcd.print("  Selecting:  ");
    if(file_index < n_files - 1)
        lcd.write(2);
    lcd.setCursor(0, 1);
    lcd.print(programSelected_navigator);
}
void draw_starting_screen() {
    timeElapsed = millis();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("    Starting    ");
    lcd.setCursor(0, 1);
    lcd.print("   Program...   ");
}
void draw_stopping_screen() {
    timeElapsed = -1;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("    Stopping    ");
    lcd.setCursor(0, 1);
    lcd.print("   Program...   ");
}
void draw_pausing_screen(){
    timeElapsed = -1;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("    Pausing    ");
    lcd.setCursor(0, 1);
    lcd.print("   Program...   ");
}
void draw_resuming_screen(){
    timeElapsed = -1;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("    Resuming    ");
    lcd.setCursor(0, 1);
    lcd.print("   Program...   ");
}
void draw_navigator_return(){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("     Return    ");
    lcd.write(2);
    lcd.setCursor(7, 1);
    lcd.write(3);
}
void draw_selected_custom_rpm(){
    String printString = String("      ") + String(custom_rpm) + String("       ");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("      RPM:      ");
    if(custom_rpm == 0){
        lcd.setCursor(1, 1);
    }
    else
    {
        lcd.setCursor(0, 1);
        lcd.write(1);
    }
    lcd.print(printString);
    lcd.setCursor(15, 1);
    lcd.write(2);
}
void draw_selected_custom_time(){
    String printString = String("      ") + String(custom_time) + String("       ");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("    Time(s):    ");
    if (custom_time == 0)
    {
        lcd.setCursor(1, 1);
    }
    else
    {
        lcd.setCursor(0, 1);
        lcd.write(1);
    }
    lcd.print(printString);
    lcd.setCursor(15, 1);
    lcd.write(2);
}

bool btnLeftUpdate() {
    if (!was_left_pressed) {
        return false; 
    }
    was_left_pressed = was_center_pressed = was_right_pressed = false;

    switch (lcdMenuState) {
        case LCD_Menu_ResumePause: {
            lcdMenuState = LCD_Menu_Info;
            break;
        }
        case LCD_Menu_StartStop: {
            if (lcdStartingStopping == LCD_SS_SelectedActive || lcdStartingStopping == LCD_SS_SelectedPaused)
                lcdMenuState = LCD_Menu_ResumePause;
            else
                lcdMenuState = LCD_Menu_Info;
            break;
        }
        case LCD_Menu_ProgramSelect: {
            lcdMenuState = LCD_Menu_StartStop;
            break;
        }
        case LCD_ProgramSelect_Pf: {
            lcdMenuState = LCD_ProgramSelect_P0;
            lcdDisplayFlip = false;
            break;
        }
        case LCD_FILENAVIGATOR: {
            //If the current file we are on is the first file, I want to go to the return screen
            if(file_index == 0)
                lcdMenuState = LCD_FILENAVIGATOR_Return;
            //Else go to the previous file in the same directory
            else 
                programSelected_navigator = get_prev_file_name();
            break;
        }
        case LCD_ProgramSelect_P0: {
            lcdMenuState = LCD_ProgramSelect_Return;
            break;
        }
        case LCD_Selected_P0_RPM:{
            if(custom_rpm == 0)
                return false;
            custom_rpm-=10;
            break;
        }
        case LCD_Selected_P0_Time:
        {
            if (custom_time == 0)
                return false;    
            custom_time -= 10;
            break;
        }
        default: {
            return false;
        }
    }
    return true;
}
bool btnRightUpdate() {
    if (!was_right_pressed) {
        return false;
    }
    was_left_pressed = was_center_pressed = was_right_pressed = false;

    switch (lcdMenuState) {
        case LCD_Menu_Info: {
            if(lcdStartingStopping == LCD_SS_SelectedActive || lcdStartingStopping == LCD_SS_SelectedPaused)
                lcdMenuState = LCD_Menu_ResumePause;
            else
                lcdMenuState = LCD_Menu_StartStop;
            break;
        }
        case LCD_Menu_StartStop: {
            if (lcdStartingStopping != LCD_SS_SelectedActive && lcdStartingStopping != LCD_SS_SelectedPaused) {
                lcdMenuState = LCD_Menu_ProgramSelect;
            }
            else {return false;}
            break;
        }
        case LCD_Menu_ResumePause: {
            lcdMenuState = LCD_Menu_StartStop;
            break;
        }
        case LCD_ProgramSelect_Return: {
            lcdMenuState = LCD_ProgramSelect_P0;
            break;
        }
        case LCD_ProgramSelect_P0: {
            lcdMenuState = LCD_ProgramSelect_Pf;
            break;
        }
        case LCD_FILENAVIGATOR: {
            if(file_index >= n_files - 1)
                return false;
            programSelected_navigator = get_next_file_name();
            break;
        }
        case LCD_FILENAVIGATOR_Return: {
            lcdMenuState = LCD_FILENAVIGATOR;
            break;
        }
        case LCD_Selected_P0_RPM:
        {
            custom_rpm += 10;
            break;
        }
        case LCD_Selected_P0_Time:
        {
            custom_time += 10;
            break;
        }
        default: {
            return false;
        }
    }
    return true;
}
bool btnCenterUpdate() {
    if (!was_center_pressed) {
        return false;
    }
    was_left_pressed = was_center_pressed = was_right_pressed = false;

    switch (lcdMenuState) {
        case LCD_Menu_Info: {
            lcdMenuState = LCD_Selected_Info;
            lcdDisplayFlip = false;
            lcdContinuousUpdate = true;
            lcd_curr_update_freq = INFOUPDATEFREQ;
            break;
        }
        case LCD_Menu_ResumePause: {
            if (lcdStartingStopping == LCD_SS_SelectedPaused) {
                lcdMenuState = LCD_Resuming;
                lcdStartingStopping = LCD_SS_SelectedActive;
                lcdDisplayFlip = false;
                lcdContinuousUpdate = true;
                lcd_curr_update_freq = SSUPDATEFREQ;
            }
            else if (lcdStartingStopping == LCD_SS_SelectedActive){
                lcdMenuState = LCD_Pausing;
                lcdStartingStopping = LCD_SS_SelectedPaused;
                lcdDisplayFlip = false;
                lcdContinuousUpdate = true;
                lcd_curr_update_freq = SSUPDATEFREQ;
            }
            break;
        }
        case LCD_Menu_StartStop: {
            if (lcdStartingStopping == LCD_SS_SelectedIdle) {
                lcdMenuState = LCD_Starting;
                lcdStartingStopping = LCD_SS_SelectedActive;
                lcdDisplayFlip = false;
                lcdContinuousUpdate = true;
                lcd_curr_update_freq = SSUPDATEFREQ;
            }
            else if (lcdStartingStopping == LCD_SS_SelectedActive || lcdStartingStopping == LCD_SS_SelectedPaused){
                lcdMenuState = LCD_Stopping;
                lcdStartingStopping = LCD_SS_SelectedIdle;
                lcdDisplayFlip = false;
                lcdContinuousUpdate = true;
                lcd_curr_update_freq = SSUPDATEFREQ;
            }
            else
                return false;
            break;
        }
        case LCD_Menu_ProgramSelect: {
            lcdMenuState = LCD_ProgramSelect_P0;
            break;
        }
        case LCD_ProgramSelect_Return: {
            lcdMenuState = LCD_Menu_ProgramSelect;
            break;
        }
        case LCD_Selected_Info: {
            lcdMenuState = LCD_Menu_Info;
            lcdDisplayFlip = false;
            lcdContinuousUpdate = false;
            break;
        }
        case LCD_ProgramSelect_P0: {
            lcdMenuState = LCD_Selected_P0_RPM;
            break;
        }
        case LCD_Selected_P0_RPM:{
            selected_rpm = custom_rpm;
            lcdMenuState = LCD_Selected_P0_Time;
            break;
        }
        case LCD_Selected_P0_Time:{
            selected_time = custom_time;
            lcdMenuState = LCD_Menu_StartStop;
            lcdStartingStopping = LCD_SS_SelectedIdle;
            programSelected = "     Custom     ";
            break;
        }
        case LCD_ProgramSelect_Pf: {
            lcdMenuState = LCD_FILENAVIGATOR;
            SD_init(4);
            root.close();
            entry.close();
            root = SD.open("/", FILE_READ);
            count_files();
            programSelected_navigator = get_next_file_name();
            file_index = 0;
            current_path_count = 0;
            break;
        }
        case LCD_FILENAVIGATOR:{
            if (open_if_folder()) {
                programSelected_navigator = get_next_file_name();
                file_index = 0;
            }
            else{
                lcdMenuState = LCD_Menu_StartStop;
                lcdStartingStopping = LCD_SS_SelectedIdle;
                programSelected = programSelected_navigator;
            }
            break;
        }
        case LCD_FILENAVIGATOR_Return:{
            if(current_path_count == 0){ //If I'm in the root of the sd card, take me back to the program select screen
                lcdMenuState = LCD_ProgramSelect_Pf;
            }
            else{ //Else take me back to the previous folder
                lcdMenuState = LCD_FILENAVIGATOR;
                get_return_path();
                root = SD.open(global_buff);
                count_files();
                programSelected_navigator = get_next_file_name();
                file_index = 0;
                current_path_count--;
            }
            break;
        }
        default: {
            return false;
        }
    }
    return true;
}

void navigator_update() {
    bool update = false;
    update += btnLeftUpdate();
	update += btnRightUpdate();
	update += btnCenterUpdate();
	
    if (!update && !(lcdContinuousUpdate && (millis() - previousLcdMillis > lcd_curr_update_freq))) {
		return;
	}

	switch (lcdMenuState) {
        case LCD_Menu_Info           :{draw_menu_info();break;}
        case LCD_Menu_ResumePause    :{draw_menu_resume_pause();break;}
        case LCD_Menu_StartStop      :{draw_menu_start_stop();break;}
        case LCD_Menu_ProgramSelect  :{draw_menu_program_select();break;}
        case LCD_Selected_Info       :{draw_info_screen();break;}
        case LCD_Starting:
        {
            if (update)
            {
                draw_starting_screen();
                grbl_sendline = true;
            }
            else
            {
                lcdMenuState = LCD_Selected_Info;
                lcdDisplayFlip = false;
                lcd_curr_update_freq = INFOUPDATEFREQ;
                draw_info_screen();
            }
            break;
        }
        case LCD_Stopping:
        {
            if (update)
            {
                draw_stopping_screen(); //TODO we need to rewind the file to the top, probably reopen it in order to start from scratch
            }
            else
            {
                lcdMenuState = LCD_Menu_StartStop;
                lcdContinuousUpdate = false;
                draw_menu_start_stop();
            }
            break;
        }
        case LCD_Resuming:
        {
            if (update)
            {
                draw_resuming_screen();
                grbl_write("~");
                grbl_sendline = true;
            }
            else
            {
                lcdMenuState = LCD_Selected_Info;
                lcdDisplayFlip = false;
                lcd_curr_update_freq = INFOUPDATEFREQ;
                draw_info_screen();
            }
            break;
        }
        case LCD_Pausing:
        {
            if (update)
            {
                draw_pausing_screen();
                grbl_write("!");
                grbl_sendline = false;
            }
            else
            {
                lcdMenuState = LCD_Menu_ResumePause;
                lcdContinuousUpdate = false;
                draw_menu_resume_pause();
            }
            break;
        }
        case LCD_ProgramSelect_P0       :{draw_menu_select_custom();break;} 
        case LCD_ProgramSelect_Pf       :{draw_menu_select_file();break;} 
        case LCD_ProgramSelect_Return   :{draw_navigator_return();break;}
        case LCD_FILENAVIGATOR          :{draw_file_navigator();break;}
        case LCD_FILENAVIGATOR_Return   :{draw_navigator_return();break;}
        case LCD_Selected_P0_RPM        :{draw_selected_custom_rpm();break;}
        case LCD_Selected_P0_Time       :{draw_selected_custom_time();break;}
        default                         :{lcdMenuState=LCD_Menu_Info;}
	}
	previousLcdMillis = millis();
}

void SD_init(int cs) {
    if (!SD.begin(cs)) {
        lcd.clear();
        lcd.print("Micro-SD failed");
        delay(2000);
    }
}

void count_files() {
    file_index = 0;
    n_files = -1;  // start at -1 to avoid off by one err.
    do {
        entry = root.openNextFile();
        n_files++;
    } while (entry);
    root.rewindDirectory();
}

bool open_if_folder() {
    if(!entry.isDirectory()) {return false;}
    
    strcpy(current_path[current_path_count],entry.name());
    current_path_count++;

    get_forward_path();
    root.close();
    entry.close();
    root = SD.open(global_buff, FILE_READ);
    //root = SD.open(entry.name());

    count_files();
    return true;
}

char* get_next_line() {
    return ("g1x1f25\n");
}

void read_file(){
    if (entry)
    {
        // read from the file until there's nothing else in it:
        while (entry.available())
        {
            Serial.write(entry.read());
        }
        // close the file:
        entry.close();
    }
    else
    {
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
    }
}

char* get_next_file_name() {
    if(file_index < n_files){
        entry.close();
        entry = root.openNextFile();
        file_index++;
    }   
    return entry.name();
}

char* get_prev_file_name() {
    root.rewindDirectory();
    if(file_index > 0)
        file_index--;
    for (int i = 0; i <= file_index; i++) {
        entry.close();
        entry = root.openNextFile();
    }
    return entry.name();
}

void set_program(char* p) {
    if (SD.exists(p)) {
        strcpy(p, loaded_program);
    } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.println("File read error ");
    }
}

void get_return_path() {
    String return_path;
    char buff[256];
    if (current_path_count < 2)
    {
        return_path = String('/');
    }
    else
    {
        for(int i = 0; i < current_path_count - 1; i++)
            return_path += String('/') + String(current_path[i]);
    }
    return_path.toCharArray(buff,256);
    Serial.println(buff);
    strcpy(global_buff, buff);
}

void get_forward_path() {
    String forward_path;
    char buff[256];
    for (int i = 0; i < current_path_count; i++)
        forward_path += String('/') + String(current_path[i]);
    forward_path.toCharArray(buff, 256);
    Serial.println(buff);
    strcpy(global_buff,buff);
}

bool read_file_line(String &grbl_line, bool grbl_ok){
    if(!grbl_sendline || !grbl_ok)
        return false;
    if (entry){
        char grbl_char;
        while(grbl_sendline){
            // Read next character from the file
            if (entry.available()){
                grbl_char = entry.read();
                //If the char is \n we need to switch the flag to false as to not send a new command until grbl responds
                //if(grbl_char == '\n')
                //    grbl_sendline = false;
                grbl_line += String(grbl_char);
            }
        }
        Serial.println(grbl_line);
        return true;
    }
    else{
        // if the file didn't open, print an error:
        Serial.println("Error opening file.");
        return false;
    }
}