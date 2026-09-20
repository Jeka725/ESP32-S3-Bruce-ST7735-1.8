//   Developed by ViniciusHNF
//   GitHub Repository: https://github.com/viniciushnf/ESP32-S3-Bruce-ST7735-1.8

#include "../src/modules/others/battery_information.h"
#include "core/powerSave.h"
#include "core/utils.h"
#include <Arduino.h>

// ======================================================
//          SETTINGS
// ======================================================
const unsigned long readDelay = 20;         // Time between readings
const unsigned long firstRepeatDelay = 400; // initial button wait
const unsigned long repeatDelay = 200;      // Continuous repetition of the button
const unsigned long debounceDelay = 50;     // Debounce button

// ======================================================
//          PINS
// ======================================================
#ifndef SEL_BTN
#define SEL_BTN 14
#endif
#ifndef BTN_LEFT
#define BTN_LEFT 12
#endif
#ifndef BTN_RIGHT
#define BTN_RIGHT 13
#endif
#ifndef BTN_UP
#define BTN_UP 9
#endif
#ifndef BTN_DOWN
#define BTN_DOWN 11
#endif
#ifndef TFT_BL
#define TFT_BL 4
#endif

// ======================================================
//          FLAGS
// ======================================================
volatile bool upPress_flag = false;
volatile bool downPress_flag = false;
volatile bool leftPress_flag = false;
volatile bool rightPress_flag = false;
volatile bool slPress_flag = false;

// ======================================================
//          DIRECTION CONTROLLER
// ======================================================
enum JoyDirection { JOY_NONE, JOY_LEFT, JOY_RIGHT, JOY_UP, JOY_DOWN };
JoyDirection currentDirection = JOY_NONE;
unsigned long lastReadTime = 0;
unsigned long lastMoveTime = 0;
bool firstRepeat = true;

// ======================================================
// BUTTON CONTROL
// ======================================================
bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;
unsigned long lastDebounceTime = 0;

// ======================================================
// SETUP GPIO
// ======================================================
void _setup_gpio() {
    pinMode(SEL_BTN, INPUT_PULLUP);
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);

    pinMode(TFT_BL, OUTPUT);    // Backlight OUTPUT
    digitalWrite(TFT_BL, HIGH); // Backlight HIGHT

    bruceConfig.colorInverted = 0;
    bruceConfigPins.rotation = 1;
}

void _post_setup_gpio() {
    // nothing
}

// ======================================================
//          BATTERY
// ======================================================
int getBattery() { return Battery_information::getBatteryPercentage(); }

// ======================================================
//          SCREEN BRIGHTNESS
// ======================================================
void _setBrightness(uint8_t brightval) { analogWrite(TFT_BL, brightval); }

// ======================================================
//          Direction Buttons Reading
// ======================================================
JoyDirection readJoystickDirection() {
    if (digitalRead(BTN_LEFT) == LOW) { return JOY_LEFT; }
    if (digitalRead(BTN_RIGHT) == LOW) { return JOY_RIGHT; }
    if (digitalRead(BTN_UP) == LOW) { return JOY_UP; }
    if (digitalRead(BTN_DOWN) == LOW) { return JOY_DOWN; }
    return JOY_NONE;
}

// ======================================================
//          GENERATE EVENT
// ======================================================
void triggerDirectionEvent(JoyDirection dir) {
    switch (dir) {
        case JOY_UP: upPress_flag = true; break;
        case JOY_DOWN: downPress_flag = true; break;
        case JOY_LEFT: leftPress_flag = true; break;
        case JOY_RIGHT: rightPress_flag = true; break;
        default: break;
    }
}

void joystickMap() {
    if (menuOptionLabel == "Main Menu") {
        // EscPress = downPress_flag; // Down
        PrevPress = leftPress_flag;  // Left
        NextPress = rightPress_flag; // Right

        // if (upPress_flag) {
        //     OpenQuickAccess = true;
        // } else {
        //     OpenQuickAccess = false;
        // }
    } else {
        PrevPress = upPress_flag;    // Up
        NextPress = downPress_flag;  // Down
        EscPress = leftPress_flag;   // Left
        DownPress = rightPress_flag; // Right
        OpenQuickAccess = false;
    }

    // Serial.print("menuOptionType: '");
    // Serial.print(menuOptionType); // Returns the menu index. Ex: Main Menu is 0, WiFi is 1, WiFi Atks is 2.
    // Serial.print("' - menuOptionLabel: '");
    // Serial.print(menuOptionLabel); // Returns the menu type. Ex: Main Menu, WiFi, BLE.
    // Serial.println("'");
}

// ======================================================
//          INPUT HANDLER
// ======================================================
void InputHandler(void) {
    unsigned long now = millis();
    if (now - lastReadTime < readDelay) { return; } // Limits reading rate
    lastReadTime = now;

    // ==================================================
    // DIRECTION BUTTONS
    // ==================================================
    JoyDirection newDirection = readJoystickDirection();
    if (newDirection != currentDirection) {
        currentDirection = newDirection;
        lastMoveTime = now; // Reset timers
        firstRepeat = true;
        if (newDirection != JOY_NONE) { triggerDirectionEvent(newDirection); }
    }

    else if (newDirection != JOY_NONE) {
        unsigned long delayTime = firstRepeat ? firstRepeatDelay : repeatDelay;
        if (now - lastMoveTime >= delayTime) {
            triggerDirectionEvent(newDirection);
            lastMoveTime = now;
            firstRepeat = false;
        }
    }

    // ==================================================
    //          Select button with debounce.
    // ==================================================
    bool reading = digitalRead(SEL_BTN);
    if (reading != lastButtonReading) { lastDebounceTime = now; }
    if ((now - lastDebounceTime) > debounceDelay) {
        if (reading != stableButtonState) {
            stableButtonState = reading;
            if (stableButtonState == LOW) { slPress_flag = true; }
        }
    }
    lastButtonReading = reading;

    // ==================================================
    //          EVENT SUBMISSION
    // ==================================================
    if (upPress_flag || downPress_flag || leftPress_flag || rightPress_flag || slPress_flag) {

        AnyKeyPress = true;

        // PrevPress = upPress_flag;    // Up default
        // NextPress = downPress_flag;  // Down default
        // EscPress = leftPress_flag;   // Left default
        // DownPress = rightPress_flag; // Right default
        // SelPress = slPress_flag;     // Select default

        joystickMap();

        SelPress = slPress_flag; // Select

        // Clear flags
        upPress_flag = false;
        downPress_flag = false;
        leftPress_flag = false;
        rightPress_flag = false;
        slPress_flag = false;
    }
}

// ======================================================
// POWER
// ======================================================
void powerOff() {}
void checkReboot() {}
