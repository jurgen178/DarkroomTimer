//
// Arduino Darkroom timer
//
// Implements 5 tasks:

// LEDBrightnessTask : Sets the brigness of the 8x8 darkroom ilumination LEDs.
// EnlargerOnOffTask : Manual enlarger switch.
// EnlargerTimerTask : Timed enlarger switch.
// DevelopStopFixTask : 3 timers for develop, stop bath and fix.
// GeneralTimerTask : General timer starting from 0s.
//
// Uses the Gamepad Input shield. 
// Up/down button/joystick changes task, and left/right button/joystick changes state/setting.
//
// Uses Arduino Uno R4.
// https://community.element14.com/products/arduino/b/blog/posts/arduino-uno-r4-minima-troubleshooting-unknown-usb-device
//

#include <functional>
#include <LedControl.h>
#include <EEPROM.h>

// Setup pins and defaults.
const int AudioPin = 13;
const int RelaisPin = 9;
const int defaultBrightness = 2;
const int constDisplayStartColum = 18;

// Define the pins for the joystick and buttons.
#define PIN_ANALOG_X A0
#define PIN_ANALOG_Y A1
#define BUTTON_UP 2
#define BUTTON_RIGHT 3
#define BUTTON_DOWN 4
#define BUTTON_LEFT 5
#define BUTTON_E 6
#define BUTTON_F 7
#define BUTTON_K 8


// Setup the 8x8 LEDs.
const int numDevices = 4;  // Number of MAX7219 devices
// DIN     (Orange)  D12
// CLK     (Green)    D11
// CS      (Yellow)    D10
// Rot     (Brown)   +5V
// Schwarz (Red)     Gnd
LedControl lc = LedControl(12, 11, 10, numDevices);

// Set the order of the darkroom tasks.
enum DarkroomTasks {
    LEDBrightnessTask,  // Sets the brigness of the 8x8 darkroom ilumination LEDs.
    EnlargerOnOffTask,  // Manual enlarger switch.
    EnlargerTimerTask,  // Timed enlarger switch.
    DevelopStopFixTask, // 3 timers for develop, stop bath and fix.
    GeneralTimerTask,   // General timer starting from 0s.

    End,
};

enum DevelopStopFixStage {
    BeginStage,  // Adding a sentinel value for the begin.

    DevelopStage,
    StopBathStage,
    FixStage,

    EndStage,  // Adding a sentinel value for the end.
};

DevelopStopFixStage operator++(DevelopStopFixStage& stage, int) {
    if (stage < DevelopStopFixStage::EndStage - 1)
        stage = (DevelopStopFixStage)(stage + 1);
    else
        stage = (DevelopStopFixStage)(DevelopStopFixStage::BeginStage + 1);

    return stage;
}

DevelopStopFixStage operator--(DevelopStopFixStage& stage, int) {
    if (stage > DevelopStopFixStage::BeginStage + 1)
        stage = (DevelopStopFixStage)(stage - 1);
    else
        stage = (DevelopStopFixStage)(DevelopStopFixStage::EndStage - 1);


    return stage;
}


