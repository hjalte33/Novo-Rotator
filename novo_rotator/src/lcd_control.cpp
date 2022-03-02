#include "../include/lcd_control.h"

// Button pins
static const byte btnLeftPin = 5;
static const byte btnCenterPin = 6;
static const byte btnRightPin = 7;

unsigned long previousButtonMillisLeft = 0;   // time when button press last checked
unsigned long previousButtonMillisCenter = 0; // time when button press last checked
unsigned long previousButtonMillisRight = 0;  // time when button press last checked

volatile byte btnLeftState = LOW;   //Button flag, HIGH = pressed, LOW = not pressed.
volatile byte btnCenterState = LOW; //Button flag, HIGH = pressed, LOW = not pressed.
volatile byte btnRightState = LOW;  //Button flag, HIGH = pressed, LOW = not pressed.

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

void lcdInit()
{
  // Initialize LCD
  lcd.init();
  lcd.customSymbol(2, symbolArrowRight);
  lcd.customSymbol(1, symbolArrowLeft);
  lcdMenuState = LCD_Menu_Info;
  lcdStartingStopping = LCD_SS_Unselected;
  byte lcdDisplayFlip = HIGH;
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

void btnsInit()
{
  pinMode(btnLeftPin, INPUT_PULLUP);
  pinMode(btnCenterPin, INPUT_PULLUP);
  pinMode(btnRightPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(btnLeftPin), ISR_btnLeftPress, FALLING);
  attachInterrupt(digitalPinToInterrupt(btnCenterPin), ISR_btnCenterPress, FALLING);
  attachInterrupt(digitalPinToInterrupt(btnRightPin), ISR_btnRightPress, FALLING);
}

void lcdStartMenu()
{
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.write(1);
  lcd.print("    Info    ");
  lcd.write(2);
}

void ISR_btnLeftPress()
{
  if (millis() - previousButtonMillisLeft >= BUTTONUPDATEFREQ)
  {
    btnLeftState = HIGH;
    previousButtonMillisLeft = millis();
  }
}

void ISR_btnCenterPress()
{
  if (millis() - previousButtonMillisCenter >= BUTTONUPDATEFREQ)
  {
    btnCenterState = HIGH;
    previousButtonMillisCenter = millis();
  }
}

void ISR_btnRightPress()
{
  if (millis() - previousButtonMillisRight >= BUTTONUPDATEFREQ)
  {
    btnRightState = HIGH;
    previousButtonMillisRight = millis();
  }
}

void btnLeftUpdate()
{
  if (btnLeftState == HIGH)
  {
    switch (lcdMenuState)
    {
    case LCD_Menu_Info:
    {
      lcdMenuState = LCD_Menu_ProgramSelect;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print("   Program  ");
      lcd.write(2);
      lcd.setCursor(1, 1);
      lcd.print("    Select    ");
      break;
    }
    case LCD_Menu_ProgramSelect:
    {
      lcdMenuState = LCD_Menu_StartStop;
      if (lcdStartingStopping == LCD_SS_Unselected)
      {
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.write(1);
        lcd.print(" Start/Stop ");
        lcd.write(2);
        lcd.setCursor(0, 1);
        lcd.print(programSelected);
      }
      else if (lcdStartingStopping == LCD_SS_SelectedIdle)
      {
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.write(1);
        lcd.print("    Start   ");
        lcd.write(2);
        lcd.setCursor(0, 1);
        lcd.print(programSelected);
      }
      else if (lcdStartingStopping == LCD_SS_SelectedActive)
      {
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.write(1);
        lcd.print("    Stop    ");
        lcd.write(2);
        lcd.setCursor(0, 1);
        lcd.print(programSelected);
      }
      break;
    }
    case LCD_Menu_StartStop:
    {
      lcdMenuState = LCD_Menu_Info;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print("    Info    ");
      lcd.write(2);
      break;
    }
    case LCD_Selected_ProgramSelect_P0:
    {
      lcdMenuState = LCD_Selected_ProgramSelect_P5;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1, 1);
      lcd.print("  Program 5   ");
      lcdDisplayFlip = HIGH;
      previousLcdMillis = millis();
      break;
    }
    case LCD_Selected_ProgramSelect_P1:
    {
      lcdMenuState = LCD_Selected_ProgramSelect_P0;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1, 1);
      lcd.print("   Custom     ");
      lcdDisplayFlip = HIGH;
      previousLcdMillis = millis();
      break;
    }
    case LCD_Selected_ProgramSelect_P2:
    {
      lcdMenuState = LCD_Selected_ProgramSelect_P1;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1, 1);
      lcd.print("  Program 1   ");
      lcdDisplayFlip = HIGH;
      previousLcdMillis = millis();
      break;
    }
    case LCD_Selected_ProgramSelect_P3:
    {
      lcdMenuState = LCD_Selected_ProgramSelect_P2;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1, 1);
      lcd.print("  Program 2   ");
      lcdDisplayFlip = HIGH;
      previousLcdMillis = millis();
      break;
    }
    case LCD_Selected_ProgramSelect_P4:
    {
      lcdMenuState = LCD_Selected_ProgramSelect_P3;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1, 1);
      lcd.print("  Program 3   ");
      lcdDisplayFlip = HIGH;
      previousLcdMillis = millis();
      break;
    }
    case LCD_Selected_ProgramSelect_P5:
    {
      lcdMenuState = LCD_Selected_ProgramSelect_P4;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1, 1);
      lcd.print("  Program 4   ");
      lcdDisplayFlip = HIGH;
      previousLcdMillis = millis();
      break;
    }
    }
    btnLeftState = btnCenterState = btnRightState = LOW;
  }
}

