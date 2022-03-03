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
byte lcdMenuState, lcdDisplayFlip, lcdStartingStopping;
String programSelected;

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

// SD card program description variables
unsigned int testRPM = 30, testTime = 150;
long timeElapsed = -1, timeInitiated = -1;

void lcdInit() {
    // Initialize LCD
    lcd.init();
    lcd.customSymbol(2, symbolArrowRight);
    lcd.customSymbol(1, symbolArrowLeft);
    lcdMenuState = LCD_Menu_Info;
    lcdStartingStopping = LCD_SS_Unselected;
    byte lcdDisplayFlip = true;
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
    lcd.write(1);
    lcd.print("     Info     ");
    lcd.write(2);
}
void draw_menu_program_select() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(1);
    lcd.print("    Program   ");
    lcd.write(2);
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
    } else if (lcdStartingStopping == LCD_SS_SelectedActive) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(1);
        lcd.print("     Stop     ");
        lcd.write(2);
        lcd.setCursor(0, 1);
        lcd.print(programSelected);
    }
}
void draw_menu_select_custom() {
    if (lcdDisplayFlip == true) {
        String printString;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(1);
        printString = String(" RPM:   ") + String(testRPM) + String("  ");
        lcd.print(printString);
        lcd.write(2);
        lcd.setCursor(0, 1);
        printString = String("  Time:  ") + String(testTime) + String("  ");
        lcd.print(printString);
        lcdDisplayFlip = false;
    } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(1);
        lcd.print("    Select    ");
        lcd.write(2);
        lcd.setCursor(0, 1);
        lcd.print("     Custom     ");
        lcdDisplayFlip = true;
    }
}
void draw_menu_select_file() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(1);
    lcd.print("    Select    ");
    lcd.write(2);
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
            printString = String("      ") + String(testRPM) + String("       ");
        }
        lcd.print(printString);
        lcdDisplayFlip = false;
    } else {
        if (timeElapsed == -1) {
            printString = String("       --       ");
        } else {
            printString = String("     ") + String((millis() - timeElapsed) / 1000) + String("s/") + String(testTime) + String("s ");
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
    //programSelected = String(get_prev_file_name());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.write(1);
    lcd.print("  Selecting:  ");
    lcd.write(2);
    lcd.setCursor(0, 1);
    lcd.print(programSelected);
    lcdDisplayFlip = true;
    previousLcdMillis = millis();
}
void draw_starting_screen() {
    timeElapsed = millis();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("    Starting    ");
    lcd.setCursor(0, 1);
    lcd.print("   Program...   ");
    previousLcdMillis = millis();
}
void draw_stopping_screen() {
    timeElapsed = -1;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("    Stopping    ");
    lcd.setCursor(0, 1);
    lcd.print("   Program...   ");
    previousLcdMillis = millis();
}

bool btnLeftUpdate() {
    if (!was_left_pressed) {
        return false; 
    }
    was_left_pressed = was_center_pressed = was_right_pressed = false;

    switch (lcdMenuState) {
        case LCD_Menu_Info: {
            // should not roll over
        }
        case LCD_Menu_StartStop: {
            lcdMenuState = LCD_Menu_Info;
            break;
        }
        case LCD_Menu_ProgramSelect: {
            lcdMenuState = LCD_Menu_StartStop;
            break;
        }
        case LCD_ProgramSelect_P0: {
            // nothing - don't roll over.
            break;
        }
        case LCD_ProgramSelect_Pf: {
            lcdMenuState = LCD_ProgramSelect_P0;
            break;
        }
        case LCD_FILENAVIGATOR:{
            programSelected = get_prev_file_name();
            break;
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
            lcdMenuState = LCD_Menu_StartStop;
            break;
        }
        case LCD_Menu_StartStop: {
            if (lcdStartingStopping != LCD_SS_SelectedActive) {
                lcdMenuState = LCD_Menu_ProgramSelect;
            }
            break;
        }
        case LCD_Menu_ProgramSelect: {
            // nothing - don't roll over
            break;
        }
        case LCD_ProgramSelect_P0: {
            lcdMenuState = LCD_ProgramSelect_Pf;
            lcdDisplayFlip = false;
            break;
        }
        case LCD_ProgramSelect_Pf: {
            lcdDisplayFlip = false;
            // nothing - don't roll over
            break;
        }
        case LCD_FILENAVIGATOR:{
            programSelected = get_next_file_name();
            break;
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
            break;
        }
        case LCD_Menu_StartStop: {
            if (lcdStartingStopping == LCD_SS_SelectedIdle) {
                lcdMenuState = LCD_Starting;
                lcdStartingStopping = LCD_SS_SelectedActive;

            } else if (lcdStartingStopping == LCD_SS_SelectedActive) {
                lcdMenuState = LCD_Stopping;
                lcdStartingStopping = LCD_SS_SelectedIdle;
            }
            break;
        }
        case LCD_Menu_ProgramSelect: {
            lcdMenuState = LCD_ProgramSelect_P0;
            break;
        }
        case LCD_Selected_Info: {
            lcdMenuState = LCD_Menu_Info;
            break;
        }
        case LCD_ProgramSelect_P0: {
            lcdMenuState = LCD_Menu_StartStop; // todo insert state for customizing the params befare going to start stop screen. 
            lcdStartingStopping = LCD_SS_SelectedIdle;
            break;
        }
        case LCD_ProgramSelect_Pf: {
            lcdMenuState = LCD_FILENAVIGATOR;
            lcdStartingStopping = LCD_SS_SelectedIdle;
            break;
        }
        case LCD_FILENAVIGATOR:{
            char buff[64];
            programSelected.toCharArray(buff, 64);

            if (open_if_folder(buff)) {
                programSelected = get_next_file_name();
                break;
            }
            else{
                lcdMenuState=LCD_Menu_StartStop;
            }
        }
    }
    return true;
}

void navigator_update() {
    bool update = false;
    update += btnLeftUpdate();
	update += btnRightUpdate();
	update += btnCenterUpdate();
	
    if (!update && millis() - previousLcdMillis < LCDUPDATEFREQ) {
		return;
	}

	switch (lcdMenuState) {
        case LCD_Menu_Info           :{draw_menu_info();break;}
        case LCD_Menu_StartStop      :{draw_menu_start_stop();break;}
        case LCD_Menu_ProgramSelect  :{draw_menu_program_select();break;}
        case LCD_Selected_Info       :{draw_info_screen();break;}
        case LCD_Starting            :{draw_starting_screen();lcdMenuState = LCD_Selected_Info;break;}
        case LCD_Stopping            :{draw_stopping_screen();lcdMenuState = LCD_Menu_StartStop;break;}
        case LCD_ProgramSelect_P0    :{draw_menu_select_custom();break;} // todo create menu where you can select rpm and time
        case LCD_ProgramSelect_Pf    :{draw_menu_select_file();break;} 
        case LCD_FILENAVIGATOR       :{draw_file_navigator();break;}
        default:{
            lcdMenuState=LCD_Menu_Info;
        }
	}
	previousLcdMillis = millis();
}


const char loaded_program[64];
unsigned int file_index = 0;
unsigned int n_files = 0;
File root;
File entry;

void SD_init(int cs) {
    if (!SD.begin(cs)) {
        lcd.clear();
        lcd.print("Micro-SD failed");
        delay(2000);
    }
    root = SD.open("/", FILE_READ);
    count_files();
}

void count_files() {
    root.rewindDirectory();
    n_files = -1;  // start at -1 to avoid off by one err.
    do {
        entry = root.openNextFile();
        n_files++;
    } while (entry);
    root.rewindDirectory();
    file_index=0;
}

bool open_if_folder(char* p) {
    if (!SD.exists(p)) {return false;}
    File tmp = SD.open(p);
    if (!tmp.isDirectory()) {return false;}
    tmp.close();
    root = SD.open(p);
    count_files();
    return true;
}

char* get_next_line() {
    return ("g1x1f25\n");
}

char* get_next_file_name() {
    if(file_index < n_files){
        entry = root.openNextFile();
        file_index++;
    }   
    return entry.name();
}

char* get_prev_file_name() {
    if (file_index > 0) {
        file_index--;
    }
    root.rewindDirectory();
    for (int i = 0; i < file_index; i++) {
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