unsigned char font[] = {
    // -------- Space
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    // -------- A
    0b01111110,
    0b10010000,
    0b10010000,
    0b01111110,
    // -------- B
    0b01101100,
    0b10010010,
    0b10010010,
    0b11111110,
    // -------- C
    0b10000010,
    0b10000010,
    0b01111100,
    // -------- D
    0b00111000,
    0b01000100,
    0b10000010,
    0b11111110,
    // -------- E
    0b10000010,
    0b10010010,
    0b11111110,
    // -------- F
    0b10000000,
    0b10010000,
    0b11111110,
    // -------- G
    0b01011100,
    0b10010010,
    0b10000010,
    0b01111100,
    // -------- H
    0b11111110,
    0b00010000,
    0b00010000,
    0b11111110,
    // -------- I
    0b10000010,
    0b11111110,
    0b10000010,
    // -------- J
    0b11111100,
    0b00000010,
    0b00001100,
    // -------- K
    0b10000110,
    0b01001000,
    0b00110000,
    0b11111110,
    // -------- L
    0b00000010,
    0b00000010,
    0b11111110,
    // -------- M
    0b11111110,
    0b01100000,
    0b00111100,
    0b01100000,
    0b11111110,
    // -------- N
    0b11111110,
    0b00011000,
    0b01100000,
    0b11111110,
    // -------- O
    0b01111100,
    0b10000010,
    0b10000010,
    0b01111100,
    // -------- P
    0b01100000,
    0b10010000,
    0b10010000,
    0b11111110,
    // -------- Q
    0b01111010,
    0b10000100,
    0b10001010,
    0b01111100,
    // -------- R
    0b01100110,
    0b10011000,
    0b10010000,
    0b11111110,
    // -------- S
    0b10001100,
    0b10010010,
    0b01100010,
    // -------- T
    0b10000000,
    0b11111110,
    0b10000000,
    // -------- U
    0b11111100,
    0b00000010,
    0b00000010,
    0b11111100,
    // -------- V
    0b11000000,
    0b00111000,
    0b00000110,
    0b00111000,
    0b11000000,
    // -------- W
    0b11111110,
    0b00001100,
    0b00111000,
    0b00001100,
    0b11111110,
    // -------- X
    0b11000110,
    0b00111000,
    0b00111000,
    0b11000110,
    // -------- Y
    0b11100000,
    0b00011110,
    0b11100000,
    // -------- Z
    0b11000010,
    0b10110010,
    0b10001110,
    // -------- Unknown character
    0b00111000,
    0b00111000,
    0b00111000,
    // -------- 0
    0b01111100,
    0b10100010,
    0b10010010,
    0b01111100,
    // -------- 1
    0b11111110,
    0b01000000,
    // -------- 2
    0b01100010,
    0b10010010,
    0b10001110,
    // -------- 3
    0b01101100,
    0b10010010,
    0b10000010,
    // -------- 4
    0b11111110,
    0b00010000,
    0b11110000,
    // -------- 5
    0b10001100,
    0b10010010,
    0b11110010,
    // -------- 6
    0b01001100,
    0b10010010,
    0b10010010,
    0b01111100,
    // -------- 7
    0b11100000,
    0b10011110,
    0b10000000,
    // -------- 8
    0b01101100,
    0b10010010,
    0b10010010,
    0b01101100,
    // -------- 9
    0b01111100,
    0b10010010,
    0b10010010,
    0b01100100,
    // -------- :
    0b00100100,
    // -------- ;
    0b00100110,
    0b00000001,
    // -------- !
    0b01100000,
    0b11111010,
    0b01100000,
    // -------- Heart
    0b01111000,
    0b11111100,
    0b11111110,
    0b01111111,
    0b11111110,
    0b11111100,
    0b01111000,
    // -------- <
    0b01000100,
    0b00101000,
    0b00010000,
    // -------- =
    0b00101000,
    0b00101000,
    0b00101000,
    0b00101000,
    // -------- >
    0b00010000,
    0b00101000,
    0b01000100,
    // -------- ?
    0b01100000,
    0b10011010,
    0b10000000,
    // -------- @
    0b01111100,
    0b10000010,
    0b10111010,
    0b10100010,
    0b01011100,
    // -------- (
    0b10000010,
    0b01111100,
    // -------- )
    0b01111100,
    0b10000010,
    // -------- *
    0b00101000,
    0b00010000,
    0b00101000,
    // -------- +
    0b00010000,
    0b00010000,
    0b01111100,
    0b00010000,
    0b00010000,
    // -------- ,
    0b00000110,
    0b00000001,
    // -------- -
    0b00010000,
    0b00010000,
    0b00010000,
    0b00010000,
    // -------- .
    0b00000010,
    // -------- /
    0b11000000,
    0b00111000,
    0b00000110,
    // -------- a
    0b00111110,
    0b00100010,
    0b00100010,
    0b00011100,
    // -------- b
    0b00011100,
    0b00100010,
    0b00100010,
    0b11111110,
    // -------- c
    0b00100010,
    0b00100010,
    0b00011100,
    // -------- d
    0b11111110,
    0b00100010,
    0b00100010,
    0b00011100,
    // -------- e
    0b00011000,
    0b00101010,
    0b00101010,
    0b00011100,
    // -------- f
    0b10010000,
    0b01111110,
    0b00010000,
    // -------- g
    0b00111110,
    0b00100101,
    0b00100101,
    0b00011000,
    // -------- h
    0b00011110,
    0b00100000,
    0b00100000,
    0b11111110,
    // -------- i
    0b00000010,
    0b01011110,
    0b00010010,
    // -------- j
    0b01011110,
    0b00000001,
    0b00000001,
    // -------- k
    0b00100010,
    0b00010100,
    0b00001000,
    0b11111110,
    // -------- l
    0b00000010,
    0b11111100,
    // -------- m
    0b00011110,
    0b00100000,
    0b00111110,
    0b00100000,
    0b00111110,
    // -------- n
    0b00011110,
    0b00100000,
    0b00100000,
    0b00111110,
    // -------- o
    0b00011100,
    0b00100010,
    0b00100010,
    0b00011100,
    // -------- p
    0b00011100,
    0b00100010,
    0b00100010,
    0b00111111,
    // -------- q
    0b00111111,
    0b00100010,
    0b00100010,
    0b00011100,
    // -------- r
    0b00010000,
    0b00100000,
    0b00111110,
    // -------- s
    0b00100100,
    0b00101010,
    0b00101010,
    0b00010010,
    // -------- t
    0b00100010,
    0b11111100,
    0b00100000,
    // -------- u
    0b00111110,
    0b00000010,
    0b00000010,
    0b00111100,
    // -------- v
    0b00111000,
    0b00000110,
    0b00111000,
    // -------- w
    0b00111110,
    0b00000010,
    0b00011110,
    0b00000010,
    0b00111100,
    // -------- x
    0b00110110,
    0b00001000,
    0b00110110,
    // -------- y
    0b00111110,
    0b00000101,
    0b00000101,
    0b00111001,
    // -------- z
    0b00110010,
    0b00101010,
    0b00100110,
    0b00100010,
    // -------- {, Switch opened
    0b00000010,
    0b00000010,
    0b11100010,
    0b10000010,
    0b11111110,
    0b00000010,
    0b00000000,
    0b00001000,
    0b00001000,
    0b00001000,
    0b00011000,
    0b01000000,
    0b00100000,
    0b00010000,
    0b00001000,
    0b00001000,
    0b00001000,
    0b00001000,
    // -------- }, Switch closed
    0b00000010,
    0b00110010,
    0b11110010,
    0b10000010,
    0b11111110,
    0b00000010,
    0b00000000,
    0b00001000,
    0b00001000,
    0b00001000,
    0b00011000,
    0b00001000,
    0b00001000,
    0b00001000,
    0b00001000,
    0b00001000,
    0b00001000,
    0b00001000,
    // -------- ~, brightness symbol
    0b10010010,
    0b01010100,
    0b00111000,
    0b11111110,
    0b00111000,
    0b01010100,
    0b10010010,
    // -------- |, tray symbols
    0b00000010,
    0b00000001,
    0b00000001,
    0b00000001,
    0b00000001,
    0b00000001,
    0b00000010,
    0b00000000,
    0b00000010,
    0b00000001,
    0b00000001,
    0b00000001,
    0b00000001,
    0b00000001,
    0b00000010,
    0b00000000,
    0b00000010,
    0b00000001,
    0b00000001,
    0b00000001,
    0b00000001,
    0b00000001,
    0b00000010,
    // -------- [, enlarger
    0b00000010,
    0b00000010,
    0b11100010,
    0b10000010,
    0b11111110,
    0b00000010,
    // -------- 0x20, small 0
    0b01110000,
    0b10001000,
    0b01110000,
    // -------- 0x21, small 1
    0b00001000,
    0b11111000,
    0b01001000,
    // -------- 0x22, small 2
    0b01001000,
    0b10101000,
    0b10011000,
    // -------- 0x23, small 3
    0b11111000,
    0b10101000,
    0b10001000,
    // -------- 0x24, small 4
    0b11111000,
    0b00100000,
    0b11100000,
    // -------- 0x25, small 5
    0b10110000,
    0b10101000,
    0b11101000,
    // -------- 0x26, small 6
    0b10111000,
    0b10101000,
    0b01111000,
    // -------- 0x27, small 7
    0b11000000,
    0b10100000,
    0b10011000,
    // -------- 0x28, small 8
    0b11111000,
    0b10101000,
    0b11111000,
    // -------- 0x29, small 9
    0b11110000,
    0b10101000,
    0b11101000,
};

