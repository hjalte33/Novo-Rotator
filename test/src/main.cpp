#include <Arduino.h>
#include <Ethernet.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <helpers.h>
#include <stdint.h>
#include <stdlib.h>

#include "grbl_talker.h"

#include "DFRobot_LCD.h"

#define BUF_LEN 128
#define LCDUPDATEFREQUENCY 1000
#define TCPUPDATEFREQUENCY 100
#define BTN_L 5
#define BTN_C 6
#define BTN_R A2
#define ETH_CS 10
#define SD_CS 4

DFRobot_LCD lcd(16, 2);  // 16 characters and 2 lines of show

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
// The IP address will be dependent on your local network:
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte ip[] = {10, 8, 0, 101};
byte subnet[] = {255, 0, 0, 0};
byte gateway[] = {10, 8, 0, 1};
EthernetServer ethServer(5001);

// global variables
byte test, start;
static char inData[BUF_LEN];
static char message[BUF_LEN];
bool messageDismissed = true;
float rotations;
unsigned long lastTick = 0;
int tickCounter = 0;
bool grblOkAwait = true;


// settings
byte rpm = 0;
unsigned long destinationTime = 0;
unsigned long consumedTime = 0;


unsigned long lastLCDUpdate = 0;
void updateLCD() {
    if (millis() < lastLCDUpdate + LCDUPDATEFREQUENCY)
        return;
    lastLCDUpdate = millis();

    switch (grbl_state) {
        case STATE_ALARM:
            lcd.setCursor(0, 0);
            lcd.println("Err             ");
            lcd.setCursor(0, 1);
            lcd.println("                ");
            lcd.setRGB(255, 10, 10);
            break;
        case STATE_IDLE:
            lcd.setCursor(0, 0);
            lcd.println("Idle            ");
            lcd.setCursor(0, 1);
            lcd.println("                ");
            lcd.setRGB(50, 50, 255);
            break;
        case STATE_CYCLE:
            lcd.setCursor(0, 0);
            lcd.print("Run  t=");
            lcd.print(consumedTime);
            lcd.setCursor(0, 1);
            lcd.println("                ");
            lcd.setRGB(50, 255, 50);
            break;
        default:
            break;
    }
}

void breath(unsigned char color) {
    for (int i = 0; i < 255; i++) {
        lcd.setPWM(color, i);
        delay(5);
    }

    delay(500);
    for (int i = 254; i >= 0; i--) {
        lcd.setPWM(color, i);
        delay(5);
    }

    delay(500);
}



void setup() {
    // Setp pins

    Wire.begin();
    // initialize LCD
    lcd.init();
    // Print a message to the LCD.
    lcd.setCursor(1, 0);
    lcd.print("Novo Rotator");
    lcd.setCursor(3, 1);
    lcd.print("FW: 0.0.1");
    delay(2500);
    lcd.clear();
    lcd.print("Starting...");
    lcd.setCursor(0, 1);
    lcd.print("Serial          ");
    Serial.begin(115200);
    // Open serial communications with grbl and wait for port to open:
    grbl_init();

    lcd.setCursor(0, 1);
    lcd.print("Ethernet        ");
    // start the Ethernet connection and the server:
    Ethernet.begin(mac, ip, gateway, gateway, subnet);
    Ethernet.init(ETH_CS);  // ethernet shield activation pin.

    
    if (Ethernet.linkStatus() == LinkOFF) {
        lcd.println("No eth cable    ");
        delay(3000);
    }
    // start the server
    ethServer.begin();

    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("SD card reader  ");

    if (!SD.begin(SD_CS)) {
        lcd.clear();
        lcd.print("initialization failed");
        // lcd.print("1. is a card inserted?");
        // lcd.print("2. is your wiring correct?");
        // lcd.print("3. did you change the chipSelect pin to match your shield or module?");
        // lcd.print("Note: press reset or reopen this Serial Monitor after fixing your issue!");
        lcd.autoscroll();
        delay(30000);
    }
    delay(2000);

    lcd.setRGB(100, 100, 254);
    lcd.clear();
    lcd.write("Ready");
}

