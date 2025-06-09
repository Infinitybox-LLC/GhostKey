#include <WiFi.h>
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
#define RELAY_SECURITY_POS 13 //IO13 - Security POS //when security is disabled 12v on pos, ground is enabled, and open is open
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

// WiFi configuration
const char* ap_ssid = "GhostKey-Config";
const char* ap_password = "123456789";
bool wifiEnabled = false;

// Add these variables near the other button state variables
unsigned long buttonPressStartTime = 0;
bool isLongPressDetected = false;
#define CONFIG_MODE_PRESS_TIME 10000  // 10 seconds for config mode

// Add these variables near the other timing variables
#define LED_PWM_FREQ 5000
#define LED_PWM_CHANNEL 0
#define LED_PWM_RESOLUTION 8
#define LED_PWM_DUTY_CYCLE 255
int ledBrightness = 0;
int ledFadeAmount = 5;  // How much to fade the LED by each step

#define SHUTDOWN_DELAY 1000  // 1 second delay after shutdown

// Add with other global variables
unsigned long lastShutdownTime = 0;
bool isShuttingDown = false;
unsigned long starterPulseTime = STARTER_PULSE_TIME;  // Can be adjusted in config mode

// Timing constants
#define DEBOUNCE_DELAY 50        // 50ms debounce time
#define CONFIG_MODE_PRESS_TIME 10000  // 10 seconds for config mode
#define STARTER_PULSE_TIME 1500    // 1.5 seconds for starter
#define STATUS_PRINT_INTERVAL 1000  // Print status every second

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
    
    // Initialize SPIFFS first
    if(!SPIFFS.begin(true)) {
        DEBUG_PRINTLN("SPIFFS Mount Failed");
        return;
    }
    DEBUG_PRINTLN("SPIFFS initialized");
    
    setupPins();
    DEBUG_PRINTLN("Pins initialized");
    
    // Set RFID as authenticated for testing
    rfidState.isAuthenticated = true;
    DEBUG_PRINTLN("RFID authentication enabled for testing");
    
    //load saved settings
    preferences.begin("ghostkey", false);
    isConfigured = preferences.getBool("configured", false);
    isBluetoothPaired = preferences.getBool("bluetooth_paired", false);
    starterPulseTime = preferences.getULong("starter_pulse", 700);  // Load saved pulse time or use default
    DEBUG_PRINT("System configured: ");
    DEBUG_PRINTLN(isConfigured ? "Yes" : "No");
    DEBUG_PRINT("Bluetooth paired: ");
    DEBUG_PRINTLN(isBluetoothPaired ? "Yes" : "No");
    DEBUG_PRINT("Starter pulse time: ");
    DEBUG_PRINT(starterPulseTime);
    DEBUG_PRINTLN("ms");
    
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
    
    DEBUG_PRINTLN("=== System Initialization Complete ===\n");
}