// Char width table
unsigned char charwidth[] = { 4, 4, 4, 3, 4, 3, 3, 4, 4, 3, 3, 4, 3, 5, 4, 4, 4, 4, 4, 3, 3, 4, 5, 5, 4, 3, 3,
                              3, 4, 2, 3, 3, 3, 3, 4, 3, 4, 4, 1, 2, 3, 7, 3, 4, 3, 3, 5, 2, 2, 3, 5, 2, 4, 1, 3,
                              4, 4, 3, 4, 4, 3, 4, 4, 3, 3, 4, 2, 5, 4, 4, 4, 4, 3, 4, 3, 4, 3, 5, 3, 4, 4,
                              18, 18, 7, 23, 6,
                              3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                              0 };

// ASCII Codes of the implemented characters
unsigned char charcodes[] = { 32,                                                                                                      // ' '
                              65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,  // A-Z
                              255,
                              48, 49, 50, 51, 52, 53, 54, 55, 56, 57,                                                           // 0-9
                              58, 59, 33,                                                                                       // :;!
                              3,                                                                                                // Heart
                              60, 61, 62, 63, 64,                                                                               // <=>?@
                              40, 41, 42, 43, 44, 45, 46, 47,                                                                   // ()*+,-./
                              97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,  // a-z
                              117, 118, 119, 120, 121, 122,
                              123, 125, 126, 124, 91,                  // {}~| for symbols
                              20, 21, 22, 23, 24, 25, 26, 27, 28, 29,  // small numbers
                              0 };


int GetCharIndex(const unsigned char c) {
    int charindex = -1;

    for (int i = 0; i < sizeof(charcodes); i++) {
        if (charcodes[i] == c) {
            charindex = i;
            break;
        }
    }

    return charindex;
}

int GetCharWidth(const unsigned char c) {
    const int charindex = GetCharIndex(c);

    if (charindex != -1)
        return charwidth[charindex] + 1;
    else
        return 0;
}

int GetTextWidth(const char* text) {
    int len = strlen(text);
    int z = 0;
    for (unsigned int i = 0; i < len; i++) {
        z += GetCharWidth(text[i]);
    }

    return z;
}

uint8_t mirrorByte(uint8_t byte) {
    byte = (byte & 0xF0) >> 4 | (byte & 0x0F) << 4;
    byte = (byte & 0xCC) >> 2 | (byte & 0x33) << 2;
    byte = (byte & 0xAA) >> 1 | (byte & 0x55) << 1;
    return byte;
}

void toSmallNumberString(char* nstr, int z) {
    int n = 0;
    int d = z;
    do {
        d = d / 10;
        n++;
    } while (d);

    nstr[n] = 0;
    do {
        nstr[--n] = (z % 10) + 20;  // char codes for small font
        z = z / 10;
    } while (z);
}

int SetChar(const unsigned char c, int z, const bool useSmallFont = false) {
    const int charindex = GetCharIndex(c);

    int charoffset = 0;
    for (int i = 0; i < charindex; i++) {
        charoffset += charwidth[i];
    }

    unsigned char* pFont = font + charoffset;
    const unsigned char w = charwidth[charindex];

    // Serial.print("char = ");
    // Serial.print((int)c);
    // Serial.print(",");
    // Serial.println(c);
    // Serial.print("charindex = ");
    // Serial.println(charindex);
    // Serial.print("charoffset = ");
    // Serial.println(charoffset);
    // Serial.print("charwidth = ");
    // Serial.println(w);

    for (int i = 0; i <= w; i++) {
        const int block = (i + z) / 8;  // number of 8x8 element
        const int j = (i + z) % 8;
        const unsigned char c = i < w ? pFont[w - i - 1] : 0;

        const uint8_t mirrorC(mirrorByte(c));

        if (useSmallFont) {
            for (int r = 0; r < 5; r++) {
                lc.setLed(block, 7 - r, 7 - j, bitRead(mirrorC, r));
            }
        }
        else {
            //lc.setRow(block, 7-j, c); // 90° rotated
            lc.setColumn(block, 7 - j, mirrorC);
        }
    }

    return w + 1 + z;
}