void btnRightUpdate()
{
  if (btnRightState == HIGH)
  {
    switch (lcdMenuState)
    {
    case LCD_Menu_Info:
    {
      lcdMenuState = LCD_Menu_StartStop;
      if (lcdStartingStopping == LCD_SS_Unselected)
      {
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.write(1);
        lcd.print(" Start/Stop ");
        lcd.write(2);
        lcd.setCursor(0, 1);
        lcd.print(programSelected);
      }
      else if (lcdStartingStopping == LCD_SS_SelectedIdle)
      {
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.write(1);
        lcd.print("    Start   ");
        lcd.write(2);
        lcd.setCursor(0, 1);
        lcd.print(programSelected);
      }
      else if (lcdStartingStopping == LCD_SS_SelectedActive)
      {
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.write(1);
        lcd.print("    Stop    ");
        lcd.write(2);
        lcd.setCursor(0, 1);
        lcd.print(programSelected);
      }
      break;
    }
    case LCD_Menu_ProgramSelect:
    {
      lcdMenuState = LCD_Menu_Info;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print("    Info    ");
      lcd.write(2);
      break;
    }
    case LCD_Menu_StartStop:
    {
      lcdMenuState = LCD_Menu_ProgramSelect;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print("   Program  ");
      lcd.write(2);
      lcd.setCursor(1, 1);
      lcd.print("    Select    ");
      break;
    }
    case LCD_Selected_ProgramSelect_P0:
    {
      lcdMenuState = LCD_Selected_ProgramSelect_P1;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1, 1);
      lcd.print("  Program 1   ");
      lcdDisplayFlip = HIGH;
      previousLcdMillis = millis();
      break;
    }
    case LCD_Selected_ProgramSelect_P1:
    {
      lcdMenuState = LCD_Selected_ProgramSelect_P2;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1, 1);
      lcd.print("  Program 2   ");
      lcdDisplayFlip = HIGH;
      previousLcdMillis = millis();
      break;
    }
    case LCD_Selected_ProgramSelect_P2:
    {
      lcdMenuState = LCD_Selected_ProgramSelect_P3;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1, 1);
      lcd.print("  Program 3   ");
      lcdDisplayFlip = HIGH;
      previousLcdMillis = millis();
      break;
    }
    case LCD_Selected_ProgramSelect_P3:
    {
      lcdMenuState = LCD_Selected_ProgramSelect_P4;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1, 1);
      lcd.print("  Program 4   ");
      lcdDisplayFlip = HIGH;
      previousLcdMillis = millis();
      break;
    }
    case LCD_Selected_ProgramSelect_P4:
    {
      lcdMenuState = LCD_Selected_ProgramSelect_P5;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1, 1);
      lcd.print("  Program 5   ");
      lcdDisplayFlip = HIGH;
      previousLcdMillis = millis();
      break;
    }
    case LCD_Selected_ProgramSelect_P5:
    {
      lcdMenuState = LCD_Selected_ProgramSelect_P0;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1, 1);
      lcd.print("   Custom     ");
      break;
    }
    }
    btnLeftState = btnCenterState = btnRightState = LOW;
  }
}

