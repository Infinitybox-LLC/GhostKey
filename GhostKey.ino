#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

//pins we're using all TBc
#define RFID_PIN 4          //IO4 - RFID IN
#define BUTTON_PIN 19       //IO19 - Start Button (Grounded When Active)
#define BRAKE_PIN 22        //IO22 - Brake Pedal Input (Grounded When Active)
#define LED_PIN 23          //IO23 - LED Indicator Light
#define BUZZER_PIN 34       //IO34 - Beeper Output
#define CONFIG_BUTTON_PIN 25 //IO25 - Configuration Button (Grounded When Active)
#define BUTTON_LED_PIN 18   //IO18 - Start Button Indicator Light

//relay pins also TBC
#define RELAY_ACCESSORY 26  //IO26 - Accessory
#define RELAY_IGNITION1 27  //IO27 - Ignition 1
#define RELAY_IGNITION2 14  //IO14 - Ignition 2
#define RELAY_START 12      //IO12 - Start
#define RELAY_SECURITY_POS 13 //IO13 - Security POS
#define RELAY_SECURITY_GND 15 //IO15 - Security Ground
#define RELAY_SECURITY_OPEN 2  //IO2 - Security NO Relay

//timing stuff
#define CONFIG_MODE_TIMEOUT 30000  //30 seconds
#define AUTO_LOCK_TIMEOUT 30000    //30 seconds
#define STARTER_PULSE_TIME 700     //0.7 seconds
#define DEBOUNCE_DELAY 50        //50ms debounce time
#define LONG_PRESS_TIME 30000    //30 seconds for long press
#define BUTTON_LED_BLINK_RATE 500 //500ms for LED blink rate
#define MAX_STORED_KEYS 10
#define RFID_TIMEOUT 5000  //5 seconds timeout for RFID stuff

//what the system's current status is
enum SystemState {
    OFF,
    ACCESSORY,
    IGNITION,
    RUNNING,
    CONFIG_MODE
};

//keep track of button settings
struct ButtonState {
    bool currentState;
    bool lastState;
    unsigned long lastDebounceTime;
    unsigned long pressStartTime;
    bool isPressed;
    bool isLongPress;
};

//keep track of RFID key and state
struct RFIDKey {
    uint32_t id;
    bool isActive;
};

struct RFIDState {
    bool isReading;
    unsigned long lastReadTime;
    uint32_t currentTag;
    bool isAuthenticated;
};

//global variables
SystemState currentState = OFF;
unsigned long lastActivityTime = 0;
bool isConfigured = false;
bool isBluetoothPaired = false;
AsyncWebServer server(80);
Preferences preferences;

//initialize button and RFID states
ButtonState buttonState = {false, false, 0, 0, false, false};
ButtonState brakeState = {false, false, 0, 0, false, false};
RFIDState rfidState = {false, 0, 0, false};
RFIDKey storedKeys[MAX_STORED_KEYS];
uint8_t numStoredKeys = 0;
bool isPairingMode = false;

// Manchester decoding state
volatile uint32_t manchesterBuffer = 0;
volatile int manchesterBitCount = 0;
volatile bool tagReady = false;
volatile unsigned long lastEdgeTime = 0;
volatile bool lastLfdataState = false;

#define MANCHESTER_BIT_PERIOD 400 // us, adjust as needed for your tag (typical 2.4kHz = ~416us per bit)

// relay pulse timing
unsigned long startRelayPulseStart = 0;
bool startRelayPulsing = false;

unsigned long lastButtonReleaseTime = 0;
int buttonPressStep = 0;
#define BUTTON_STEP_TIMEOUT 2000 //2 seconds to reset button press sequence

void IRAM_ATTR lfdata_isr() {
    unsigned long now = micros();
    bool lfdata = digitalRead(RFID_PIN);
    unsigned long dt = now - lastEdgeTime;
    lastEdgeTime = now;

    // Only process if not already got a tag
    if (tagReady) return;

    // Manchester decoding: look for transitions at expected intervals
    if (dt > (MANCHESTER_BIT_PERIOD / 2) && dt < (MANCHESTER_BIT_PERIOD * 1.5)) {
        // Valid bit period, decode bit
        // Manchester: transition direction determines bit value
        // High-to-low = 0, low-to-high = 1 (typical, check your tag type)
        if (lfdata != lastLfdataState) {
            // Rising edge = 1, falling edge = 0
            manchesterBuffer <<= 1;
            if (lfdata) {
                manchesterBuffer |= 1; // 1 bit
            } // else 0 bit
            manchesterBitCount++;
        }
    } else {
        // If timing is off, reset
        manchesterBitCount = 0;
        manchesterBuffer = 0;
    }
    lastLfdataState = lfdata;

    // If we have 32 bits, set flag
    if (manchesterBitCount >= 32) {
        tagReady = true;
    }
}

//function declarations
void setupPins();
void setupWiFi();
void setupWebServer();
void setupBluetooth();
void handleRFID();
void handleButtonPress();
void handleBrakeInput();
void updateSystemState();
void controlRelays();
void checkAutoLock();
void enterConfigMode();
void exitConfigMode();