int TextDisplay(const char* text, int z = 0, const bool useSmallFont = false) {
    int len = strlen(text);

    // Serial.println(text);
    // Serial.println(len);

    for (int i = 0; i < len; i++) {
        z = SetChar(text[i], z, useSmallFont);

        // Text outside of the visible area?
        if (z >= 31)
            return z;
    }

    // Clear until the end of the display for non small fonts.
    if (!useSmallFont)
    {
        // Text is smaller than the range and will be cleared with ' '.
        int k = z;
        while (k < 32)
            k = SetChar(' ', k);
    }

    return z;
}

void FillSolidArea(const int startCol, const int endCol, const byte value = 255) {
    for (int colIndex = startCol; colIndex <= endCol; colIndex++) {
        const int block = colIndex / 8;  // number of 8x8 element
        const int col = colIndex % 8;

        lc.setColumn(block, 7 - col, value);
    }
}


class CTask {
public:
    CTask()
        : initialized(false),
        symbolStartColum(9),
        displayStartColum(constDisplayStartColum),
        EEPROMaddrBrightness(4),
        EEPROMaddrTimer(EEPROMaddrBrightness + 4) {
        const int magicNumberAddress = 0;  // Address to store the magic number.
        const int magicNumber = 12345;     // A unique number to identify.

        int storedNumber;
        EEPROM.get(magicNumberAddress, storedNumber);

        if (storedNumber != magicNumber) {
            // EEPROM is not initialized, so initialize it.
            Serial.println("Initializing EEPROM");

            EEPROM.put(EEPROMaddrBrightness, 5);
            EEPROM.put(EEPROMaddrTimer, 10);

            EEPROM.put(magicNumberAddress, magicNumber);  // Write the magic number.
        }

        const int brightness(getBrightness());
        for (int d = 0; d < numDevices; d++) {
            lc.setIntensity(d, brightness);  // set Brightness 0..15
        }
    };

    virtual const char* taskName() const {
        return "CTask";
    };

    virtual void initializeTask() {
        // Serial.println("CTask initializeTask()");
        initialized = true;
    };

    virtual void closeTask() {
        // Serial.println("CTask closeTask()");
        initialized = false;
    };

    void ProcessSwitch(bool switchState) const {
        Serial.print("switch = ");
        Serial.println(switchState);

        digitalWrite(RelaisPin, switchState ? HIGH : LOW);
    };

    void Beep(const bool longBeep = false) const {
        Serial.println("Beep");

        // 440Hz, 500/50ms
        tone(AudioPin, 440, longBeep ? 500 : 50);
    };

    int getBrightness() const {
        int brightness = defaultBrightness;
        EEPROM.get(EEPROMaddrBrightness, brightness);  // Read the int value from EEPROM.

        if (brightness < 0 && brightness > 15) {
            brightness = defaultBrightness;
        }

        return brightness;
    }

    void setBrightness(int brightness) const {
        if (brightness < 0 && brightness > 15) {
            brightness = defaultBrightness;
        }

        EEPROM.put(EEPROMaddrBrightness, brightness);

        Serial.print("Set Brightness to ");
        Serial.println(brightness);
        for (int d = 0; d < numDevices; d++) {
            lc.setIntensity(d, brightness);  // set Brightness 0..15
        }
    }

    int getTimerValue() const {
        int value = 10;
        EEPROM.get(EEPROMaddrTimer, value);  // Read the int value from EEPROM.

        return value;
    }

    void setTimerValue(int& value) const {
        if (value < 1 || value > 1000)
            value = 10;

        EEPROM.put(EEPROMaddrTimer, value);
    }

    void ClearDisplay() const {
        FillSolidArea(symbolStartColum, 31, 0);
    }

    void ClearDisplayText(const int startColum = constDisplayStartColum) const {
        FillSolidArea(startColum, 31, 0);
    }

    void display(long value, const char* unit = "", const int startColum = constDisplayStartColum) const {
        Serial.print("display value = ");
        Serial.println(value);

        // int to string conversion
        char text[21] = { 0 };
        ltoa(value, text, 10);
        strcat(text, unit);
        TextDisplay(text, startColum);
    };

    // Run each time from the main loop() function.
    virtual void step() {};

    // Each task must implement the action button.
    virtual void actionButton() = 0;

    // Left/Right buttons are optional.
    virtual void leftButton() {};
    virtual void rightButton() {};

public:
    bool initialized;
    const int symbolStartColum;
    const int displayStartColum;
    const int EEPROMaddrBrightness;
    const int EEPROMaddrTimer;
};

// Sets the brigness of the 8x8 darkroom ilumination LEDs.
class CLEDBrightnessTask : public CTask {
public:
    virtual const char* taskName() const {
        return "LEDBrightnessTask";
    };

    virtual void initializeTask() {
        CTask::initializeTask();

        Serial.println("LEDBrightnessTask initializeTask()");
        TextDisplay("~", symbolStartColum);  // use extend chars to implement symbols, ~ is the brightness symbol

        const int brightness(getBrightness());
        display(brightness);
    };

    virtual void actionButton() {
        Serial.println("LEDBrightnessTask actionButton()");
    }

    virtual void leftButton() {
        int brightness(getBrightness());
        if (brightness > 0) {
            brightness--;
            setBrightness(brightness);
            display(brightness);
        }
        else {
            brightness = 0;
            Beep();
        }

        Serial.println("LEDBrightnessTask leftButton()");
    }

