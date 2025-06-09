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

// Current monitoring
#define CURRENT_SENSE_PIN 36  // ADC1_CH0 (GPIO36)
#define SHUNT_RESISTOR 0.1   // 0.1 ohm shunt resistor
#define ADC_VREF 3.3         // ESP32 ADC reference voltage
#define ADC_RESOLUTION 4095  // 12-bit ADC resolution
#define CURRENT_SCALE 10     // Reduced scale for LED testing (adjust as needed)

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
    bool isPressed;
    bool isLongPress;
    unsigned long lastDebounceTime;
    unsigned long pressStartTime;
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
ButtonState buttonState = {false, false, false, false, 0, 0};
ButtonState brakeState = {false, false, false, false, 0, 0};
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
#define BUTTON_STEP_TIMEOUT 2000  // 2 seconds to reset button press sequence

// Debug flags
#define DEBUG_SYSTEM 1
#define DEBUG_BUTTON 1
#define DEBUG_RFID 1
#define DEBUG_RELAY 1

// Debug macros
#define DEBUG_PRINT(x) if(DEBUG_SYSTEM) Serial.print(x)
#define DEBUG_PRINTLN(x) if(DEBUG_SYSTEM) Serial.println(x)
#define DEBUG_BUTTON_PRINT(x) if(DEBUG_BUTTON) Serial.print(x)
#define DEBUG_BUTTON_PRINTLN(x) if(DEBUG_BUTTON) Serial.println(x)
#define DEBUG_RFID_PRINT(x) if(DEBUG_RFID) Serial.print(x)
#define DEBUG_RFID_PRINTLN(x) if(DEBUG_RFID) Serial.println(x)
#define DEBUG_RELAY_PRINT(x) if(DEBUG_RELAY) Serial.print(x)
#define DEBUG_RELAY_PRINTLN(x) if(DEBUG_RELAY) Serial.println(x)

// Button state tracking
unsigned long lastButtonPress = 0;
unsigned long lastBrakePress = 0;
unsigned long startRelayTimer = 0;
bool startRelayActive = false;
bool engineRunning = false;
int systemState = 0;  // 0 = off, 1 = accessory, 2 = accessory+ignition
const unsigned long START_RELAY_TIME = 700;  // 0.7 seconds

// For edge detection
bool lastButtonReading = HIGH;
bool lastBrakeReading = HIGH;
bool brakeHeld = false;
bool buttonPressed = false;

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
    DEBUG_PRINTLN("\n\n=== GhostKey System Starting ===");
    DEBUG_PRINTLN("Initializing system...");
    
    setupPins();
    DEBUG_PRINTLN("Pins initialized");
    
    setupWiFi();
    DEBUG_PRINTLN("WiFi initialized");
    
    setupWebServer();
    DEBUG_PRINTLN("Web server initialized");
    
    setupBluetooth();
    DEBUG_PRINTLN("Bluetooth initialized");
    
    // Set RFID as authenticated for testing
    rfidState.isAuthenticated = true;
    DEBUG_PRINTLN("RFID authentication enabled for testing");
    
    //load saved settings
    preferences.begin("ghostkey", false);
    isConfigured = preferences.getBool("configured", false);
    isBluetoothPaired = preferences.getBool("bluetooth_paired", false);
    DEBUG_PRINT("System configured: ");
    DEBUG_PRINTLN(isConfigured ? "Yes" : "No");
    DEBUG_PRINT("Bluetooth paired: ");
    DEBUG_PRINTLN(isBluetoothPaired ? "Yes" : "No");
    
    //load saved RFID keys
    numStoredKeys = preferences.getUInt("num_keys", 0);
    DEBUG_PRINT("Number of stored RFID keys: ");
    DEBUG_PRINTLN(numStoredKeys);
    
    if (numStoredKeys > 0) {
        preferences.getBytes("keys", storedKeys, sizeof(storedKeys));
        DEBUG_PRINTLN("RFID keys loaded from memory");
    }

    // Attach interrupt for LFDATA
    pinMode(RFID_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(RFID_PIN), lfdata_isr, CHANGE);
    DEBUG_PRINTLN("RFID interrupt attached");
    
    // Initialize current monitoring
    pinMode(CURRENT_SENSE_PIN, INPUT);
    analogSetWidth(12);  // Set ADC resolution to 12 bits
    analogSetAttenuation(ADC_11db);  // Set ADC attenuation for 0-3.3V range
    
    DEBUG_PRINTLN("=== System Initialization Complete ===\n");
}

