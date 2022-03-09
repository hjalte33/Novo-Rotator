#include <Ethernet.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <stdint.h>
#include <stdlib.h>

#include "../include/grbl_talker.h"
#include "../include/helpers.h"
#include "../include/navigator.h"

#define ETH_CS 10
#define SD_CS 4
#define BUF_LEN 128

unsigned long destinationTimeS;

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
                    client->println("ok");
                    break;
                case 'R':
                   grbl_reset(); // ctrl+c means reset grbl.
                   break;
                default:
                    client->println("error");
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
    client->println("ok");
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


void setup() {
    Wire.begin();
    Serial.begin(115200);
    // Initialize LCD screen
    lcdInit();

    // Open serial communications with grbl and wait for port to open:
    grbl_init();

    lcd.setCursor(0, 1);
    lcd.print("Ethernet        ");
    // start the Ethernet connection and the server:
    Ethernet.begin(mac, ip, gateway, gateway, subnet);
    Ethernet.init(ETH_CS);  // ethernet shield activation pin.

    
    if (Ethernet.linkStatus() == LinkOFF) {
        lcd.println("No eth cable    ");
        delay(2000);
    }
    // start the server
    ethServer.begin();

    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("SD card reader  ");
    SD_init(SD_CS);
    draw_menu_info();
    btnsInit();
}

void loop() {
    ticker();
    grbl_sync();

    navigator_update();

    // read tcp communication.
    // Wait for an incomming connection
    EthernetClient client = ethServer.available();

    // Do we have a client? if not return back out of loop
    if (!client) return;

    // Only here if anything to read from eth port
    // Is there anything to read?
    if (client.available()) {
        int bytesReceived = client.readBytesUntil('\n', inData, BUF_LEN);
        inData[bytesReceived] = '\0';  // Null terminate the string.
    } else
        return;

    commandParser(&client);

}