    virtual void rightButton() {
        int brightness(getBrightness());
        if (brightness < 15) {
            brightness++;
            setBrightness(brightness);
            display(brightness);
        }
        else {
            brightness = 15;
            Beep();
        }

        Serial.println("LEDBrightnessTask rightButton()");
    }
};

// Manual enlarger switch.
class CEnlargerOnOffTask : public CTask {
public:
    CEnlargerOnOffTask()
        : onOffSwitch(false) {};

public:
    virtual const char* taskName() const {
        return "EnlargerOnOffTask";
    };

    virtual void initializeTask() {
        CTask::initializeTask();

        Serial.println("EnlargerOnOffTask initializeTask()");
        TextDisplay(onOffSwitch ? "}" : "{", symbolStartColum);  // use extend chars to implement symbols, } is closed switch, and { is opened switch

        onOffSwitch = false;
    };

    virtual void closeTask() {
        CTask::closeTask();

        // Serial.println("EnlargerOnOffTask closeTask()");
        onOffSwitch = false;
        ProcessSwitch(onOffSwitch);
    };

    virtual void actionButton() {
        Serial.println("EnlargerOnOffTask actionButton()");
        onOffSwitch = !onOffSwitch;

        ProcessSwitch(onOffSwitch);
        TextDisplay(onOffSwitch ? "}" : "{", symbolStartColum);  // use extend chars to implement symbols, } is closed switch, and { is opened switch
    }

    virtual void leftButton() {
        Serial.println("EnlargerOnOffTask leftButton()");
        actionButton();
    }

    virtual void rightButton() {
        Serial.println("EnlargerOnOffTask rightButton()");
        actionButton();
    }

private:
    bool onOffSwitch;
};

// Timed enlarger switch.
class CEnlargerTimerTask : public CTask {
public:
    CEnlargerTimerTask()
        : timer(0),
        timerStartFlag(true),
        startTime(0L),
        timer_max(30),
        secondStart(0),
        elapsedSeconds(0),
        funcPtr(NULL) {};

public:
    virtual const char* taskName() const {
        return "EnlargerTimerTask";
    };

    virtual void initializeTask() {
        CTask::initializeTask();

        Serial.println("EnlargerTimerTask initializeTask()");
        TextDisplay("[", symbolStartColum);  // use extend chars to implement symbols, [ is the enlarger symbol

        timer = getTimerValue();
        Serial.print("timer value = ");
        Serial.println(timer);

        timerStartFlag = true;

        display(timer, "s");
    };

    virtual void closeTask() {
        CTask::closeTask();

        Serial.println("EnlargerTimerTask closeTask()");
        ProcessSwitch(false);
        funcPtr = NULL;
    };

    virtual void actionButton() {
        Serial.println("EnlargerTimerTask actionButton()");
        Serial.println("timer init");

        if (timerStartFlag) {
            setTimerValue(timer);
            display(timer, "s");
        }
        else {
            const long remainingSeconds(timer - elapsedSeconds);
            if (remainingSeconds >= 0 && remainingSeconds < timer_max) {
                int rs(remainingSeconds);
                setTimerValue(rs);
                timer = rs;
                display(rs, "s");
            }
        }

        ProcessSwitch(true);

        startTime = millis();
        secondStart = startTime;
        elapsedSeconds = 0;
        timerStartFlag = true;

        funcPtr = &CEnlargerTimerTask::timerFunc;
    }

    virtual void step() {
        if (funcPtr)
            (this->*funcPtr)();
    };

    virtual void leftButton() {
        if (timer > 1)
            timer--;

        Serial.println("EnlargerTimerTask leftButton()");

        if (timerStartFlag) {
            display(timer, "s");
        }
        else {
            const long remainingSeconds(timer - elapsedSeconds);
            if (remainingSeconds >= 0 && remainingSeconds < timer_max) {
                display(remainingSeconds, "s");
            }
        }
    }

    virtual void rightButton() {
        if (timer < timer_max)
            timer++;

        Serial.println("EnlargerTimerTask rightButton()");

        if (timerStartFlag) {
            display(timer, "s");
        }
        else {
            const long remainingSeconds(timer - elapsedSeconds);
            if (remainingSeconds >= 0 && remainingSeconds < timer_max) {
                display(remainingSeconds, "s");
            }
        }
    }

private:
    virtual void timerFunc() {
        const unsigned long currentTime(millis());

        // Process start.
        if (timerStartFlag) {
            Serial.print(" timer start = ");
            Serial.print(timer);
            Serial.println("s");

            display(timer, "s");

            timerStartFlag = false;
        }

        // Process each second.
        if (currentTime - secondStart >= 1000 && timer >= elapsedSeconds) {
            elapsedSeconds++;
            secondStart = millis();

            const long remainingSeconds(timer - elapsedSeconds);
            Serial.print(" timer = ");
            Serial.print(remainingSeconds);
            Serial.println("s");

            display(remainingSeconds, "s");

            // Short Beep.
            Beep();
        }

        if (currentTime - startTime >= timer * 1000 || timer <= elapsedSeconds) {
            Serial.print("timer end, duration: ");
            Serial.print(currentTime - startTime);
            Serial.println("ms");

            ProcessSwitch(false);

            // Long beep.
            Beep(true);

            elapsedSeconds = 0;
            display(0, "s");

            timerStartFlag = true;
            funcPtr = NULL;
        }
    };

private:
    int timer;
    bool timerStartFlag;
    unsigned long startTime;
    const unsigned int timer_max;
    unsigned long secondStart;
    int elapsedSeconds;
    void (CEnlargerTimerTask::* funcPtr)();
};