void btnCenterUpdate()
{
  if (btnCenterState == HIGH)
  {
    switch (lcdMenuState)
    {
    case LCD_Menu_Info:
    {
      String printString;
      lcdMenuState = LCD_Selected_Info;
      if (timeElapsed == -1)
      {
        printString = String("       --       ");
      }
      else
      {
        printString = String("   ") + String((millis() - timeElapsed) / 1000) + String("s/") + String(testTime) + String("s ");
      }
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print(" Time elapsed:  ");
      lcd.setCursor(0, 1);
      lcd.print(printString);
      lcdDisplayFlip = HIGH;
      previousLcdMillis = millis();
      break;
    }
    case LCD_Menu_StartStop:
    {
      if (lcdStartingStopping == LCD_SS_SelectedIdle)
      {
        lcdMenuState = LCD_Starting;
        lcdStartingStopping = LCD_SS_SelectedActive;
        timeElapsed = millis();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("    Starting    ");
        lcd.setCursor(0, 1);
        lcd.print("   Program...   ");
        previousLcdMillis = millis();
      }
      else if (lcdStartingStopping == LCD_SS_SelectedActive)
      {
        lcdMenuState = LCD_Stopping;
        lcdStartingStopping = LCD_SS_SelectedIdle;
        timeElapsed = -1;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("    Stopping    ");
        lcd.setCursor(0, 1);
        lcd.print("   Program...   ");
        previousLcdMillis = millis();
      }
      break;
    }
    case LCD_Menu_ProgramSelect:
    {
      lcdMenuState = LCD_Selected_ProgramSelect_P0;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print(" Selecting: ");
      lcd.write(2);
      lcd.setCursor(1, 1);
      lcd.print("   Custom     ");
      lcdDisplayFlip = HIGH;
      break;
    }
    case LCD_Selected_Info:
    {
      lcdMenuState = LCD_Menu_Info;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print("    Info    ");
      lcd.write(2);
      break;
    }
    case LCD_Selected_ProgramSelect_P0:
    {
      lcdMenuState = LCD_Menu_StartStop;
      programSelected = String("     Custom    ");
      lcdStartingStopping = LCD_SS_SelectedIdle;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print("    Start   ");
      lcd.write(2);
      lcd.setCursor(0, 1);
      lcd.print(programSelected);
      break;
    }
    case LCD_Selected_ProgramSelect_P1:
    {
      lcdMenuState = LCD_Menu_StartStop;
      programSelected = String("   Program 1   ");
      lcdStartingStopping = LCD_SS_SelectedIdle;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print("    Start   ");
      lcd.write(2);
      lcd.setCursor(0, 1);
      lcd.print(programSelected);
      break;
    }
    case LCD_Selected_ProgramSelect_P2:
    {
      lcdMenuState = LCD_Menu_StartStop;
      programSelected = String("   Program 2   ");
      lcdStartingStopping = LCD_SS_SelectedIdle;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print("    Start   ");
      lcd.write(2);
      lcd.setCursor(0, 1);
      lcd.print(programSelected);
      break;
    }
    case LCD_Selected_ProgramSelect_P3:
    {
      lcdMenuState = LCD_Menu_StartStop;
      programSelected = String("   Program 3   ");
      lcdStartingStopping = LCD_SS_SelectedIdle;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print("    Start   ");
      lcd.write(2);
      lcd.setCursor(0, 1);
      lcd.print(programSelected);
      break;
    }
    case LCD_Selected_ProgramSelect_P4:
    {
      lcdMenuState = LCD_Menu_StartStop;
      programSelected = String("   Program 4   ");
      lcdStartingStopping = LCD_SS_SelectedIdle;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print("    Start   ");
      lcd.write(2);
      lcd.setCursor(0, 1);
      lcd.print(programSelected);
      break;
    }
    case LCD_Selected_ProgramSelect_P5:
    {
      lcdMenuState = LCD_Menu_StartStop;
      programSelected = String("   Program 5   ");
      lcdStartingStopping = LCD_SS_SelectedIdle;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print("    Start   ");
      lcd.write(2);
      lcd.setCursor(0, 1);
      lcd.print(programSelected);
      break;
    }
    }
    btnLeftState = btnCenterState = btnRightState = LOW;
  }
}