void loop() {
    // Update system state
    updateSystemState();
    
    // Handle RFID
    handleRFID();
    
    // Handle button presses
    handleButtonPress();
    
    // Update relay states
    controlRelays();
    
    // Check auto-lock
    checkAutoLock();
    
    // Print system status periodically
    static unsigned long lastStatusPrint = 0;
    if (millis() - lastStatusPrint >= STATUS_PRINT_INTERVAL) {
        printSystemStatus();
        lastStatusPrint = millis();
    }
    
    // Small delay to prevent overwhelming the serial port
    delay(10);
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
    
    // Configure PWM for button LED
    ledcSetup(LED_PWM_CHANNEL, LED_PWM_FREQ, LED_PWM_RESOLUTION);
    ledcAttachPin(BUTTON_LED_PIN, LED_PWM_CHANNEL);
    
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
    ledcWrite(LED_PWM_CHANNEL, 0);
    
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

    // Handle button press detection
    if (buttonReading == LOW && !buttonPressed) {  // Button just pressed
        buttonPressed = true;
        buttonPressStartTime = millis();
        isLongPressDetected = false;
        DEBUG_BUTTON_PRINTLN("Button pressed");
    } 
    else if (buttonReading == HIGH && buttonPressed) {  // Button just released
        buttonPressed = false;
        DEBUG_BUTTON_PRINTLN("Button released");
        
        // If we were in a long press but didn't trigger config mode, reset
        if (isLongPressDetected) {
            isLongPressDetected = false;
        }
    }

    // Check for long press without brake
    if (buttonPressed && !brakeHeld && !isLongPressDetected) {
        unsigned long pressDuration = millis() - buttonPressStartTime;
        if (pressDuration >= CONFIG_MODE_PRESS_TIME) {
            isLongPressDetected = true;
            DEBUG_BUTTON_PRINTLN("Long press detected - Entering config mode");
            enterConfigMode();
        }
    }

    // Only process normal button operations if not in config mode
    if (currentState != CONFIG_MODE) {
        // Check for button press while brake is held
        if (buttonReading == LOW && brakeHeld && 
            (millis() - lastButtonPress) > DEBOUNCE_DELAY) {
            lastButtonPress = millis();
            
            if (rfidState.isAuthenticated) {
                if (engineRunning) {
                    // If engine is running, only allow turning off
                    DEBUG_PRINTLN("Engine sequence stopped");
                    engineRunning = false;
                    systemState = 0;  // Set to OFF
                    startRelayActive = false;
                    controlRelays();
                } else if (!startRelayActive) {
                    // Only allow starting sequence if engine is not running and not already starting
                    DEBUG_PRINTLN("Starting engine sequence...");
                    startRelayActive = true;
                    startRelayTimer = millis();
                    controlRelays();
                }
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
        }

        // Check for button press without brake
        if (buttonReading == HIGH && lastButtonReading == LOW && 
            brakeReading == HIGH && (millis() - lastButtonPress) > DEBOUNCE_DELAY) {
            // Only process button release if we're not in shutdown delay
            if (!isShuttingDown && !engineRunning && !startRelayActive) {
                if (rfidState.isAuthenticated) {  // Only allow normal sequence if engine isn't running
                    systemState = (systemState + 1) % 3;
                    DEBUG_BUTTON_PRINT("Button released. New state: ");
                    DEBUG_BUTTON_PRINTLN(systemState);
                    controlRelays();
                }
            }
            lastButtonPress = millis();
        }
    }

    // Update last states
    lastButtonReading = buttonReading;
    lastBrakeReading = brakeReading;
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
    // Handle start sequence timing
    if (startRelayActive) {
        if (millis() - startRelayTimer >= starterPulseTime) {
            DEBUG_PRINTLN("Start sequence complete - transitioning to running state");
            startRelayActive = false;
            engineRunning = true;
            systemState = 2;  // Set to IGNITION state when engine is running
            controlRelays();
        }
    }

    // Handle LED pulsing in config mode
    if (currentState == CONFIG_MODE) {
        static unsigned long lastLedUpdate = 0;
        if (millis() - lastLedUpdate > 20) {  // Update every 20ms for smooth fade
            ledBrightness = ledBrightness + ledFadeAmount;
            
            // Reverse the direction of the fading at the ends of the fade
            if (ledBrightness <= 0 || ledBrightness >= 255) {
                ledFadeAmount = -ledFadeAmount;
            }
            
            ledcWrite(LED_PWM_CHANNEL, ledBrightness);
            lastLedUpdate = millis();
        }
    } else {
        // Normal LED control when not in config mode
        if (brakeHeld) {
            ledcWrite(LED_PWM_CHANNEL, 255);  // Full brightness when brake is held
        } else {
            ledcWrite(LED_PWM_CHANNEL, 0);    // Off when brake is not held
        }
    }
}

void checkStartSequence() {
    if (startRelayActive) {
        unsigned long currentTime = millis();
        unsigned long elapsedTime = currentTime - startRelayTimer;
        DEBUG_PRINT("Start sequence time: ");
        DEBUG_PRINT(elapsedTime);
        DEBUG_PRINT(" / ");
        DEBUG_PRINTLN(starterPulseTime);
        
        if (elapsedTime >= starterPulseTime) {
            DEBUG_PRINTLN("Start sequence complete - transitioning to running state");
            startRelayActive = false;
            engineRunning = true;
            systemState = 2;  // Set to IGNITION state when engine is running
            controlRelays();
        }
    }
}

void controlRelays() {
    // Update system state based on engine running status
    if (engineRunning) {
        systemState = 2;  // Set to IGNITION when engine is running
    }

    // Update relay states based on system state and engine status
    if (engineRunning) {
        // Engine is running - all relays ON except START
        digitalWrite(RELAY_ACCESSORY, HIGH);
        digitalWrite(RELAY_IGNITION1, HIGH);
        digitalWrite(RELAY_IGNITION2, HIGH);
        digitalWrite(RELAY_START, LOW);
    } else if (startRelayActive) {
        // Engine is starting - only IGN2 and START relays ON
        digitalWrite(RELAY_ACCESSORY, LOW);
        digitalWrite(RELAY_IGNITION1, LOW);
        digitalWrite(RELAY_IGNITION2, HIGH);
        digitalWrite(RELAY_START, HIGH);
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

// call this when starting the vehicle
void triggerStartRelayPulse() {
    startRelayPulseStart = millis();
    startRelayPulsing = true;
}

void checkAutoLock() {
    // TBD
}

void enterConfigMode() {
    currentState = CONFIG_MODE;
    DEBUG_PRINTLN("Entering configuration mode");
    
    // Initialize WiFi and web server
    setupWiFi();
    setupWebServer();
    
    // Visual feedback
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
    }
    
    // Start LED pulsing
    ledBrightness = 0;
    ledFadeAmount = 5;
}

void exitConfigMode() {
    currentState = OFF;
    DEBUG_PRINTLN("Exiting configuration mode");
    
    // Stop WiFi and web server
    if (wifiEnabled) {
        WiFi.softAPdisconnect(true);
        wifiEnabled = false;
    }
    server.end();
    
    // Visual feedback
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    
    // Turn off LED
    ledcWrite(LED_PWM_CHANNEL, 0);
}

void setupWiFi() {
    DEBUG_PRINTLN("Starting WiFi Access Point...");
    WiFi.disconnect();  // Disconnect from any existing connections
    WiFi.mode(WIFI_AP);  // Set WiFi to AP mode
    WiFi.softAP(ap_ssid, ap_password);
    
    IPAddress IP = WiFi.softAPIP();
    DEBUG_PRINT("AP IP address: ");
    DEBUG_PRINTLN(IP);
    
    wifiEnabled = true;
}

void setupWebServer() {
    // Initialize SPIFFS
    if(!SPIFFS.begin(true)){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    // Serve static files from SPIFFS
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
    
    // Add MIME type for AVIF
    server.on("/logo.avif", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/logo.avif", "image/avif");
    });

    // Handle root path
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = "<!DOCTYPE html><html>";
        html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
        html += "<title>Ghost Key Configuration</title>";
        html += "<style>";
        html += "body { font-family: Arial; text-align: center; margin: 0px auto; padding: 20px; }";
        html += ".logo { position: absolute; top: 20px; left: 20px; width: 100px; height: auto; }";
        html += ".container { margin-top: 60px; }";
        html += ".button { background-color:rgb(170, 0, 225); border: none; color: white; padding: 16px 40px;";
        html += "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; }";
        html += ".button2 { background-color: #555555; }";
        html += ".form-group { margin: 20px 0; }";
        html += "input[type=number] { padding: 8px; font-size: 16px; width: 100px; }";
        html += ".popup { position: fixed; top: 20px; left: 50%; transform: translateX(-50%); ";
        html += "background-color: #4CAF50; color: white; padding: 15px 30px; ";
        html += "border-radius: 5px; display: none; z-index: 1000; }";
        html += "</style>";
        html += "<script>";
        html += "function showPopup(message) {";
        html += "  var popup = document.getElementById('popup');";
        html += "  popup.textContent = message;";
        html += "  popup.style.display = 'block';";
        html += "  setTimeout(function() { popup.style.display = 'none'; }, 5000);";
        html += "}";
        html += "function updatePulseTime(event) {";
        html += "  event.preventDefault();";
        html += "  var form = event.target;";
        html += "  var formData = new FormData(form);";
        html += "  fetch('/update_pulse', {";
        html += "    method: 'POST',";
        html += "    body: formData";
        html += "  })";
        html += "  .then(response => response.text())";
        html += "  .then(data => {";
        html += "    showPopup(data);";
        html += "  })";
        html += "  .catch(error => {";
        html += "    showPopup('Error: ' + error);";
        html += "  });";
        html += "}";
        html += "</script>";
        html += "</head>";
        html += "<body>";
        html += "<img src='/logo.avif' alt='Ghost Key Logo' class='logo'>";
        html += "<div class='container'>";
        html += "<div id='popup' class='popup'></div>";
        html += "<h1>Ghost Key Configuration</h1>";
        html += "<p>System Status: ";
        switch(currentState) {
            case OFF: html += "OFF"; break;
            case ACCESSORY: html += "ACCESSORY"; break;
            case IGNITION: html += "IGNITION"; break;
            case RUNNING: html += "RUNNING"; break;
            case CONFIG_MODE: html += "CONFIG MODE"; break;
        }
        html += "</p>";
        html += "<p>RFID Authenticated: ";
        html += rfidState.isAuthenticated ? "Yes" : "No";
        html += "</p>";
        html += "<p>Number of Stored Keys: ";
        html += numStoredKeys;
        html += "</p>";
        
        // Add starter pulse time configuration
        html += "<div class='form-group'>";
        html += "<h2>Starter Configuration</h2>";
        html += "<form onsubmit='updatePulseTime(event)'>";
        html += "<label for='pulse_time'>Starter Crank Time (ms): </label>";
        html += "<input type='number' id='pulse_time' name='pulse_time' min='100' max='2000' step='100' value='";
        html += starterPulseTime;
        html += "'>";
        html += "<button type='submit' class='button'>Update</button>";
        html += "</form>";
        html += "</div>";
        
        html += "<form action='/exit' method='post'>";
        html += "<button type='submit' class='button button2'>Exit Config Mode</button>";
        html += "</form>";
        html += "</div></body></html>";
        request->send(200, "text/html", html);
    });

    // Route for updating starter pulse time
    server.on("/update_pulse", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("pulse_time", true)) {
            String pulseTimeStr = request->getParam("pulse_time", true)->value();
            unsigned long newPulseTime = pulseTimeStr.toInt();
            
            // Validate the input
            if (newPulseTime >= 100 && newPulseTime <= 2000) {
                starterPulseTime = newPulseTime;
                preferences.putULong("starter_pulse", starterPulseTime);
                DEBUG_PRINT("Starter pulse time updated to: ");
                DEBUG_PRINT(starterPulseTime);
                DEBUG_PRINTLN("ms");
                request->send(200, "text/plain", "Starter crank time updated successfully");
            } else {
                request->send(400, "text/plain", "Invalid crank time. Must be between 100ms and 2000ms");
            }
        } else {
            request->send(400, "text/plain", "Missing pulse_time parameter");
        }
    });

    // Route for exiting config mode
    server.on("/exit", HTTP_POST, [](AsyncWebServerRequest *request){
        exitConfigMode();
        request->send(200, "text/plain", "Exiting config mode...");
    });

    // Start server
    server.begin();
    DEBUG_PRINTLN("Web server started");
}

void setupBluetooth() {
    // TBD
}

void printSystemStatus() {
    Serial.println("\nSystem State: ");
    if (engineRunning) {
        Serial.println("RUNNING");
    } else if (startRelayActive) {
        Serial.println("STARTING");
    } else {
        switch (systemState) {
            case 0:
                Serial.println("OFF");
                break;
            case 1:
                Serial.println("ACCESSORY");
                break;
            case 2:
                Serial.println("IGNITION");
                break;
        }
    }
    
    Serial.print("\nLED States - ACC: ");
    Serial.print(digitalRead(RELAY_ACCESSORY) ? "ON" : "OFF");
    Serial.print(" IGN1: ");
    Serial.print(digitalRead(RELAY_IGNITION1) ? "ON" : "OFF");
    Serial.print(" IGN2: ");
    Serial.print(digitalRead(RELAY_IGNITION2) ? "ON" : "OFF");
    Serial.print(" START: ");
    Serial.println(digitalRead(RELAY_START) ? "ON" : "OFF");
    Serial.println("========================\n");
} 