// 3 timers for develop, stop bath and fix.
class CDevelopStopFixTask : public CTask {
public:
    CDevelopStopFixTask()
        : timerDevelop(60),
        timerStopBath(10),
        timerFix(20),
        timerStartFlag(true),
        secondStart(0),
        elapsedSeconds(0),
        funcPtr(NULL) {
        ResetStage();
    };

public:
    virtual const char* taskName() const {
        return "DevelopStopFixTask";
    };

    virtual void initializeTask() {
        CTask::initializeTask();

        Serial.println("DevelopStopFixTask initializeTask()");
        TextDisplay("|", symbolStartColum);  // use extend chars to implement symbols, | are the tray symbols

        // Develop=1m, Stopbath=10s, Fix=30s
        // one tray in the symbol is 8 pixel wide
        char nstr[10] = { 0 };
        toSmallNumberString(nstr, timerDevelop);
        TextDisplay(nstr, symbolStartColum + 4 - GetTextWidth(nstr) / 2, true);  // use small font

        toSmallNumberString(nstr, timerStopBath);
        TextDisplay(nstr, symbolStartColum + 12 - GetTextWidth(nstr) / 2, true);  // use small font

        toSmallNumberString(nstr, timerFix);
        TextDisplay(nstr, symbolStartColum + 20 - GetTextWidth(nstr) / 2, true);  // use small font

        ResetStage();
    };

    virtual void closeTask() {
        CTask::closeTask();

        Serial.println("DevelopStopFixTask closeTask()");
        funcPtr = NULL;
    };

    virtual void actionButton() {
        Serial.println("DevelopStopFixTask actionButton()");
        Serial.println("timer init");

        processStage = DevelopStopFixStage::DevelopStage;
        ProcessStage(processStage);
    }

    virtual void step() {
        if (funcPtr)
            (this->*funcPtr)();
    };

    virtual void leftButton() {
        // Skip back to the previous stage.
        if (processStage > DevelopStopFixStage::BeginStage + 1) {
            processStage--;
        }

        ProcessStage(processStage);

        Serial.println("DevelopStopFixTask leftButton()");
    }

    virtual void rightButton() {
        // Skip to the next stage.
        if (processStage < DevelopStopFixStage::EndStage - 1) {
            processStage++;
        }

        ProcessStage(processStage);

        Serial.println("DevelopStopFixTask rightButton()");
    }

private:
    void ResetStage() {
        processStage = DevelopStopFixStage::BeginStage;
        funcPtr = NULL;
        timerStartFlag = true;
    }

    void ProcessStage(DevelopStopFixStage stage) {

        // timer init
        startTime = millis();
        secondStart = startTime;
        elapsedSeconds = 0;
        timerStartFlag = true;

        switch (stage) {
        case DevelopStopFixStage::DevelopStage:
        {
            Serial.println("DevelopStage");

            TextDisplay("1:", symbolStartColum);
            funcPtr = &CDevelopStopFixTask::timerDevelopFunc;

            break;
        }

        case DevelopStopFixStage::StopBathStage:
        {
            Serial.println("StopBathStage");

            TextDisplay("2:", symbolStartColum);
            funcPtr = &CDevelopStopFixTask::timerStopBathFunc;

            Beep();
            break;
        }

        case DevelopStopFixStage::FixStage:
        {
            Serial.println("FixStage");

            TextDisplay("3:", symbolStartColum);
            funcPtr = &CDevelopStopFixTask::timerFixFunc;

            Beep();
            break;
        }
        }
    }

    virtual void timerDevelopFunc() {
        timerFunc("timerDevelop", timerDevelop);
    }

    virtual void timerStopBathFunc() {
        timerFunc("timerStopBath", timerStopBath);
    }

    virtual void timerFixFunc() {
        timerFunc("timerFix", timerFix);
    }

    virtual void timerFunc(const char* timeName, const unsigned int timerLength) {
        const unsigned long currentTime(millis());

        // Process start.
        if (timerStartFlag) {
            Serial.print(" timer start = ");
            Serial.print(timerLength);
            Serial.println("s");

            display(timerLength, "s");

            timerStartFlag = false;
        }

        // Process each second.
        if (currentTime - secondStart >= 1000 && timerLength > elapsedSeconds) {
            elapsedSeconds++;
            secondStart = millis();

            const long remainingSeconds(timerLength - elapsedSeconds);
            Serial.print(" ");
            Serial.print(timeName);
            Serial.print(" = ");
            Serial.print(remainingSeconds);
            Serial.println("s");

            display(remainingSeconds, "s");

            // Short Beep.
            Beep();
        }

        if (currentTime - startTime >= timerLength * 1000 || timerLength <= elapsedSeconds) {
            Serial.print(timeName);
            Serial.print(" end, duration: ");
            Serial.print(currentTime - startTime);
            Serial.println("ms");

            // Long beep.
            Beep(true);

            funcPtr = NULL;
        }
    };

private:
    DevelopStopFixStage processStage;
    const unsigned int timerDevelop;
    unsigned long startTime;
    const unsigned int timerStopBath;
    const unsigned int timerFix;
    unsigned long secondStart;
    bool timerStartFlag;
    int elapsedSeconds;
    void (CDevelopStopFixTask::* funcPtr)();
};