void loop() {
    static unsigned long lastDebugTime = 0;
    
    // Print system state every 5 seconds
    if (millis() - lastDebugTime > 5000) {
        DEBUG_PRINT("Current System State: ");
        switch(currentState) {
            case OFF: DEBUG_PRINTLN("OFF"); break;
            case ACCESSORY: DEBUG_PRINTLN("ACCESSORY"); break;
            case IGNITION: DEBUG_PRINTLN("IGNITION"); break;
            case RUNNING: DEBUG_PRINTLN("RUNNING"); break;
            case CONFIG_MODE: DEBUG_PRINTLN("CONFIG_MODE"); break;
        }
        DEBUG_PRINT("RFID Authenticated: ");
        DEBUG_PRINTLN(rfidState.isAuthenticated ? "Yes" : "No");
        lastDebugTime = millis();
    }
    
    handleRFID();
    handleButtonPress();
    handleBrakeInput();
    
    // Handle starter relay pulsing
    if (startRelayPulsing) {
        if (millis() - startRelayPulseStart >= STARTER_PULSE_TIME) {
            DEBUG_RELAY_PRINTLN("Starter pulse complete");
            startRelayPulsing = false;
            digitalWrite(RELAY_START, LOW);
            // Turn back on accessory and ignition 1
            digitalWrite(RELAY_ACCESSORY, HIGH);
            digitalWrite(RELAY_IGNITION1, HIGH);
        }
    }
    
    checkAutoLock();
    
    // Print system status every second
    static unsigned long lastStatusTime = 0;
    if (millis() - lastStatusTime > 1000) {
        printSystemStatus();
        lastStatusTime = millis();
    }
    
    delay(50);
}