void setup() {
    Serial.begin(115200);
    setupPins();
    setupWiFi();
    setupWebServer();
    setupBluetooth();
    
    //load saved settings
    preferences.begin("ghostkey", false);
    isConfigured = preferences.getBool("configured", false);
    isBluetoothPaired = preferences.getBool("bluetooth_paired", false);
    
    //load saved RFID keys
    numStoredKeys = preferences.getUInt("num_keys", 0);
    if (numStoredKeys > 0) {
        preferences.getBytes("keys", storedKeys, sizeof(storedKeys));
    }

    // Attach interrupt for LFDATA
    pinMode(RFID_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(RFID_PIN), lfdata_isr, CHANGE);
}

void loop() {
    handleRFID();
    handleButtonPress();
    handleBrakeInput();
    updateSystemState();
    controlRelays();
    checkAutoLock();
    
    
    delay(10);
}

void setupPins() {
    //setup input pins with pullups (they're active low)
    pinMode(RFID_PIN, INPUT); // does need a pullup
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BRAKE_PIN, INPUT_PULLUP);
    pinMode(CONFIG_BUTTON_PIN, INPUT_PULLUP);
    
    //setup output pins
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BUTTON_LED_PIN, OUTPUT);
    
    //setup relay pins
    pinMode(RELAY_ACCESSORY, OUTPUT);
    pinMode(RELAY_IGNITION1, OUTPUT);
    pinMode(RELAY_IGNITION2, OUTPUT);
    pinMode(RELAY_START, OUTPUT);
    pinMode(RELAY_SECURITY_POS, OUTPUT);
    pinMode(RELAY_SECURITY_GND, OUTPUT);
    pinMode(RELAY_SECURITY_OPEN, OUTPUT);
    
    //turn everything off to start
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(BUTTON_LED_PIN, LOW);
    
    digitalWrite(RELAY_ACCESSORY, LOW);
    digitalWrite(RELAY_IGNITION1, LOW);
    digitalWrite(RELAY_IGNITION2, LOW);
    digitalWrite(RELAY_START, LOW);
    digitalWrite(RELAY_SECURITY_POS, LOW);
    digitalWrite(RELAY_SECURITY_GND, LOW);
    digitalWrite(RELAY_SECURITY_OPEN, LOW);
}

//check if this tag is allowed
bool isTagAuthorized(uint32_t tagId) {
    for (int i = 0; i < numStoredKeys; i++) {
        if (storedKeys[i].id == tagId && storedKeys[i].isActive) {
            return true;
        }
    }
    return false;
}

//add a new RFID key
bool addRFIDKey(uint32_t tagId) {
    if (numStoredKeys >= MAX_STORED_KEYS) return false;
    
    //check if we already have this tag
    for (int i = 0; i < numStoredKeys; i++) {
        if (storedKeys[i].id == tagId) {
            storedKeys[i].isActive = true;
            return true;
        }
    }
    
    //add the new tag
    storedKeys[numStoredKeys].id = tagId;
    storedKeys[numStoredKeys].isActive = true;
    numStoredKeys++;
    
    //save it
    preferences.putUInt("num_keys", numStoredKeys);
    preferences.putBytes("keys", storedKeys, sizeof(storedKeys));
    
    return true;
}

//remove an RFID key
bool removeRFIDKey(uint32_t tagId) {
    for (int i = 0; i < numStoredKeys; i++) {
        if (storedKeys[i].id == tagId) {
            //move everything up
            for (int j = i; j < numStoredKeys - 1; j++) {
                storedKeys[j] = storedKeys[j + 1];
            }
            numStoredKeys--;
            
            //save the changes
            preferences.putUInt("num_keys", numStoredKeys);
            preferences.putBytes("keys", storedKeys, sizeof(storedKeys));
            
            return true;
        }
    }
    return false;
}

void handleRFID() {
    if (tagReady) {
        uint32_t tagId = manchesterBuffer;
        manchesterBitCount = 0;
        manchesterBuffer = 0;
        tagReady = false;

        if (isPairingMode) {
            if (addRFIDKey(tagId)) {
                digitalWrite(BUZZER_PIN, HIGH);
                delay(100);
                digitalWrite(BUZZER_PIN, LOW);
            }
        } else {
            rfidState.isAuthenticated = isTagAuthorized(tagId);
            if (rfidState.isAuthenticated) {
                digitalWrite(BUZZER_PIN, HIGH);
                delay(100);
                digitalWrite(BUZZER_PIN, LOW);
            } else {
                for (int i = 0; i < 3; i++) {
                    digitalWrite(BUZZER_PIN, HIGH);
                    delay(50);
                    digitalWrite(BUZZER_PIN, LOW);
                    delay(50);
                }
            }
        }
    }
}