// General timer starting from 0s.
class CGeneralTimerTask : public CTask {
public:
    CGeneralTimerTask()
        : timer(0),
        timerStartFlag(true),
        startTime(0L),
        secondStart(0),
        startColumn(GetCharWidth('t') + 10),
        funcPtr(NULL) {};

public:
    virtual const char* taskName() const {
        return "CGeneralTimerTask";
    };

    virtual void initializeTask() {
        CTask::initializeTask();

        Serial.println("CGeneralTimerTask initializeTask()");
        TextDisplay("t", symbolStartColum);

        timer = 0;
        timerStartFlag = true;

        display(timer, "s", startColumn);
    };

    virtual void closeTask() {
        CTask::closeTask();

        Serial.println("CGeneralTimerTask closeTask()");
        funcPtr = NULL;
    };

    virtual void actionButton() {
        Serial.println("CGeneralTimerTask actionButton()");
        Serial.println("timer init");

        timer = 0;

        startTime = millis();
        secondStart = startTime;
        timerStartFlag = true;

        funcPtr = &CGeneralTimerTask::timerFunc;
    }

    virtual void step() {
        if (funcPtr)
            (this->*funcPtr)();
    };

    virtual void leftButton() {
        Serial.println("CGeneralTimerTask leftButton()");
        actionButton();
    }

    virtual void rightButton() {
        Serial.println("CGeneralTimerTask rightButton()");
        actionButton();
    }

private:
    virtual void timerFunc() {
        const unsigned long currentTime(millis());

        // Process start.
        if (timerStartFlag) {
            Serial.print(" timer start = ");
            Serial.print(timer);
            Serial.println("s");

            display(timer, "s", startColumn);

            timerStartFlag = false;
        }

        // Process each second.
        if (currentTime - secondStart >= 1000) {
            timer++;
            secondStart = millis();

            Serial.print(" timer = ");
            Serial.print(timer);
            Serial.println("s");

            display(timer, "s", startColumn);

            // Long beep every 30s.
            if (timer % 30 == 0) {
                // Long beep.
                Beep(true);
            }
            else {
                // Short Beep.
                Beep();
            }
        }
    };

private:
    int timer;
    bool timerStartFlag;
    unsigned long startTime;
    unsigned long secondStart;
    const unsigned int startColumn;
    void (CGeneralTimerTask::* funcPtr)();
};


class TaskManager {
public:
    TaskManager()
        : numTasks(DarkroomTasks::End),
        activeTaskIndex(DarkroomTasks::EnlargerTimerTask) {
        tasks = new CTask * [numTasks];

        // Create the task list matching the order of the DarkroomTasks enum.
        Serial.print("Create ");
        Serial.print(numTasks);
        Serial.println(" tasks:");

        for (int task = 0; task < numTasks; task++) {
            tasks[task] = createObject((DarkroomTasks)task);
            Serial.print(" Created task ");
            Serial.println(tasks[task]->taskName());
        }

        assert(activeTaskIndex < numTasks);
        tasks[activeTaskIndex]->initializeTask();
        Serial.print("Active task is ");
        Serial.println(tasks[activeTaskIndex]->taskName());
    }

    ~TaskManager() {
        for (int i = 0; i < numTasks; ++i) {
            delete tasks[i];
        }

        delete[] tasks;
    }

    void step() {
        getTask()->step();
    }

    CTask* getTask() const {
        CTask* task = tasks[activeTaskIndex];
        if (task->initialized) {
            return task;
        }
        else {
            Serial.println("*** getTask() : task not initialized ***\n");
            assert(false);
            return NULL;
        }
    }

    void setTask(DarkroomTasks taskIndex) {
        tasks[activeTaskIndex]->closeTask();
        activeTaskIndex = taskIndex;
        tasks[activeTaskIndex]->initializeTask();
    }

    void setNextTask() {
        tasks[activeTaskIndex]->closeTask();
        activeTaskIndex = (activeTaskIndex + 1) % numTasks;
        tasks[activeTaskIndex]->initializeTask();
    }

    void setPreviousTask() {
        tasks[activeTaskIndex]->closeTask();
        activeTaskIndex = (activeTaskIndex + numTasks - 1) % numTasks;
        tasks[activeTaskIndex]->initializeTask();
    }

private:
    CTask* createObject(enum DarkroomTasks type) {

        switch (type) {

        case DarkroomTasks::LEDBrightnessTask:
            return new CLEDBrightnessTask();

        case DarkroomTasks::EnlargerOnOffTask:
            return new CEnlargerOnOffTask();

        case DarkroomTasks::EnlargerTimerTask:
            return new CEnlargerTimerTask();

        case DarkroomTasks::DevelopStopFixTask:
            return new CDevelopStopFixTask();

        case DarkroomTasks::GeneralTimerTask:
            return new CGeneralTimerTask();
        }
    }

private:
    CTask** tasks;
    const int numTasks;
    int activeTaskIndex;
};

// The TaskManager.
TaskManager* taskManager;

// Base class defining an input device.
class InputDevice
{
public:
    InputDevice(const int pin, std::function<void()> funcPtr)
        : pin(pin),
        funcPtr(funcPtr),
        switchDelay(50),
        previousState(false)
    {
    };

    virtual void CheckInput() = 0;

    void callFunction()
    {
        if (funcPtr)
            funcPtr();
    }

protected:
    const int pin;
    const int switchDelay;
    bool previousState;
    std::function<void()> funcPtr;
};

// Stores all created input devices.
class InputSet
{
public:
    InputSet()
        : deviceCount(0)
    {
    };

    void Add(InputDevice* inputDevice)
    {
        inputDevices[deviceCount++] = inputDevice;
    };