void updateLED(int input) {
    if (input) {
        // coil value set, turn LED color blue
        lcd.setRGB(10, 10, 254);
    } else {
        // coil value clear, turn LED color red
        lcd.setRGB(254, 0, 0);
    }
}



/**
 * @brief Parses a command sent to the rotator. 
 * The format is always <Letter><Value><NextLetter><Value>  
 * @param line char array with the command line.     
 * @return true if parsed successfully
 * @return false if parsing failed.
 */
bool commandParser(EthernetClient *client) {
    uint8_t char_counter = 0;
    char letter;
    float value;
    uint8_t int_value = 0;
    bool sysCommand = false;
    float parameterValue = 0;



    while (inData[char_counter] != 0) {  // Loop until null terminated inData.
   

        // first check for sys command and set flag
        if (inData[0] == '$') {
            sysCommand = true;
            char_counter++;
        }

        // read letter
        letter = inData[char_counter];

        // Lowercase to uppercase
        if (letter >= 'a' && letter <= 'z') {  
            letter = letter - 'a' + 'A';
        }

        // Is it a letter? otherwise bail.
        if ((letter < 'A') || (letter > 'Z')) {  
            client->write("expected a letter but received: ");
            client->write(letter);
            return false;
        }  // [Expected word letter]

        // Handle sys commands
        if (sysCommand) {
            switch (letter) {
                case 'A': // pass throug to grbl
                    grbl_write(&inData[++char_counter]);
                    break;
                case 'H': // Home
                    grbl_go_home();
                    break;
                case 'I': // Info
                    client->println(grbl_state);
                    client->println(grbl_ok);
                    // print info not implemented.
                    break;
                case 'M': // Message
                    if(inData[2] == '~') {messageDismissed = true;}
                    else if(inData[2] != '=') {return false;}
                    else {strcpy(&inData[3],message); messageDismissed=false;}
                    break;
                case 'R':
                   grbl_reset(); // ctrl+c means reset grbl.
                   break;
                default:
                    return false;
                    break;
            }
            return true; // all sys commands only run once. 
        } else {
            // Handle normal commands.
            // Try to read value after letter 
            char_counter++;
            if (!read_float(inData, &char_counter, &value)) {
                client->write("could not read number");
                return false;
            }  // [Expected word value]
            
            // Convert values to smaller uint8 significand for parsing this word.
            int_value = trunc(value);
            switch (letter) {
                case 'M':
                    switch (int_value) {
                        case 100:
                              // M100 start program.
                            break;
                        case 101:
                              // M101 stop any program running
                            break;
                        case 110:
                              // M110 run to idle pos where the rack can be taken out (feedrate)
                            break;
                    }
                case 'E':
                    destinationTime = abs(value);
                    break;
                case 'P':
                    parameterValue = value;
                    break;
                case 'F': 
                    rpm = value;
                    break;
            }
        }
    }// end while
    client->println("got it");
    return true;

}  // end commandParser



void P0(){
    float destinationRotations = (destinationTimeS/60)*rpm;
    char command[16] ;
    sprintf(command, "G0X%.2fF%i\n",destinationRotations,rpm); 
    grbl_write(command);
}

void ticker() {
    unsigned long nowS = millis()/1000;
    if (grbl_state == STATE_CYCLE){
        if (nowS > lastTick) {
            consumedTime = lastTick-nowS;
            lastTick = nowS;
        }
    }
}

void loop() {
    ticker();
    grbl_sync();
    //updateLCD();

    // read tcp communication.
    // Wait for an incomming connection
    EthernetClient client = ethServer.available();

    // Do we have a client? if not return back out of loop
    if (!client) return;

    // Only here if anything to read from eth port
    // Is there anything to read?
    if (client.available()) {
        int bytesReceived = client.readBytesUntil('\n', inData, BUFFER_LENGTH);
        inData[bytesReceived] = '\0';  // Null terminate the string.
    } else
        return;

    commandParser(&client);


}