void handleButtonPress() {
    updateButtonState(buttonState, BUTTON_PIN);
    updateButtonState(brakeState, BRAKE_PIN);
    
    //handle the LED
    if (currentState == CONFIG_MODE) {
        digitalWrite(BUTTON_LED_PIN, (millis() % 200) < 100);
    } else if (buttonState.isPressed) {
        digitalWrite(BUTTON_LED_PIN, HIGH);
    } else {
        digitalWrite(BUTTON_LED_PIN, LOW);
    }

    //reset step if too much time passes
    if ((millis() - lastButtonReleaseTime) > BUTTON_STEP_TIMEOUT && buttonPressStep > 0) {
        buttonPressStep = 0;
    }

    //handle button combos
    if (buttonState.currentState && !buttonState.lastState) {  //button just pressed
        if (brakeState.currentState) {  //brake is pressed
            if (rfidState.isAuthenticated) {
                currentState = RUNNING;
                triggerStartRelayPulse();
                buttonPressStep = 0;
            } else {
                for (int i = 0; i < 3; i++) {
                    digitalWrite(BUZZER_PIN, HIGH);
                    delay(50);
                    digitalWrite(BUZZER_PIN, LOW);
                    delay(50);
                }
            }
        } else { //no brake
            if (rfidState.isAuthenticated) {
                buttonPressStep++;
                if (buttonPressStep == 1) {
                    currentState = ACCESSORY;
                } else if (buttonPressStep == 2) {
                    currentState = IGNITION;
                } else if (buttonPressStep == 3) {
                    currentState = OFF;
                    buttonPressStep = 0;
                }
            } else {
                for (int i = 0; i < 3; i++) {
                    digitalWrite(BUZZER_PIN, HIGH);
                    delay(50);
                    digitalWrite(BUZZER_PIN, LOW);
                    delay(50);
                }
            }
        }
    }
    if (!buttonState.currentState && buttonState.lastState) { //button just released
        lastButtonReleaseTime = millis();
    }
    //check for config mode
    if (buttonState.isLongPress && brakeState.currentState) {
        enterConfigMode();
    }
}

void handleBrakeInput() {
    //brake is handled in handleButtonPress()
}


//function to handle button state changes
void updateButtonState(ButtonState &state, int pin) {
    //inverting the reading since inputs are active-low
    bool reading = !digitalRead(pin);
    
    //checking if the button state has changed
    if (reading != state.lastState) {
        state.lastDebounceTime = millis();
    }
    
    //if the button state has been stable for the debounce period
    if ((millis() - state.lastDebounceTime) > DEBOUNCE_DELAY) {
        if (reading != state.currentState) {
            state.currentState = reading;
            
            if (state.currentState) {  // Button press
                state.pressStartTime = millis();
                state.isPressed = true;
                state.isLongPress = false;
            } else {  // Button releas
                state.isPressed = false;
                if ((millis() - state.pressStartTime) >= LONG_PRESS_TIME) {
                    state.isLongPress = true;
                }
            }
        }
    }
    
    state.lastState = reading;
}

void updateSystemState() {
    // TBD
}
// ADD:longer you hold the starter button the longer the starter engages (extra time for the starter to engage)
void controlRelays() {
    digitalWrite(RELAY_ACCESSORY, LOW);
    digitalWrite(RELAY_IGNITION1, LOW);
    digitalWrite(RELAY_IGNITION2, LOW);
    digitalWrite(RELAY_START, LOW);
    digitalWrite(RELAY_SECURITY_POS, LOW);
    digitalWrite(RELAY_SECURITY_GND, LOW);
    digitalWrite(RELAY_SECURITY_OPEN, LOW);

    // if state turns to accessory, turn on accessory relay
    if (currentState == ACCESSORY) {
        digitalWrite(RELAY_ACCESSORY, HIGH);
    // if state turns to ignition, turn on accessory and ignition relays
    } else if (currentState == IGNITION) {
        digitalWrite(RELAY_ACCESSORY, HIGH);
        digitalWrite(RELAY_IGNITION1, HIGH);
        digitalWrite(RELAY_IGNITION2, HIGH);
    // if state turns to running, turn on accessory and ignition relays and start the starter relay pulse
    } else if (currentState == RUNNING) {
        digitalWrite(RELAY_ACCESSORY, HIGH);
        digitalWrite(RELAY_IGNITION1, HIGH);
        digitalWrite(RELAY_IGNITION2, HIGH);
        if (startRelayPulsing) {
            digitalWrite(RELAY_IGNITION2, LOW);
            digitalWrite(RELAY_ACCESSORY, LOW);
            digitalWrite(RELAY_START, HIGH);
            if (millis() - startRelayPulseStart >= 700) {
                startRelayPulsing = false;
            }
        }
    }
    // add more logic for security relays if needed
    // need to turn off relay 2 and accessory when cranking, they turn back on when cranking is over
}

// call this when starting the vehicle
void triggerStartRelayPulse() {
    startRelayPulseStart = millis();
    startRelayPulsing = true;
}

void checkAutoLock() {
    // TBD

void enterConfigMode() {
    // TBD
}

void exitConfigMode() {
    // TBD
}

void setupWiFi() {
    // TBD
}

void setupWebServer() {
    // TBD
}

void setupBluetooth() {
    // TBD
} 