    void CheckInput()
    {
        for (int i = 0; i < deviceCount; i++)
        {
            inputDevices[i]->CheckInput();
        }
    };

private:
    int deviceCount;
    InputDevice* inputDevices[9];  // Keep things simple, fixed array for the 9 inputs.
};

// The button input device.
class ButtonDebouncing : public InputDevice
{
public:
    ButtonDebouncing(InputSet& inputSet, const int pin, std::function<void()> funcPtr)
        : InputDevice(pin, funcPtr),
        debounceDelay(50)
    {
        // Set button pin as input with internal pull-up resistors for the GamePad Shield.
        pinMode(pin, INPUT_PULLUP);

        // Add this instance to the input set.
        inputSet.Add(this);
    };

    virtual void CheckInput()
    {
      // Simple debounce: If button is pressed, check again after 50ms if button is still pressed. 
        const bool buttonPressedState(digitalRead(pin) == LOW);

        if (previousState != buttonPressedState) {
            if (buttonPressedState) {
                delay(debounceDelay);

                if (digitalRead(pin) == LOW)
                {
                    callFunction();
                }
            }

            previousState = buttonPressedState;
        }
    }

private:
    const int debounceDelay;
};

// Each analog input is divided into a lower half and upper half to map a switch.
enum AnalogRange
{
    LowerHalf,
    UpperHalf,
};

// The anaolg input device.
class AnalogDebouncing : public InputDevice
{
public:
    AnalogDebouncing(InputSet& inputSet, const int pin, const enum AnalogRange analogRange, std::function<void()> funcPtr)
        : InputDevice(pin, funcPtr),
        analogRange(analogRange),
        lowerThreshold(100),  // lower 100, 0+100
        upperThreshold(924),  // upper 100, 1024-100
        thresholdOffset(100),
        thresholdLength(10)
    {
        // Add this instance to the input set.
        inputSet.Add(this);
    };

    virtual void CheckInput()
    {
        const int position = analogRead(pin);
        bool analogState(false);

        // Add threshold to avoid flickering.
        // Check if input is for lower or upper half of the analog range.
        if (analogRange == AnalogRange::LowerHalf && position < lowerThreshold + thresholdOffset)
        {
            analogState = position < (lowerThreshold + (previousState ? thresholdLength : 0));
        }
        else
            if (analogRange == AnalogRange::UpperHalf && position > upperThreshold - thresholdOffset)
            {
                analogState = position > (upperThreshold + (previousState ? 0 : thresholdLength));
            }

        if (previousState != analogState) {
            if (analogState) {
                callFunction();
            }

            previousState = analogState;
        }
    }

private:
    const enum AnalogRange analogRange;
    const int lowerThreshold;
    const int upperThreshold;
    const int thresholdOffset;
    const int thresholdLength;
};

// Define the input.
// 5 Button and 4 analog inputs.
InputSet inputSet;

ButtonDebouncing kButton(inputSet, BUTTON_K, []() { taskManager->getTask()->actionButton(); }); // Joystick Button
ButtonDebouncing leftButton(inputSet, BUTTON_LEFT, []() { taskManager->getTask()->leftButton(); });
ButtonDebouncing rightButton(inputSet, BUTTON_RIGHT, []() { taskManager->getTask()->rightButton(); });
ButtonDebouncing upButton(inputSet, BUTTON_UP, []() { taskManager->setPreviousTask(); });
ButtonDebouncing downButton(inputSet, BUTTON_DOWN, []() { taskManager->setNextTask(); });

// Use the lower or upper half of the analog range as a switch.
AnalogDebouncing leftAnalog(inputSet, PIN_ANALOG_X, AnalogRange::LowerHalf, []() { taskManager->getTask()->leftButton(); });
AnalogDebouncing rightAnalog(inputSet, PIN_ANALOG_X, AnalogRange::UpperHalf, []() { taskManager->getTask()->rightButton(); });
AnalogDebouncing upAnalog(inputSet, PIN_ANALOG_Y, AnalogRange::UpperHalf, []() { taskManager->setPreviousTask(); });
AnalogDebouncing downAnalog(inputSet, PIN_ANALOG_Y, AnalogRange::LowerHalf, []() { taskManager->setNextTask(); });


// Run once.
void setup() {

    Serial.begin(9600);

    //while(!Serial); // Wait here until the serial port is ready.

    Serial.println("DarkroomTimer");

    // Setup the relais and beeper.
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(RelaisPin, OUTPUT);  // Set D9 as an output.
    pinMode(AudioPin, OUTPUT);   // Set D13 as an output.

    // Setup the 8x8 LEDs.
    for (int d = 0; d < numDevices; d++) {
        lc.shutdown(d, false);  // MAX72XX is in power-saving mode when startet.
        lc.setIntensity(d, 0);  // set Brightness 0..15
        lc.clearDisplay(d);
    }

    // Setup the main task manager to handle all the defined tasks.
    taskManager = new TaskManager();
    // taskManager.setTask(DarkroomTasks::EnlargerTimerTask);
    // taskManager.getTask()->actionButton();

    // Use the first 8x8 block for the darkroom lumination.
    FillSolidArea(0, 7);

    // Blink board LED 3x to signal end of setup.
    for (int i = 1; i <= 6; i++) {
        digitalWrite(LED_BUILTIN, i % 2);
        delay(100);
    }
}

// The main loop.
void loop() {

    // Read all button and analog input.
    inputSet.CheckInput();

    // Process the selected task.
    taskManager->step();
}