void setupPins() {
    //setup input pins with pullups (they're active low)
    pinMode(RFID_PIN, INPUT_PULLUP);
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
        DEBUG_RFID_PRINT("RFID Tag detected: 0x");
        char hexStr[9];
        sprintf(hexStr, "%08X", tagId);
        DEBUG_RFID_PRINT(hexStr);
        DEBUG_RFID_PRINTLN("");
        
        manchesterBitCount = 0;
        manchesterBuffer = 0;
        tagReady = false;

        if (isPairingMode) {
            DEBUG_RFID_PRINTLN("Pairing mode active - Attempting to add new key");
            if (addRFIDKey(tagId)) {
                DEBUG_RFID_PRINTLN("New key added successfully");
                digitalWrite(BUZZER_PIN, HIGH);
                delay(100);
                digitalWrite(BUZZER_PIN, LOW);
            } else {
                DEBUG_RFID_PRINTLN("Failed to add new key");
            }
        } else {
            rfidState.isAuthenticated = isTagAuthorized(tagId);
            DEBUG_RFID_PRINT("Tag authentication: ");
            DEBUG_RFID_PRINTLN(rfidState.isAuthenticated ? "Success" : "Failed");
            
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
    bool buttonReading = digitalRead(BUTTON_PIN);
    bool brakeReading = digitalRead(BRAKE_PIN);

    // Update brake held state and LED
    if (brakeReading == LOW && !brakeHeld) {
        brakeHeld = true;
        digitalWrite(BUTTON_LED_PIN, HIGH);
        DEBUG_BUTTON_PRINTLN("Brake held");
    } else if (brakeReading == HIGH && brakeHeld) {
        brakeHeld = false;
        digitalWrite(BUTTON_LED_PIN, LOW);
        DEBUG_BUTTON_PRINTLN("Brake released");
    }

    // Check for button press while brake is held
    if (buttonReading == LOW && !buttonPressed && brakeHeld && 
        (millis() - lastButtonPress) > DEBOUNCE_DELAY) {
        buttonPressed = true;
        lastButtonPress = millis();
        
        if (rfidState.isAuthenticated) {
            if (!startRelayActive && !engineRunning) {
                // Start the engine sequence
                startRelayActive = true;
                startRelayTimer = millis();
                systemState = 0;  // Reset normal sequence state
                DEBUG_BUTTON_PRINTLN("Starting engine sequence...");
            } else if (engineRunning) {
                // Turn everything off
                engineRunning = false;
                startRelayActive = false;
                systemState = 0;
                DEBUG_BUTTON_PRINTLN("Engine sequence stopped");
            }
            updateRelays();
        } else {
            DEBUG_BUTTON_PRINTLN("RFID not authenticated - Access denied");
            // Error feedback
            for (int i = 0; i < 3; i++) {
                digitalWrite(BUZZER_PIN, HIGH);
                delay(50);
                digitalWrite(BUZZER_PIN, LOW);
                delay(50);
            }
        }
    } else if (buttonReading == HIGH) {
        buttonPressed = false;
    }

    // Check for button press without brake
    if (buttonReading == HIGH && lastButtonReading == LOW && 
        brakeReading == HIGH && (millis() - lastButtonPress) > DEBOUNCE_DELAY) {
        if (rfidState.isAuthenticated && !engineRunning) {  // Only allow normal sequence if engine isn't running
            systemState = (systemState + 1) % 3;
            DEBUG_BUTTON_PRINT("Button released. New state: ");
            DEBUG_BUTTON_PRINTLN(systemState);
            updateRelays();
        }
        lastButtonPress = millis();
    }

    // Update last states
    lastButtonReading = buttonReading;
    lastBrakeReading = brakeReading;

    // Update relay states
    updateRelays();
}

void handleBrakeInput() {
    //brake is handled in handleButtonPress()
}

//function to handle button state changes
void updateButtonState(ButtonState &state, int pin) {
    // Inverting the reading since inputs are active-low
    bool reading = !digitalRead(pin);
    
    // If the button state has changed, start/restart the debounce timer
    if (reading != state.lastState) {
        state.lastDebounceTime = millis();
        DEBUG_BUTTON_PRINT("Button raw state changed to: ");
        DEBUG_BUTTON_PRINTLN(reading ? "HIGH" : "LOW");
    }
    
    // Only update the state if the reading has been stable for the debounce period
    if ((millis() - state.lastDebounceTime) > DEBOUNCE_DELAY) {
        // Only process if the reading is different from the current state
        if (reading != state.currentState) {
            state.currentState = reading;
            state.lastState = reading;
            
            DEBUG_BUTTON_PRINT("Button state stabilized to: ");
            DEBUG_BUTTON_PRINTLN(reading ? "HIGH" : "LOW");
        }
    }
}

void updateSystemState() {
    // TBD
}
// ADD:longer you hold the starter button the longer the starter engages (extra time for the starter to engage)
void controlRelays() {
    static SystemState lastState = OFF;
    static bool relaysInitialized = false;
    
    // Initialize relays only once
    if (!relaysInitialized) {
        DEBUG_RELAY_PRINTLN("Initializing relays to OFF state");
        digitalWrite(RELAY_ACCESSORY, LOW);
        digitalWrite(RELAY_IGNITION1, LOW);
        digitalWrite(RELAY_IGNITION2, LOW);
        digitalWrite(RELAY_START, LOW);
        digitalWrite(RELAY_SECURITY_POS, LOW);
        digitalWrite(RELAY_SECURITY_GND, LOW);
        digitalWrite(RELAY_SECURITY_OPEN, LOW);
        relaysInitialized = true;
    }
    
    // Only update relays if state has changed
    if (lastState != currentState) {
        DEBUG_RELAY_PRINT("System state changed from ");
        switch(lastState) {
            case OFF: DEBUG_RELAY_PRINT("OFF"); break;
            case ACCESSORY: DEBUG_RELAY_PRINT("ACCESSORY"); break;
            case IGNITION: DEBUG_RELAY_PRINT("IGNITION"); break;
            case RUNNING: DEBUG_RELAY_PRINT("RUNNING"); break;
            case CONFIG_MODE: DEBUG_RELAY_PRINT("CONFIG_MODE"); break;
        }
        DEBUG_RELAY_PRINT(" to ");
        switch(currentState) {
            case OFF: DEBUG_RELAY_PRINTLN("OFF"); break;
            case ACCESSORY: DEBUG_RELAY_PRINTLN("ACCESSORY"); break;
            case IGNITION: DEBUG_RELAY_PRINTLN("IGNITION"); break;
            case RUNNING: DEBUG_RELAY_PRINTLN("RUNNING"); break;
            case CONFIG_MODE: DEBUG_RELAY_PRINTLN("CONFIG_MODE"); break;
        }
        
        // Control relays based on state
        switch (currentState) {
            case ACCESSORY:
                DEBUG_RELAY_PRINTLN("Setting ACCESSORY relays");
                digitalWrite(RELAY_ACCESSORY, HIGH);
                digitalWrite(RELAY_IGNITION1, LOW);
                digitalWrite(RELAY_IGNITION2, LOW);
                break;
                
            case IGNITION:
                DEBUG_RELAY_PRINTLN("Setting IGNITION relays");
                digitalWrite(RELAY_ACCESSORY, HIGH);
                digitalWrite(RELAY_IGNITION1, HIGH);
                digitalWrite(RELAY_IGNITION2, HIGH);
                break;
                
            case RUNNING:
                DEBUG_RELAY_PRINTLN("Setting RUNNING relays");
                digitalWrite(RELAY_ACCESSORY, HIGH);
                digitalWrite(RELAY_IGNITION1, HIGH);
                digitalWrite(RELAY_IGNITION2, HIGH);
                
                if (startRelayPulsing) {
                    DEBUG_RELAY_PRINTLN("Starter relay pulsing");
                    digitalWrite(RELAY_IGNITION2, LOW);
                    digitalWrite(RELAY_ACCESSORY, LOW);
                    digitalWrite(RELAY_START, HIGH);
                    
                    if (millis() - startRelayPulseStart >= STARTER_PULSE_TIME) {
                        DEBUG_RELAY_PRINTLN("Starter pulse complete");
                        startRelayPulsing = false;
                        digitalWrite(RELAY_START, LOW);
                        digitalWrite(RELAY_IGNITION2, HIGH);
                        digitalWrite(RELAY_ACCESSORY, HIGH);
                    }
                }
                break;
                
            default:
                DEBUG_RELAY_PRINTLN("Setting all relays OFF");
                digitalWrite(RELAY_ACCESSORY, LOW);
                digitalWrite(RELAY_IGNITION1, LOW);
                digitalWrite(RELAY_IGNITION2, LOW);
                digitalWrite(RELAY_START, LOW);
                break;
        }
        
        lastState = currentState;
    }
    
    // Handle starter relay pulsing separately since it's time-based
    if (currentState == RUNNING && startRelayPulsing) {
        if (millis() - startRelayPulseStart >= STARTER_PULSE_TIME) {
            DEBUG_RELAY_PRINTLN("Starter pulse complete");
            startRelayPulsing = false;
            digitalWrite(RELAY_START, LOW);
            digitalWrite(RELAY_IGNITION2, HIGH);
            digitalWrite(RELAY_ACCESSORY, HIGH);
        }
    }
}

// call this when starting the vehicle
void triggerStartRelayPulse() {
    startRelayPulseStart = millis();
    startRelayPulsing = true;
}

void checkAutoLock() {
    // TBD
}

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

void updateRelays() {
    if (startRelayActive) {
        // Start sequence active
        digitalWrite(RELAY_IGNITION2, HIGH);
        digitalWrite(RELAY_START, HIGH);
        digitalWrite(RELAY_ACCESSORY, LOW);
        digitalWrite(RELAY_IGNITION1, LOW);
        
        // Check if start relay time has elapsed
        if (millis() - startRelayTimer >= START_RELAY_TIME) {
            startRelayActive = false;
            engineRunning = true;
            // Turn off start relay and turn on all other relays
            digitalWrite(RELAY_START, LOW);
            digitalWrite(RELAY_ACCESSORY, HIGH);
            digitalWrite(RELAY_IGNITION1, HIGH);
            digitalWrite(RELAY_IGNITION2, HIGH);
        }
    } else if (engineRunning) {
        // Engine is running - all relays on except start
        digitalWrite(RELAY_ACCESSORY, HIGH);
        digitalWrite(RELAY_IGNITION1, HIGH);
        digitalWrite(RELAY_IGNITION2, HIGH);
        digitalWrite(RELAY_START, LOW);
    } else {
        // Normal sequence states
        switch (systemState) {
            case 0:  // All off
                digitalWrite(RELAY_ACCESSORY, LOW);
                digitalWrite(RELAY_IGNITION1, LOW);
                digitalWrite(RELAY_IGNITION2, LOW);
                digitalWrite(RELAY_START, LOW);
                break;
            
            case 1:  // Accessory only
                digitalWrite(RELAY_ACCESSORY, HIGH);
                digitalWrite(RELAY_IGNITION1, LOW);
                digitalWrite(RELAY_IGNITION2, LOW);
                digitalWrite(RELAY_START, LOW);
                break;
            
            case 2:  // Accessory + Ignition 1 + Ignition 2
                digitalWrite(RELAY_ACCESSORY, HIGH);
                digitalWrite(RELAY_IGNITION1, HIGH);
                digitalWrite(RELAY_IGNITION2, HIGH);
                digitalWrite(RELAY_START, LOW);
                break;
        }
    }
}

float readCurrent() {
    // Read ADC value
    int adcValue = analogRead(CURRENT_SENSE_PIN);
    
    // Convert ADC value to voltage
    float voltage = (adcValue * ADC_VREF) / ADC_RESOLUTION;
    
    // Calculate current using Ohm's law (I = V/R)
    float current = voltage / SHUNT_RESISTOR;
    
    // Apply scaling factor
    current *= CURRENT_SCALE;
    
    return current;
}

void printSystemStatus() {
    float current = readCurrent();
    DEBUG_PRINTLN("\n=== System Power Status ===");
    DEBUG_PRINT("Total Current Draw: ");
    DEBUG_PRINT(current);
    DEBUG_PRINTLN(" mA");
    
    // Calculate expected current
    int activeLeds = 0;
    if (digitalRead(RELAY_ACCESSORY)) activeLeds++;
    if (digitalRead(RELAY_IGNITION1)) activeLeds++;
    if (digitalRead(RELAY_IGNITION2)) activeLeds++;
    if (digitalRead(RELAY_START)) activeLeds++;
    
    DEBUG_PRINTLN("\nPower Breakdown:");
    DEBUG_PRINT("- ESP32 Base: ~120 mA");
    DEBUG_PRINT("\n- Active LEDs: ");
    DEBUG_PRINT(activeLeds);
    DEBUG_PRINT(" × 20mA = ");
    DEBUG_PRINT(activeLeds * 20);
    DEBUG_PRINTLN(" mA");
    DEBUG_PRINT("- Other Peripherals: ~30 mA");
    
    DEBUG_PRINT("\nExpected Total: ~");
    DEBUG_PRINT(120 + (activeLeds * 20) + 30);
    DEBUG_PRINTLN(" mA");
    
    DEBUG_PRINTLN("\nSystem State: ");
    if (engineRunning) {
        DEBUG_PRINTLN("ENGINE RUNNING");
    } else if (startRelayActive) {
        DEBUG_PRINTLN("STARTING");
    } else {
        switch (systemState) {
            case 0:
                DEBUG_PRINTLN("OFF");
                break;
            case 1:
                DEBUG_PRINTLN("ACCESSORY");
                break;
            case 2:
                DEBUG_PRINTLN("IGNITION");
                break;
        }
    }
    
    DEBUG_PRINT("\nLED States - ACC: ");
    DEBUG_PRINT(digitalRead(RELAY_ACCESSORY) ? "ON" : "OFF");
    DEBUG_PRINT(" IGN1: ");
    DEBUG_PRINT(digitalRead(RELAY_IGNITION1) ? "ON" : "OFF");
    DEBUG_PRINT(" IGN2: ");
    DEBUG_PRINT(digitalRead(RELAY_IGNITION2) ? "ON" : "OFF");
    DEBUG_PRINT(" START: ");
    DEBUG_PRINTLN(digitalRead(RELAY_START) ? "ON" : "OFF");
    DEBUG_PRINTLN("========================\n");
} 