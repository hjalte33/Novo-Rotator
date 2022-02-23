#include "../include/lcd_control.h"

// Button pins
static const byte btnLeftPin = 2;
static const byte btnCenterPin = 18;
static const byte btnRightPin = 19;

static const int buttonInterval = 300; // number of millisecs between button readings
unsigned long previousButtonMillisLeft = 0; // time when button press last checked
unsigned long previousButtonMillisCenter = 0; // time when button press last checked
unsigned long previousButtonMillisRight = 0; // time when button press last checked

volatile byte btnLeftState = LOW; //Button flag, HIGH = pressed, LOW = not pressed.
volatile byte btnCenterState = LOW; //Button flag, HIGH = pressed, LOW = not pressed.
volatile byte btnRightState = LOW; //Button flag, HIGH = pressed, LOW = not pressed.

// Menu state
byte lcdMenuState = 0;
byte programSelected = -1;

// LCD variable
DFRobot_LCD lcd(16,2);

// Symbol defs
byte symbolArrowRight[8] = {
  B00000,
  B00100,
  B00010,
  B11111,
  B00010,
  B00100,
  B00000,
  B00000
};
byte symbolArrowLeft[8] = {
  B00000,
  B00100,
  B01000,
  B11111,
  B01000,
  B00100,
  B00000,
  B00000
};

void lcdInit()
{
    // Initialize LCD
    lcd.init();
    lcd.customSymbol(2, symbolArrowRight);
    lcd.customSymbol(1, symbolArrowLeft);
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

void btnsInit()
{
    pinMode(btnLeftPin, INPUT_PULLUP);
    pinMode(btnCenterPin, INPUT_PULLUP);
    pinMode(btnRightPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(btnLeftPin), btnLeftPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(btnCenterPin), btnCenterPress, FALLING);
    attachInterrupt(digitalPinToInterrupt(btnRightPin), btnRightPress, FALLING);
}

void lcdStartMenu()
{
  lcdMenuState = LCD_Menu_Info;
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.write(1);
  lcd.print("    Info    ");
  lcd.write(2);
}

void btnLeftPress() {
  if (millis() - previousButtonMillisLeft >= buttonInterval) {
      btnLeftState = HIGH;
    }
   previousButtonMillisLeft = millis();
  }
  
void btnCenterPress() {
  if (millis() - previousButtonMillisCenter >= buttonInterval) {
      btnCenterState = HIGH;
    }
   previousButtonMillisCenter = millis();
  }

void btnRightPress() {
  if (millis() - previousButtonMillisRight >= buttonInterval) {
      btnRightState = HIGH;
    }
   previousButtonMillisRight = millis();
  }

void btnLeftUpdate() {
  if (btnLeftState == HIGH)
  {
    switch (lcdMenuState)
    {
    case LCD_Menu_Info:
      lcdMenuState = LCD_Menu_ProgramSelect;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print("  Program   ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("   Select     ");
      break;
    case LCD_Menu_ProgramSelect:
      lcdMenuState = LCD_Menu_StartStop;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print("   Start/   ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("    Stop     ");
      break;
    case LCD_Menu_StartStop:
      lcdMenuState = LCD_Menu_Info;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print("    Info    ");
      lcd.write(2);
      break;
    case LCD_Selected_ProgramSelect_P0:
      lcdMenuState = LCD_Selected_ProgramSelect_P5;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("  Program 5   ");
      break;
    case LCD_Selected_ProgramSelect_P1:
      lcdMenuState = LCD_Selected_ProgramSelect_P0;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("   Custom     ");
      break;
    case LCD_Selected_ProgramSelect_P2:
      lcdMenuState = LCD_Selected_ProgramSelect_P1;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("  Program 1   ");
      break;
    case LCD_Selected_ProgramSelect_P3:
      lcdMenuState = LCD_Selected_ProgramSelect_P2;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("  Program 2   ");
      break;
    case LCD_Selected_ProgramSelect_P4:
      lcdMenuState = LCD_Selected_ProgramSelect_P3;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("  Program 3   ");
      break;
    case LCD_Selected_ProgramSelect_P5:
      lcdMenuState = LCD_Selected_ProgramSelect_P4;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("  Program 4   ");
      break;
    }
    btnLeftState = btnCenterState = btnRightState = LOW;
  }
}

void btnRightUpdate() {
  if (btnRightState == HIGH)
  {
    switch (lcdMenuState)
    {
    case LCD_Menu_Info:
      lcdMenuState = LCD_Menu_StartStop;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print("   Start/   ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("    Stop     ");
      break;
    case LCD_Menu_ProgramSelect:
      lcdMenuState = LCD_Menu_Info;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print("   Info     ");
      lcd.write(2);
      break;
    case LCD_Menu_StartStop:
      lcdMenuState = LCD_Menu_ProgramSelect;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print("  Program   ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("   Select     ");
      break;
    case LCD_Selected_ProgramSelect_P0:
      lcdMenuState = LCD_Selected_ProgramSelect_P1;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("  Program 1   ");
      break;
    case LCD_Selected_ProgramSelect_P1:
      lcdMenuState = LCD_Selected_ProgramSelect_P2;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("  Program 2   ");
      break;
    case LCD_Selected_ProgramSelect_P2:
      lcdMenuState = LCD_Selected_ProgramSelect_P3;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("  Program 3   ");
      break;
    case LCD_Selected_ProgramSelect_P3:
      lcdMenuState = LCD_Selected_ProgramSelect_P4;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("  Program 4   ");
      break;
    case LCD_Selected_ProgramSelect_P4:
      lcdMenuState = LCD_Selected_ProgramSelect_P5;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("  Program 5   ");
      break;
    case LCD_Selected_ProgramSelect_P5:
      lcdMenuState = LCD_Selected_ProgramSelect_P0;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("   Custom     ");
      break;
    }
    btnLeftState = btnCenterState = btnRightState = LOW;
  }
}

void btnCenterUpdate() {
  if (btnCenterState == HIGH)
  {
    switch (lcdMenuState)
    {
    case LCD_Menu_Info:

      break;
    case LCD_Menu_ProgramSelect:
      lcdMenuState = LCD_Selected_ProgramSelect_P0;
      lcd.clear();
      lcd.setCursor(1,0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1,1);
      lcd.print("   Custom     ");
      break;
    }
    btnLeftState = btnCenterState = btnRightState = LOW;
  }
}
