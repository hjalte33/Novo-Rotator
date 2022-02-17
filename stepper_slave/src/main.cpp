/*    Serial motor controller.
//    This code functions to turn your arduino into a simple serial controlled
//    stepper motor controller for NN sample rotator.
//
//    Created by HTNL@novonordisk.com
*/

#include <AccelStepper.h>
#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>
//#include <stdlib.h>

#define STEP_PIN 3
#define DIR_PIN 7
#define DISABLE_PIN 8
#define HOME_PIN 4
#define BUF_LEN 9
#define RATIO (28 / 12.0)
#define STEPPRREVOLUTION 200
#define MAXACCELERATION 2000
#define MAXRPM 60
#define TICKTIME 250
#define TICKS2S (1000. / TICKTIME)

long int destinationStep;
unsigned long int destinationTimeS = 600;
unsigned long int consumedTime = 0;

int program = 0;  // selected program ToDo
enum State {
    err = -1,
    done,
    run,
    stopped,
    inject,
    home,
};
State state = stopped;  // state of our driver.

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);  // step pin

int rpm2sps(float r) { return (r * STEPPRREVOLUTION * RATIO) / 60.0; }

void homeing() {
    // rotate fast til senseor detect
    stepper.setSpeed(500);
    while (digitalRead(HOME_PIN) == HIGH) {
        stepper.runSpeed();
    }
    stepper.stop();
    // back off sensor
    stepper.setSpeed(-50);
    while (digitalRead(HOME_PIN) == LOW) {
        stepper.runSpeed();
    }
    stepper.stop();
    // approach sensor again but slow
    stepper.setSpeed(10);
    while (digitalRead(HOME_PIN) == HIGH) {
        stepper.runSpeed();
    }
    stepper.stop();
    // reset position
    stepper.setCurrentPosition(0);
}

void goHome() {
    stepper.stop();
    long pos = stepper.currentPosition();
    long target = stepper.targetPosition();
    if (target > pos) {
        target += target % int(STEPPRREVOLUTION * RATIO);
    } else if (target < pos) {
        target -= target % int(STEPPRREVOLUTION * RATIO);
    }
    stepper.moveTo(target);
}

void goInject() {
    homeing();                                                 // make sure we are where we think we are
    stepper.runToNewPosition((STEPPRREVOLUTION / 2) * RATIO);  // blocking - turn 180 degrees
}

void respond(int val) {
    switch (val) {
        case 1:  // consumed Time
            Serial.print('c');
            Serial.println(consumedTime);
            break;
        case 2:
            Serial.print('t');
            Serial.println(destinationTimeS);
            break;
        case 3:
            Serial.print('p');
            Serial.println(program);
            break;
        default:
            char buff[10];
            buff[0] = 'e';
            itoa(state, &buff[1], 10);
            Serial.println(buff);
            break;
    }
}
void readSerial() {
    static char sdata[BUF_LEN];
    int bytesReceived;
    int val = 0;
    bool ok = true;

    bytesReceived = Serial.readBytesUntil('\0', sdata, BUF_LEN);
    sdata[bytesReceived] = '\0';  // Null terminate the string.
    val = atoi(&sdata[1]);

    switch (sdata[0]) {
        case 'r':  // rpm
            if (val <= MAXRPM && val > 0) {
                stepper.setMaxSpeed(rpm2sps(val));
            } else {
                ok = false;
            }
            break;
        case 'a':  // acceleration
            if (val <= MAXACCELERATION && val > 0) {
                stepper.setAcceleration(val);
            } else {
                ok = false;
            }
            break;
        case 't':  // time in seconds - cannot be negative
            if (val > 0) {
                destinationTimeS = max(0, val);
            } else {
                ok = false;
            }
            break;
        case 's':  // STOP
            goHome();
            state = stopped;
            break;
        case 'g':  // Go
            digitalWrite(DISABLE_PIN,LOW);
            consumedTime = 0;
            if (val < 0) {
                stepper.moveTo(-100000000);
            } else {
                stepper.moveTo(100000000);
            }
            state = run;
            break;
        case 'p':  // Program select ToDo
            program = val;
            break;
        case 'h':  // Home
            digitalWrite(DISABLE_PIN, LOW);
            homeing();
            state = home;
            break;
        case 'i':  // injection position
            digitalWrite(DISABLE_PIN, LOW);
            goInject();
            state = inject;
            break;
        case 'q':  // Question the state
            respond(val);
            return;  // return early and skip the ack msg.
        case 'o' : // off
            digitalWrite(DISABLE_PIN, HIGH);
        default:
            ok = false;
            break;
    }

    // send ack/nak back to master
    Serial.println(ok ? 6 : 22);
}

unsigned long lastTick = 0;
int tickCounter = 0;

void ticker() {
    unsigned long now = millis();

    if (now > lastTick + TICKTIME) {
        if (tickCounter >= TICKS2S && state == run) {  // if a second has passed
            consumedTime++;
            tickCounter = 0;
            if (consumedTime >= destinationTimeS) {
                goHome();
                state = done;
            }
        }

        tickCounter++;
        lastTick = now;
    }
}

void setup() {
    pinMode(DISABLE_PIN,OUTPUT);
    digitalWrite(DISABLE_PIN,HIGH);

    // Open serial communications and wait for port to open:
    Serial.begin(115200);
    while (!Serial) {
        ;  // wait for serial port to connect. Needed for native USB port only
    }
    Serial.setTimeout(10);  // maximum time to wait for data is 1ms

    //attachInterrupt(digitalPinToInterrupt(2), readSerial, RISING);

    // set some sane defaults
    stepper.setMaxSpeed(rpm2sps(25));
    stepper.setAcceleration(400);
}

void loop() {
    ticker();
    stepper.run();
    if (Serial.available()) {
        readSerial();
    }
}