void lcdUpdate()
{
  if (millis() - previousLcdMillis >= LCDUPDATEFREQ)
  {
    switch (lcdMenuState)
    {
    case LCD_Selected_Info:
    {
      String printString;
      if (lcdDisplayFlip == HIGH)
      {
        lcd.clear();
        lcd.setCursor(1, 0);
        printString = String(" Current RPM:   ");
        lcd.print(printString);
        lcd.setCursor(0, 1);
        if (timeElapsed == -1)
        {
          printString = String("       --       ");
        }
        else
        {
          printString = String("      ") + String(testRPM) + String("       ");
        }
        lcd.print(printString);
        lcdDisplayFlip = LOW;
      }
      else
      {
        if (timeElapsed == -1)
        {
          printString = String("       --       ");
        }
        else
        {
          printString = String("     ") + String((millis() - timeElapsed) / 1000) + String("s/") + String(testTime) + String("s ");
        }
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print(" Time elapsed:  ");
        lcd.setCursor(0, 1);
        lcd.print(printString);
        lcdDisplayFlip = HIGH;
      }
      break;
    }
    case LCD_Starting:
    {
      String printString;
      lcdMenuState = LCD_Selected_Info;
      printString = String("     ") + String((millis() - timeElapsed) / 1000) + String("s/") + String(testTime) + String("s ");
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print(" Time elapsed:  ");
      lcd.setCursor(0, 1);
      lcd.print(printString);
      lcdDisplayFlip = HIGH;
      break;
    }
    case LCD_Stopping:
    {
      lcdMenuState = LCD_Menu_StartStop;
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.write(1);
      lcd.print("    Start   ");
      lcd.write(2);
      lcd.setCursor(0, 1);
      lcd.print(programSelected);
      break;
    }
    case LCD_Selected_ProgramSelect_P1:
    {
      if (lcdDisplayFlip == HIGH)
      {
        String printString;
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.write(1);
        printString = String(" RPM:   ") + String(testRPM) + String("  ");
        lcd.print(printString);
        lcd.write(2);
        lcd.setCursor(1, 1);
        printString = String("  Time:  ") + String(testTime) + String("  ");
        lcd.print(printString);
        lcdDisplayFlip = LOW;
      }
      else
      {
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.write(1);
        lcd.print(" Selecting: ");
        lcd.write(2);
        lcd.setCursor(1, 1);
        lcd.print("  Program 1   ");
        lcdDisplayFlip = HIGH;
      }
      break;
    }
    case LCD_Selected_ProgramSelect_P2:
    {
      if (lcdDisplayFlip == HIGH)
      {
        String printString;
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.write(1);
        printString = String(" RPM:   ") + String(testRPM * 2) + String("  ");
        lcd.print(printString);
        lcd.write(2);
        lcd.setCursor(1, 1);
        printString = String("  Time:  ") + String(testTime + 10) + String("  ");
        lcd.print(printString);
        lcdDisplayFlip = LOW;
      }
      else
      {
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.write(1);
        lcd.print(" Selecting: ");
        lcd.write(2);
        lcd.setCursor(1, 1);
        lcd.print("  Program 2   ");
        lcdDisplayFlip = HIGH;
      }
      break;
    }
    case LCD_Selected_ProgramSelect_P3:
    {
      if (lcdDisplayFlip == HIGH)
      {
        String printString;
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.write(1);
        printString = String(" RPM:   ") + String(testRPM * 2) + String("  ");
        lcd.print(printString);
        lcd.write(2);
        lcd.setCursor(1, 1);
        printString = String("  Time:  ") + String(testTime + 10) + String("  ");
        lcd.print(printString);
        lcdDisplayFlip = LOW;
      }
      else
      {
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.write(1);
        lcd.print(" Selecting: ");
        lcd.write(2);
        lcd.setCursor(1, 1);
        lcd.print("  Program 3   ");
        lcdDisplayFlip = HIGH;
      }
      break;
    }
    case LCD_Selected_ProgramSelect_P4:
    {
      if (lcdDisplayFlip == HIGH)
      {
        String printString;
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.write(1);
        printString = String(" RPM:   ") + String(testRPM * 2) + String("  ");
        lcd.print(printString);
        lcd.write(2);
        lcd.setCursor(1, 1);
        printString = String("  Time:  ") + String(testTime + 10) + String("  ");
        lcd.print(printString);
        lcdDisplayFlip = LOW;
      }
      else
      {
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.write(1);
        lcd.print(" Selecting: ");
        lcd.write(2);
        lcd.setCursor(1, 1);
        lcd.print("  Program 4   ");
        lcdDisplayFlip = HIGH;
      }
      break;
    }
    case LCD_Selected_ProgramSelect_P5:
    {
      if (lcdDisplayFlip == HIGH)
      {
        String printString;
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.write(1);
        printString = String(" RPM:   ") + String(testRPM * 2) + String("  ");
        lcd.print(printString);
        lcd.write(2);
        lcd.setCursor(1, 1);
        printString = String("  Time:  ") + String(testTime + 10) + String("  ");
        lcd.print(printString);
        lcdDisplayFlip = LOW;
      }
      else
      {
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.write(1);
        lcd.print(" Selecting: ");
        lcd.write(2);
        lcd.setCursor(1, 1);
        lcd.print("  Program 5   ");
        lcdDisplayFlip = HIGH;
      }
      break;
    }
    }
    previousLcdMillis = millis();
  }
}
