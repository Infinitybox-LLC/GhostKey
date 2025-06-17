#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BleKeyboard.h>
#include <nvs_flash.h>
#include <esp_gap_ble_api.h>
#include <ArduinoJson.h>
#include "config_html.h"

// Debug levels for reducing serial output
#define DEBUG_LEVEL_NONE 0
#define DEBUG_LEVEL_BASIC 1
#define DEBUG_LEVEL_VERBOSE 2
#define DEBUG_LEVEL_FULL 3
#define CURRENT_DEBUG_LEVEL DEBUG_LEVEL_BASIC  // Change this to control verbosity

// Optimized logging macros to reduce serial overhead
#define DEBUG_BLE_BASIC(x) do { if(CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_BASIC) { Serial.print(x); } } while(0)
#define DEBUG_BLE_BASICLN(x) do { if(CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_BASIC) { Serial.println(x); } } while(0)
#define DEBUG_BLE_BASICF(...) do { if(CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_BASIC) { Serial.printf(__VA_ARGS__); } } while(0)

#define DEBUG_BLE_VERBOSE(x) do { if(CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE) { Serial.print(x); } } while(0)
#define DEBUG_BLE_VERBOSELN(x) do { if(CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE) { Serial.println(x); } } while(0)
#define DEBUG_BLE_VERBOSEF(...) do { if(CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE) { Serial.printf(__VA_ARGS__); } } while(0)

//pins we're using all TBc
#define RFID_PIN 4          //IO4 - RFID IN
#define BUTTON_PIN 19       //IO19 - Start Button (Grounded When Active)
#define BRAKE_PIN 22        //IO22 - Brake Pedal Input (Grounded When Active)
#define LED_PIN 23          //IO23 - LED Indicator Light
// Test button removed - no longer needed
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
#define STARTER_PULSE_TIME 1500    // 1.5 seconds for starter
#define DEBOUNCE_DELAY 50        //50ms debounce time
#define LONG_PRESS_TIME 30000    //30 seconds for long press
#define BUTTON_LED_BLINK_RATE 500 //500ms for LED blink rate
#define MAX_STORED_KEYS 10
#define RFID_TIMEOUT 5000  //5 seconds timeout for RFID stuff
#define PAIRING_MODE_TIMEOUT 30000  //30 seconds timeout for pairing mode

// Current monitoring
#define CURRENT_SENSE_PIN 36  // ADC1_CH0 (GPIO36)
#define SHUNT_RESISTOR 0.1   // 0.1 ohm shunt resistor
#define ADC_VREF 3.3         // ESP32 ADC reference voltage
#define ADC_RESOLUTION 4095  // 12-bit ADC resolution
#define CURRENT_SCALE 10     // Reduced scale for LED testing (adjust as needed)

// BLE Configuration
#define MAX_BONDS 3  // Maximum number of devices that can be bonded
#define BOND_STORAGE_NAMESPACE "ble_bonds"
#define DEVICE_NAME_KEY "dev_name_"
#define DEVICE_PRIORITY_KEY "dev_priority_"
#define MAX_RSSI -30  // Strongest signal we expect
#define MIN_RSSI -100 // Weakest signal we expect
#define RSSI_AUTH_THRESHOLD -68  // RSSI threshold for authentication (~4 feet)
#define RSSI_DEAUTH_THRESHOLD -73  // RSSI threshold for deauthentication (hysteresis)

// BLE Timing definitions
#define RSSI_UPDATE_INTERVAL 5000 // Update RSSI every 5 seconds
#define CONNECTION_TIMEOUT 30000  // 30 seconds connection timeout
#define MIN_CONN_INTERVAL 0x10    // 20ms
#define MAX_CONN_INTERVAL 0x20    // 40ms
#define SLAVE_LATENCY 0
#define SUPERVISION_TIMEOUT 0x100 // 1 second

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
WebServer server(80);
Preferences preferences;

// BLE Global variables
bool bluetoothAuthenticated = false;  // Separate from RFID authentication
bool isBleConnected = false;
esp_bd_addr_t connectedDeviceAddr = {0};  // Store currently connected device address
bool hasConnectedDevice = false;  // Track if we have a device connected

BleKeyboard bleKeyboard("Ghost-Key Secure", "Ghost-Key Inc.", 100);
BLEServer* pServer = nullptr;

// Forward declaration of BLE callback class
class MyServerCallbacks;

//initialize button and RFID states
ButtonState buttonState = {false, false, false, false, 0, 0};
ButtonState brakeState = {false, false, false, false, 0, 0};
RFIDState rfidState = {false, 0, 0, false};
RFIDKey storedKeys[MAX_STORED_KEYS];
uint8_t numStoredKeys = 0;
bool isPairingMode = false;
unsigned long pairingModeStartTime = 0;

// BLE Device structures for caching
struct RSSICache {
    esp_bd_addr_t address;
    int8_t rssi;
    unsigned long lastUpdate;
    bool valid;
};

struct DeviceNameCache {
    esp_bd_addr_t address;
    char name[32];
    bool hasCustomName;  // True if user set a custom name
    bool nvsStored;      // True if stored in NVS
    bool valid;
};

struct DevicePriorityCache {
    esp_bd_addr_t address;
    bool isPriority;
    bool nvsStored;      // True if stored in NVS
    bool valid;
};

#define RSSI_CACHE_SIZE 10
#define NAME_CACHE_SIZE 10
#define PRIORITY_CACHE_SIZE 10
RSSICache rssiCache[RSSI_CACHE_SIZE];
DeviceNameCache nameCache[NAME_CACHE_SIZE];
DevicePriorityCache priorityCache[PRIORITY_CACHE_SIZE];

// Shared buffer for bonded device operations
static esp_ble_bond_dev_t sharedBondedDevicesBuffer[MAX_BONDS];
static bool sharedBufferInUse = false;

// Cache for JSON generation
static String cachedDevicesJson = "";
static unsigned long lastJsonUpdate = 0;
static int lastDeviceCount = -1;
static bool jsonCacheValid = false;

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
#define CONFIG_MODE_PRESS_TIME 10000  // 10 seconds for config mode
#define STATUS_PRINT_INTERVAL 1000  // Print status every second

// Add with other global variables
bool securityEnabled = false;
unsigned long lastSecurityCheck = 0;
unsigned long lastEngineShutdown = 0;  // Track when engine was last running
#define SECURITY_CHECK_INTERVAL 1000  // Check security state every second

// Add with other global variables
unsigned long autoLockTimeout = AUTO_LOCK_TIMEOUT;  // Can be adjusted in config mode

// Test button state removed - no longer needed

// ===== BLE FUNCTIONS =====

// Function to clean up only our custom NVS storage (NOT the BLE bonding data)
void cleanupNVSStorage() {
    Serial.println("=== STARTING CUSTOM NVS CLEANUP ===");
    
    // Only clean our custom namespace, NOT the entire NVS
    nvs_handle_t handle;
    esp_err_t err = nvs_open(BOND_STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        Serial.println("Erasing our custom namespace only...");
        nvs_erase_all(handle);
        nvs_commit(handle);
        nvs_close(handle);
        Serial.println("Custom namespace cleaned successfully");
    } else {
        Serial.printf("Could not open custom namespace for cleanup: %d\n", err);
    }
    
    Serial.println("=== CUSTOM NVS CLEANUP COMPLETE ===");
    Serial.println("Note: BLE bonding data preserved");
}

// Helper function to get bonded devices using shared buffer
esp_ble_bond_dev_t* getBondedDevicesSafe(int* count) {
    if (sharedBufferInUse) {
        int bondedCount = esp_ble_get_bond_device_num();
        if (bondedCount == 0) {
            *count = 0;
            return nullptr;
        }
        esp_ble_bond_dev_t* tempBuffer = (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * bondedCount);
        if (tempBuffer) {
            esp_ble_get_bond_device_list(&bondedCount, tempBuffer);
            *count = bondedCount;
        }
        return tempBuffer;
    } else {
        sharedBufferInUse = true;
        int bondedCount = esp_ble_get_bond_device_num();
        if (bondedCount == 0) {
            *count = 0;
            sharedBufferInUse = false;
            return nullptr;
        }
        esp_ble_get_bond_device_list(&bondedCount, sharedBondedDevicesBuffer);
        *count = bondedCount;
        return sharedBondedDevicesBuffer;
    }
}

// Helper function to release bonded devices buffer
void releaseBondedDevicesBuffer(esp_ble_bond_dev_t* buffer) {
    if (buffer == sharedBondedDevicesBuffer) {
        sharedBufferInUse = false;
    } else if (buffer != nullptr) {
        free(buffer);
    }
}

// Invalidate JSON cache when devices change
void invalidateDeviceCache() {
    jsonCacheValid = false;
    cachedDevicesJson = "";
}

// Function to find or create name cache entry
int findOrCreateNameCacheEntry(esp_bd_addr_t address) {
    // First, look for existing entry
    for (int i = 0; i < NAME_CACHE_SIZE; i++) {
        if (nameCache[i].valid && 
            memcmp(nameCache[i].address, address, sizeof(esp_bd_addr_t)) == 0) {
            return i;
        }
    }
    
    // Look for empty slot
    for (int i = 0; i < NAME_CACHE_SIZE; i++) {
        if (!nameCache[i].valid) {
            memcpy(nameCache[i].address, address, sizeof(esp_bd_addr_t));
            strcpy(nameCache[i].name, "Unknown Device");
            nameCache[i].hasCustomName = false;
            nameCache[i].nvsStored = false;
            nameCache[i].valid = true;
            return i;
        }
    }
    
    // No empty slot, replace oldest entry
    memcpy(nameCache[0].address, address, sizeof(esp_bd_addr_t));
    strcpy(nameCache[0].name, "Unknown Device");
    nameCache[0].hasCustomName = false;
    nameCache[0].nvsStored = false;
    nameCache[0].valid = true;
    return 0;
}

// Function to save device name to cache and conditionally to NVS
void saveDeviceName(esp_bd_addr_t address, const char* name) {
    int cacheIndex = findOrCreateNameCacheEntry(address);
    
    // Update cache
    strncpy(nameCache[cacheIndex].name, name, sizeof(nameCache[cacheIndex].name) - 1);
    nameCache[cacheIndex].name[sizeof(nameCache[cacheIndex].name) - 1] = '\0';
    nameCache[cacheIndex].hasCustomName = true;
    
    // Save user-set custom names to NVS
    nvs_handle_t handle;
    esp_err_t err = nvs_open(BOND_STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        char key[64];
        snprintf(key, sizeof(key), "%s%02x%02x%02x%02x%02x%02x", 
                 DEVICE_NAME_KEY, address[0], address[1], address[2], 
                 address[3], address[4], address[5]);

        err = nvs_set_str(handle, key, name);
        if (err == ESP_OK) {
            nvs_commit(handle);
            nameCache[cacheIndex].nvsStored = true;
            Serial.printf("Device name saved: %s\n", name);
        }
        nvs_close(handle);
    }
}

// Function to get device name
bool getDeviceName(esp_bd_addr_t address, char* name, size_t max_len) {
    // First check cache
    for (int i = 0; i < NAME_CACHE_SIZE; i++) {
        if (nameCache[i].valid && 
            memcmp(nameCache[i].address, address, sizeof(esp_bd_addr_t)) == 0) {
            strncpy(name, nameCache[i].name, max_len - 1);
            name[max_len - 1] = '\0';
            return true;
        }
    }
    
    // Not in cache, try to load from NVS
    nvs_handle_t handle;
    esp_err_t err = nvs_open(BOND_STORAGE_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        return false;
    }

    char key[64];
    snprintf(key, sizeof(key), "%s%02x%02x%02x%02x%02x%02x", 
             DEVICE_NAME_KEY, address[0], address[1], address[2], 
             address[3], address[4], address[5]);

    size_t required_size = max_len;
    err = nvs_get_str(handle, key, name, &required_size);
    nvs_close(handle);
    
    if (err == ESP_OK) {
        // Store in cache for future access
        int cacheIndex = findOrCreateNameCacheEntry(address);
        strncpy(nameCache[cacheIndex].name, name, sizeof(nameCache[cacheIndex].name) - 1);
        nameCache[cacheIndex].name[sizeof(nameCache[cacheIndex].name) - 1] = '\0';
        nameCache[cacheIndex].hasCustomName = true;
        nameCache[cacheIndex].nvsStored = true;
        return true;
    }
    
    return false;
}

// Function to save device RSSI in memory cache only
void saveDeviceRSSI(esp_bd_addr_t address, int8_t rssi) {
    int targetIndex = -1;
    unsigned long oldestTime = ULONG_MAX;
    
    for (int i = 0; i < RSSI_CACHE_SIZE; i++) {
        if (!rssiCache[i].valid) {
            targetIndex = i;
            break;
        }
        
        if (memcmp(rssiCache[i].address, address, sizeof(esp_bd_addr_t)) == 0) {
            targetIndex = i;
            break;
        }
        
        if (rssiCache[i].lastUpdate < oldestTime) {
            oldestTime = rssiCache[i].lastUpdate;
            targetIndex = i;
        }
    }
    
    if (targetIndex >= 0) {
        memcpy(rssiCache[targetIndex].address, address, sizeof(esp_bd_addr_t));
        rssiCache[targetIndex].rssi = rssi;
        rssiCache[targetIndex].lastUpdate = millis();
        rssiCache[targetIndex].valid = true;
    }
}

// Function to get device RSSI from memory cache
bool getDeviceRSSI(esp_bd_addr_t address, int8_t* rssi) {
    for (int i = 0; i < RSSI_CACHE_SIZE; i++) {
        if (rssiCache[i].valid && 
            memcmp(rssiCache[i].address, address, sizeof(esp_bd_addr_t)) == 0) {
            
            // Check if data is recent (within last 30 seconds)
            if (millis() - rssiCache[i].lastUpdate < 30000) {
                *rssi = rssiCache[i].rssi;
                return true;
            } else {
                rssiCache[i].valid = false;
                break;
            }
        }
    }
    return false;
}

// Function to calculate approximate distance from RSSI
float calculateDistance(int8_t rssi) {
    if (rssi == 0 || rssi >= -10) {
        return -1.0;
    }
    
    if (rssi >= -50) return 1.0;       // Very close (< 1m)
    else if (rssi >= -65) return 2.0;  // Close (1-3m)
    else if (rssi >= -68) return 4.0;  // Medium (3-4m) - Auth threshold
    else if (rssi >= -73) return 5.0;  // Far (4-5m) - Deauth threshold
    else if (rssi >= -80) return 8.0;  // Very far (5-8m)
    else return 12.0;                  // Extremely far (8m+)
}

// BLE Server Callback Class
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        Serial.println("BLE: Device attempting to connect...");
        
        // If not in pairing mode, check will be done in GAP event handler
        if (!isPairingMode) {
            Serial.println("BLE: Not in pairing mode - will verify during authentication");
        } else {
            Serial.println("BLE: In pairing mode - accepting connection from any device");
        }
        
        // Accept connection initially, but don't set hasConnectedDevice until authentication
        Serial.println("BLE: Device connected - waiting for authentication");
        isBleConnected = true;
        // hasConnectedDevice will be set in GAP event handler after successful authentication
        
        // Visual feedback
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
    }
    
    void onDisconnect(BLEServer* pServer) {
        Serial.println("BLE: Device disconnected");
        
        hasConnectedDevice = false;
        isBleConnected = false;
        bluetoothAuthenticated = false;
        
        // Clear connected device address
        memset(connectedDeviceAddr, 0, sizeof(connectedDeviceAddr));
        
        invalidateDeviceCache();
        
        // Restart advertising after disconnect
        Serial.println("BLE: Restarting advertising after disconnect...");
        startBLEAdvertising(isPairingMode);
    }
};

// Function to initialize Bluetooth
void initializeBluetooth() {
    Serial.println("=== INITIALIZING BLUETOOTH ===");
    
    // Initialize BLE
    Serial.println("BLE: Initializing BLE Device...");
    BLEDevice::init("Ghost-Key Secure");
    Serial.println("BLE: Device initialized with name: Ghost-Key Secure");
    
    // Set BLE power to maximum for better range
    Serial.println("BLE: Setting power levels...");
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
    Serial.println("BLE: Power levels set to maximum");
    
    // Create BLE Server
    Serial.println("BLE: Creating BLE Server...");
    pServer = BLEDevice::createServer();
    
    // Set server callbacks
    pServer->setCallbacks(new MyServerCallbacks());
    Serial.println("BLE: Server callbacks set");
    
    // Set up security with bonding
    Serial.println("BLE: Setting up security...");
    BLESecurity* pSecurity = new BLESecurity();
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
    pSecurity->setCapability(ESP_IO_CAP_NONE);
    pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
    Serial.println("BLE: Security configured with bonding");
    
    // Start keyboard (this handles its own advertising)
    Serial.println("BLE: Starting BLE Keyboard...");
    bleKeyboard.begin();
    Serial.println("BLE: Keyboard started");
    
    // Register GAP callback for events
    esp_ble_gap_register_callback(onGapEvent);
    Serial.println("BLE: GAP callback registered");
    
    // BLE advertising will be started separately
    Serial.println("BLE: Ready to start advertising");
    
    // Print bonded devices count
    int bondedCount = esp_ble_get_bond_device_num();
    Serial.printf("BLE: Found %d bonded devices\n", bondedCount);
    
    // Start advertising in normal mode initially
    startBLEAdvertising(false);
    
    Serial.println("=== BLUETOOTH INITIALIZATION COMPLETE ===");
}

// Function to start BLE advertising with proper discoverability
void startBLEAdvertising(bool discoverable) {
    if(pServer == nullptr) {
        Serial.println("BLE: ERROR - Server not initialized");
        return;
    }
    
    // Check if we can accept new bonds
    int currentBonds = esp_ble_get_bond_device_num();
    if (discoverable && currentBonds >= MAX_BONDS) {
        Serial.println("BLE: Cannot enter pairing mode - maximum bonds reached!");
        Serial.println("BLE: Please remove a device first.");
        isPairingMode = false;
        return;
    }
    
    BLEAdvertising* pAdvertising = pServer->getAdvertising();
    pAdvertising->stop();
    delay(100); // Give time for stop to complete
    
    if(discoverable) {
        Serial.println("BLE: Setting up advertising for PAIRING MODE:");
        Serial.println("BLE: - Fast advertising for discovery");
        Serial.println("BLE: - General discoverable mode");
        Serial.println("BLE: - Interval: 20ms to 40ms");
        Serial.println("BLE: - Allowing ALL devices to connect");
        
        // Set up advertising for pairing mode - accept all devices
        pAdvertising->setAdvertisementType(esp_ble_adv_type_t::ADV_TYPE_IND);
        pAdvertising->setScanResponse(true);
        pAdvertising->setAppearance(0x03C1);  // Keyboard appearance
        
        // Create advertising data with proper flags for discoverability
        BLEAdvertisementData adv_data;
        adv_data.setFlags(0x06);  // General discoverable + BR/EDR not supported
        adv_data.setCompleteServices(BLEUUID((uint16_t)0x1812)); // HID Service
        adv_data.setName("Ghost-Key Secure");
        pAdvertising->setAdvertisementData(adv_data);
        
        // Create scan response data
        BLEAdvertisementData scan_data;
        scan_data.setManufacturerData("Ghost-Key Inc.");
        scan_data.setName("Ghost-Key Secure");
        pAdvertising->setScanResponseData(scan_data);
        
        // Fast advertising for pairing
        pAdvertising->setMinInterval(0x20);  // 20ms
        pAdvertising->setMaxInterval(0x40);  // 40ms
        
    } else {
        Serial.println("BLE: Setting up advertising for NORMAL MODE:");
        Serial.println("BLE: - Slow advertising to save power");
        Serial.println("BLE: - Limited discoverable mode");
        Serial.println("BLE: - Interval: 160ms to 320ms");
        Serial.println("BLE: - Only bonded devices allowed");
        
        // Set up advertising for normal mode
        pAdvertising->setAdvertisementType(esp_ble_adv_type_t::ADV_TYPE_IND);
        pAdvertising->setScanResponse(true);
        pAdvertising->setAppearance(0x03C1);  // Keyboard appearance
        
        // Create advertising data with limited discoverability
        BLEAdvertisementData adv_data;
        adv_data.setFlags(0x05);  // Limited discoverable + BR/EDR not supported
        adv_data.setCompleteServices(BLEUUID((uint16_t)0x1812)); // HID Service
        adv_data.setName("Ghost-Key Secure");
        pAdvertising->setAdvertisementData(adv_data);
        
        // Slower advertising for normal operation
        pAdvertising->setMinInterval(0x100); // 160ms
        pAdvertising->setMaxInterval(0x200); // 320ms
    }
    
    pAdvertising->start();
    Serial.println("BLE: Advertising started successfully - Device should be discoverable");
    if (discoverable) {
        Serial.println("BLE: *** DEVICE IS NOW VISIBLE ON IPHONE ***");
        Serial.println("BLE: Look for 'Ghost-Key Secure' in Bluetooth settings");
    }
}

// Function to stop BLE advertising  
void stopBLEAdvertising() {
    Serial.println("BLE: Stopping advertising...");
    
    if(pServer == nullptr) return;
    
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->stop();
    Serial.println("BLE: Advertising stopped successfully");
}

// Enhanced GAP event handler for debugging
void onGapEvent(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        case ESP_GAP_BLE_SCAN_RESULT_EVT:
            if (param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) {
                // Update RSSI for bonded devices (silent update)
                int bondedCount = esp_ble_get_bond_device_num();
                if (bondedCount > 0) {
                    esp_ble_bond_dev_t* bondedDevices = getBondedDevicesSafe(&bondedCount);
                    if (bondedDevices) {
                        for (int i = 0; i < bondedCount; i++) {
                            if (memcmp(bondedDevices[i].bd_addr, param->scan_rst.bda, sizeof(esp_bd_addr_t)) == 0) {
                                saveDeviceRSSI(param->scan_rst.bda, param->scan_rst.rssi);
                                // Only log significant RSSI changes (>10 dBm difference)
                                static int8_t lastLoggedRSSI = -99;
                                if (abs(param->scan_rst.rssi - lastLoggedRSSI) > 10) {
                                    Serial.printf("BLE: RSSI update: %d dBm\n", param->scan_rst.rssi);
                                    lastLoggedRSSI = param->scan_rst.rssi;
                                }
                                break;
                            }
                        }
                        releaseBondedDevicesBuffer(bondedDevices);
                    }
                }
            }
            break;

        case ESP_GAP_BLE_AUTH_CMPL_EVT:
            if (param->ble_security.auth_cmpl.success) {
                Serial.println("BLE: Authentication successful!");
                
                // Get the authenticated device address
                esp_bd_addr_t addr;
                memcpy(addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
                memcpy(connectedDeviceAddr, addr, sizeof(esp_bd_addr_t));
                
                Serial.printf("BLE: Authenticated device: %02x:%02x:%02x:%02x:%02x:%02x\n",
                    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
                
                hasConnectedDevice = true;
                
                // Visual feedback for successful authentication
                for(int i = 0; i < 3; i++) {
                    digitalWrite(LED_PIN, HIGH);
                    delay(200);
                    digitalWrite(LED_PIN, LOW);
                    delay(200);
                }
                
            } else {
                Serial.printf("BLE: Authentication failed, error: %d\n", param->ble_security.auth_cmpl.fail_reason);
                if (pServer != nullptr) {
                    pServer->disconnect(pServer->getConnId());
                }
            }
            break;
            
        case ESP_GAP_BLE_SEC_REQ_EVT:
            {
                esp_bd_addr_t addr;
                memcpy(addr, param->ble_security.ble_req.bd_addr, sizeof(esp_bd_addr_t));
                
                Serial.printf("BLE: Security request from: %02x:%02x:%02x:%02x:%02x:%02x\n",
                    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
                
                if (isPairingMode) {
                    // In pairing mode - accept any device (up to bond limit)
                    Serial.println("BLE: In pairing mode - accepting security request from any device");
                    esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
                } else {
                    // NOT in pairing mode - only allow whitelisted (bonded) devices
                    bool isDeviceBonded = false;
                    int bondedCount = esp_ble_get_bond_device_num();
                    if (bondedCount > 0) {
                        esp_ble_bond_dev_t* bondedDevices = getBondedDevicesSafe(&bondedCount);
                        if (bondedDevices) {
                            for (int i = 0; i < bondedCount; i++) {
                                if (memcmp(bondedDevices[i].bd_addr, addr, sizeof(esp_bd_addr_t)) == 0) {
                                    isDeviceBonded = true;
                                    break;
                                }
                            }
                            releaseBondedDevicesBuffer(bondedDevices);
                        }
                    }
                    
                    if (isDeviceBonded) {
                        Serial.println("BLE: Device is whitelisted - accepting security request");
                        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
                    } else {
                        Serial.println("BLE: Device NOT whitelisted - rejecting security request");
                        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, false);
                        // Disconnect the device immediately
                        if (pServer != nullptr) {
                            pServer->disconnect(pServer->getConnId());
                        }
                    }
                }
            }
            break;

        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                Serial.println("BLE: Advertising started successfully");
            } else {
                Serial.printf("BLE: Advertising start failed: %d\n", param->adv_start_cmpl.status);
            }
            break;

        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                Serial.println("BLE: Advertising stopped successfully");
            } else {
                Serial.printf("BLE: Advertising stop failed: %d\n", param->adv_stop_cmpl.status);
            }
            break;

        default:
            Serial.printf("BLE: GAP event: %d\n", event);
            break;
    }
}

// Function to toggle pairing mode with proper advertising control
void toggleBluetoothPairingMode() {
    isPairingMode = !isPairingMode;
    
    Serial.println("=== BLUETOOTH PAIRING MODE TOGGLE ===");
    Serial.printf("Pairing mode is now: %s\n", isPairingMode ? "ACTIVE" : "INACTIVE");
    
    if (isPairingMode) {
        pairingModeStartTime = millis();  // Record when pairing mode started
        Serial.println("PAIRING: Entering pairing mode...");
        Serial.println("PAIRING: Making device discoverable...");
        Serial.printf("PAIRING: Auto-timeout in %d seconds\n", PAIRING_MODE_TIMEOUT / 1000);
        
        // Restart advertising to make device discoverable
        stopBLEAdvertising();
        delay(100);
        startBLEAdvertising(true);
        
        Serial.println("PAIRING: *** DEVICE SHOULD NOW BE VISIBLE ON IPHONE ***");
        Serial.println("PAIRING: Look for 'Ghost-Key Secure' in Bluetooth settings");
        Serial.println("PAIRING: Device name: Ghost-Key Secure");
        Serial.println("PAIRING: Manufacturer: Ghost-Key Inc.");
        
    } else {
        pairingModeStartTime = 0;  // Clear the start time
        Serial.println("PAIRING: Exiting pairing mode...");
        Serial.println("PAIRING: Switching to bonded devices only mode");
        
        // Switch to normal advertising mode
        stopBLEAdvertising();
        delay(100);
        startBLEAdvertising(false);
    }
    
    Serial.println("=== PAIRING MODE TOGGLE COMPLETE ===");
}

// Function to disable pairing mode (for timeout and safety)
void disablePairingMode() {
    if (isPairingMode) {
        Serial.println("PAIRING: Disabling pairing mode (timeout/safety)");
        isPairingMode = false;
        pairingModeStartTime = 0;
        
        // Switch to normal advertising mode
        stopBLEAdvertising();
        delay(100);
        startBLEAdvertising(false);
    }
}

// Function to start RSSI scanning
void startRSSIScan() {
    esp_ble_scan_params_t scan_params = {
        .scan_type = BLE_SCAN_TYPE_ACTIVE,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval = 0x50,  // Reduced interval for more frequent updates
        .scan_window = 0x30,    // Increased window for better reception
        .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
    };
    esp_ble_gap_set_scan_params(&scan_params);
    esp_ble_gap_start_scanning(0);
    // Reduced logging - only log once at startup
    static bool firstScan = true;
    if (firstScan) {
        Serial.println("BLE: RSSI scanning started");
        firstScan = false;
    }
}

// Function to stop RSSI scanning
void stopRSSIScan() {
    esp_ble_gap_stop_scanning();
    // Reduced logging - comment out the stop message
    // Serial.println("BLE: RSSI scanning stopped");
}

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
void updateSecurityState();

void setup() {
    Serial.begin(115200);
    DEBUG_PRINTLN("\n\n=== GhostKey System Starting ===");
    DEBUG_PRINTLN("Initializing system...");
    
    setupPins();
    DEBUG_PRINTLN("Pins initialized");
    
    // Set RFID as authenticated for testing
    rfidState.isAuthenticated = false;
    DEBUG_PRINTLN("RFID authentication enabled for testing");
    
    //load saved settings
    preferences.begin("ghostkey", false);
    isConfigured = preferences.getBool("configured", false);
    isBluetoothPaired = preferences.getBool("bluetooth_paired", false);
    starterPulseTime = preferences.getULong("starter_pulse", STARTER_PULSE_TIME);
    autoLockTimeout = preferences.getULong("auto_lock_timeout", AUTO_LOCK_TIMEOUT);
    DEBUG_PRINT("System configured: ");
    DEBUG_PRINTLN(isConfigured ? "Yes" : "No");
    DEBUG_PRINT("Bluetooth paired: ");
    DEBUG_PRINTLN(isBluetoothPaired ? "Yes" : "No");
    DEBUG_PRINT("Starter pulse time: ");
    DEBUG_PRINT(starterPulseTime);
    DEBUG_PRINTLN("ms");
    DEBUG_PRINT("Auto-lock timeout: ");
    DEBUG_PRINT(autoLockTimeout);
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
    
    // Initialize NVS for bluetooth
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        Serial.printf("Error initializing NVS: %d\n", err);
    } else {
        Serial.println("NVS initialized for Bluetooth");
    }
    
    // Initialize BLE cache arrays
    for (int i = 0; i < RSSI_CACHE_SIZE; i++) {
        rssiCache[i].valid = false;
    }
    for (int i = 0; i < NAME_CACHE_SIZE; i++) {
        nameCache[i].valid = false;
    }
    for (int i = 0; i < PRIORITY_CACHE_SIZE; i++) {
        priorityCache[i].valid = false;
    }
    
    // Initialize BLE
    initializeBluetooth();
    DEBUG_PRINTLN("Bluetooth initialized");
    
    // Start RSSI scanning for bonded devices
    startRSSIScan();
    DEBUG_PRINTLN("RSSI scanning started");
    
    DEBUG_PRINTLN("=== System Initialization Complete ===\n");
}

void loop() {
    // Update system state
    updateSystemState();
    
    // Handle RFID
    handleRFID();
    
    // Update bluetooth authentication
    updateBluetoothAuthentication();
    
    // Handle button presses
    handleButtonPress();
    
    // Test button removed - no longer needed
    
    // Update relay states
    controlRelays();
    
    // Check auto-lock
    checkAutoLock();
    
    // Update security state periodically
    if (millis() - lastSecurityCheck >= SECURITY_CHECK_INTERVAL) {
        updateSecurityState();
        lastSecurityCheck = millis();
    }
    
    // Update BLE connection status
    isBleConnected = bleKeyboard.isConnected();
    
    // Start RSSI scanning periodically to track device proximity
    static unsigned long lastRSSIScan = 0;
    if (millis() - lastRSSIScan >= RSSI_UPDATE_INTERVAL) {
        startRSSIScan();
        lastRSSIScan = millis();
    }
    
    // Check pairing mode timeout
    if (isPairingMode && pairingModeStartTime > 0) {
        if (millis() - pairingModeStartTime >= PAIRING_MODE_TIMEOUT) {
            Serial.println("PAIRING: Timeout reached - disabling pairing mode");
            disablePairingMode();
        }
    }
    
    // Handle web server requests when in config mode
    if (currentState == CONFIG_MODE) {
        server.handleClient();
    }
    
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
    // Test button pin removed - no longer needed
    
    //setup output pins
    pinMode(LED_PIN, OUTPUT);
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
                digitalWrite(LED_PIN, HIGH);
                delay(100);
                digitalWrite(LED_PIN, LOW);
            } else {
                DEBUG_RFID_PRINTLN("Failed to add new key");
            }
        } else {
            rfidState.isAuthenticated = isTagAuthorized(tagId);
            DEBUG_RFID_PRINT("Tag authentication: ");
            DEBUG_RFID_PRINTLN(rfidState.isAuthenticated ? "Success" : "Failed");
            
            if (rfidState.isAuthenticated) {
                digitalWrite(LED_PIN, HIGH);
                delay(100);
                digitalWrite(LED_PIN, LOW);
            } else {
                for (int i = 0; i < 3; i++) {
                    digitalWrite(LED_PIN, HIGH);
                    delay(50);
                    digitalWrite(LED_PIN, LOW);
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

    // Check for long press without brake - config mode can be accessed regardless of security state
    if (buttonPressed && !brakeHeld && !isLongPressDetected) {
        unsigned long pressDuration = millis() - buttonPressStartTime;
        if (pressDuration >= CONFIG_MODE_PRESS_TIME) {
            isLongPressDetected = true;
            DEBUG_BUTTON_PRINTLN("Long press detected - Entering config mode (security override)");
            enterConfigMode();
        }
    }

    // Check for start button press in config mode with pairing active
    if (currentState == CONFIG_MODE && isPairingMode && 
        buttonReading == LOW && lastButtonReading == HIGH && 
        (millis() - lastButtonPress) > DEBOUNCE_DELAY) {
        DEBUG_BUTTON_PRINTLN("Start button pressed in pairing mode - Exiting config mode");
        exitConfigMode();
        lastButtonPress = millis();
    }
    
    // Only process normal button operations if not in config mode
    if (currentState != CONFIG_MODE) {
        // Check for button press while brake is held
        if (buttonReading == LOW && brakeHeld && 
            (millis() - lastButtonPress) > DEBOUNCE_DELAY) {
            lastButtonPress = millis();
            
            // Check if authenticated by either RFID OR Bluetooth
            if (rfidState.isAuthenticated || bluetoothAuthenticated) {
                if (engineRunning) {
                    // If engine is running, only allow turning off
                    DEBUG_PRINTLN("Engine sequence stopped");
                    engineRunning = false;
                    systemState = 0;  // Set to OFF
                    startRelayActive = false;
                    lastEngineShutdown = millis();  // Record time of engine shutdown
                    controlRelays();
                } else if (!startRelayActive) {
                    // Only allow starting sequence if engine is not running and not already starting
                    DEBUG_PRINTLN("Starting engine sequence...");
                    startRelayActive = true;
                    startRelayTimer = millis();
                    controlRelays();
                }
            } else {
                DEBUG_BUTTON_PRINTLN("Not authenticated (RFID or Bluetooth) - Access denied");
                // Error feedback
                for (int i = 0; i < 3; i++) {
                    digitalWrite(LED_PIN, HIGH);
                    delay(50);
                    digitalWrite(LED_PIN, LOW);
                    delay(50);
                }
            }
        }

        // Check for button press without brake
        if (buttonReading == HIGH && lastButtonReading == LOW && 
            brakeReading == HIGH && (millis() - lastButtonPress) > DEBOUNCE_DELAY) {
            // Only process button release if we're not in shutdown delay
            if (!isShuttingDown && !engineRunning && !startRelayActive) {
                // Check if authenticated by either RFID OR Bluetooth
                if (rfidState.isAuthenticated || bluetoothAuthenticated) {  // Only allow normal sequence if engine isn't running
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
    
    // Disable pairing mode when exiting config
    disablePairingMode();
    
    // Stop WiFi and web server
    if (wifiEnabled) {
        WiFi.softAPdisconnect(true);
        wifiEnabled = false;
    }
    // Note: WebServer doesn't have an end() method, server stops when WiFi stops
    
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

// Function to get device info as JSON with caching
String getDevicesJson() {
    int bondedCount = esp_ble_get_bond_device_num();
    
    // Check if we can use cached JSON
    if (jsonCacheValid && 
        lastDeviceCount == bondedCount && 
        (millis() - lastJsonUpdate < 5000)) {  // Cache valid for 5 seconds
        return cachedDevicesJson;
    }
    
    String json = "[";
    
    if (bondedCount == 0) {
        json += "]";
        cachedDevicesJson = json;
        jsonCacheValid = true;
        lastDeviceCount = bondedCount;
        lastJsonUpdate = millis();
        return json;
    }
    
    esp_ble_bond_dev_t* bondedDevices = getBondedDevicesSafe(&bondedCount);
    if (!bondedDevices) {
        json += "]";
        return json;
    }
    
    for (int i = 0; i < bondedCount; i++) {
        char mac[18];
        snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
                bondedDevices[i].bd_addr[0], bondedDevices[i].bd_addr[1], bondedDevices[i].bd_addr[2],
                bondedDevices[i].bd_addr[3], bondedDevices[i].bd_addr[4], bondedDevices[i].bd_addr[5]);
        
        char name[32] = "Unknown Device";
        getDeviceName(bondedDevices[i].bd_addr, name, sizeof(name));
        
        bool hasRssi = false;
        int8_t rssi = -99;
        float distance = -1;
        
        if (getDeviceRSSI(bondedDevices[i].bd_addr, &rssi)) {
            hasRssi = true;
            distance = calculateDistance(rssi);
        }
        
        // Note: Priority functionality would be added here if needed
        bool priority = false; // Placeholder for now
        
        if (i > 0) json += ",";
        json += "{";
        json += "\"mac\":\"" + String(mac) + "\",";
        json += "\"name\":\"" + String(name) + "\",";
        json += "\"priority\":" + String(priority ? "true" : "false") + ",";
        json += "\"rssi\":" + String(hasRssi ? rssi : -99) + ",";
        json += "\"distance\":" + String(distance >= 0 ? distance : 0.0, 1);
        json += "}";
    }
    json += "]";
    
    releaseBondedDevicesBuffer(bondedDevices);
    
    // Cache the result
    cachedDevicesJson = json;
    jsonCacheValid = true;
    lastDeviceCount = bondedCount;
    lastJsonUpdate = millis();
    
    return json;
}

void setupWebServer() {
    Serial.println("Starting Web Server...");

    // Handle root path - serve the PROGMEM HTML
    server.on("/", HTTP_GET, [](){
        Serial.println("Serving main configuration page");
        server.send_P(200, "text/html", config_html);
    });
    
    // System status endpoint
    server.on("/status", HTTP_GET, [](){
        String stateStr;
        switch(currentState) {
            case OFF: stateStr = "OFF"; break;
            case ACCESSORY: stateStr = "ACCESSORY"; break;
            case IGNITION: stateStr = "IGNITION"; break;
            case RUNNING: stateStr = "RUNNING"; break;
            case CONFIG_MODE: stateStr = "CONFIG MODE"; break;
            default: stateStr = "UNKNOWN"; break;
        }
        
        String json = "{";
        json += "\"state\":\"" + stateStr + "\",";
        json += "\"rfid\":" + String(rfidState.isAuthenticated ? "true" : "false") + ",";
        json += "\"bluetooth\":" + String(bluetoothAuthenticated ? "true" : "false") + ",";
        json += "\"keys\":" + String(numStoredKeys) + ",";
        json += "\"starterPulse\":" + String(starterPulseTime) + ",";
        json += "\"autoLockTimeout\":" + String(autoLockTimeout);
        json += "}";
        
        server.send(200, "application/json", json);
    });
    
    // Bluetooth devices endpoint
    server.on("/devices", HTTP_GET, [](){
        Serial.println("Bluetooth devices request received");
        String json = getDevicesJson();
        server.send(200, "application/json", json);
    });
    
    // Bluetooth pairing toggle endpoint
    server.on("/pairing", HTTP_GET, [](){
        Serial.println("Bluetooth pairing toggle request received");
        toggleBluetoothPairingMode();
        String status = isPairingMode ? "Pairing mode active" : "Pairing mode inactive";
        server.send(200, "text/plain", status);
    });
    
    // Set device name endpoint
    server.on("/setname", HTTP_POST, [](){
        Serial.println("Set device name request received");
        if (server.hasArg("plain")) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, server.arg("plain"));
            
            if (!error) {
                const char* macStr = doc["mac"];
                const char* name = doc["name"];
                
                if (macStr && name) {
                    esp_bd_addr_t address;
                    int parsed = sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                        &address[0], &address[1], &address[2],
                        &address[3], &address[4], &address[5]);
                    
                    if (parsed == 6) {
                        saveDeviceName(address, name);
                        invalidateDeviceCache();
                        server.send(200, "text/plain", "Device name updated");
                        return;
                    }
                }
            }
        }
        server.send(400, "text/plain", "Invalid request");
    });
    
    // Remove device endpoint
    server.on("/remove", HTTP_POST, [](){
        Serial.println("Remove device request received");
        if (server.hasArg("plain")) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, server.arg("plain"));
            
            if (!error) {
                const char* macStr = doc["mac"];
                
                if (macStr) {
                    esp_bd_addr_t address;
                    int parsed = sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                        &address[0], &address[1], &address[2],
                        &address[3], &address[4], &address[5]);
                    
                    if (parsed == 6) {
                        esp_err_t err = esp_ble_remove_bond_device(address);
                        if (err == ESP_OK) {
                            invalidateDeviceCache();
                            server.send(200, "text/plain", "Device removed");
                        } else {
                            server.send(500, "text/plain", "Failed to remove device");
                        }
                        return;
                    }
                }
            }
        }
        server.send(400, "text/plain", "Invalid request");
    });
    
    // Priority toggle endpoint (placeholder for now)
    server.on("/priority", HTTP_POST, [](){
        Serial.println("Priority toggle request received");
        server.send(200, "text/plain", "Priority feature not implemented yet");
    });

    // Route for updating starter pulse time
    server.on("/update_pulse", HTTP_POST, [](){
        if (server.hasArg("pulse_time")) {
            String pulseTimeStr = server.arg("pulse_time");
            unsigned long newPulseTime = pulseTimeStr.toInt();
            
            // Validate the input
            if (newPulseTime >= 100 && newPulseTime <= 3000) {
                starterPulseTime = newPulseTime;
                preferences.putULong("starter_pulse", starterPulseTime);
                DEBUG_PRINT("Starter pulse time updated to: ");
                DEBUG_PRINT(starterPulseTime);
                DEBUG_PRINTLN("ms");
                server.send(200, "text/plain", "Starter crank time updated successfully");
            } else {
                server.send(400, "text/plain", "Invalid crank time. Must be between 100ms and 3000ms");
            }
        } else {
            server.send(400, "text/plain", "Missing pulse_time parameter");
        }
    });

    // Route for updating auto-lock timeout
    server.on("/update_autolock", HTTP_POST, [](){
        if (server.hasArg("auto_lock")) {
            String timeoutStr = server.arg("auto_lock");
            unsigned long newTimeout = timeoutStr.toInt();
            
            // Validate the input
            if (newTimeout >= 5000 && newTimeout <= 120000) {
                autoLockTimeout = newTimeout;
                preferences.putULong("auto_lock_timeout", autoLockTimeout);
                DEBUG_PRINT("Auto-lock timeout updated to: ");
                DEBUG_PRINT(autoLockTimeout);
                DEBUG_PRINTLN("ms");
                server.send(200, "text/plain", "Auto-lock timeout updated successfully");
            } else {
                server.send(400, "text/plain", "Invalid timeout. Must be between 5000ms and 120000ms");
            }
        } else {
            server.send(400, "text/plain", "Missing auto_lock parameter");
        }
    });

    // Route for exiting config mode
    server.on("/exit", HTTP_POST, [](){
        exitConfigMode();
        server.send(200, "text/plain", "Exiting config mode...");
    });

    // Start server
    server.begin();
    Serial.println("Web server started successfully");
    Serial.print("Server listening on IP: ");
    Serial.println(WiFi.softAPIP());
}

void setupBluetooth() {
    // Bluetooth initialization is now handled in initializeBluetooth()
    // This function kept for compatibility but does nothing
}

// Function to update bluetooth authentication status
void updateBluetoothAuthentication() {
    bool wasAuthenticated = bluetoothAuthenticated;
    
    // Bluetooth authentication requires:
    // 1. Active BLE connection (isBleConnected = true)
    // 2. Device has completed authentication process (hasConnectedDevice = true)
    // 3. Device address is known (not all zeros)
    // 4. Device is within RSSI range (close enough for authentication)
    
    bluetoothAuthenticated = false;  // Start with false
    
    if (isBleConnected && hasConnectedDevice) {
        // Check if we have a valid connected device address
        bool hasValidAddress = false;
        for (int i = 0; i < 6; i++) {
            if (connectedDeviceAddr[i] != 0) {
                hasValidAddress = true;
                break;
            }
        }
        
        if (hasValidAddress) {
            // Check if device is within authentication range using hysteresis
            int8_t rssi = 0;
            if (getDeviceRSSI(connectedDeviceAddr, &rssi)) {
                // Use hysteresis to prevent rapid switching:
                // - Authenticate when RSSI >= -75 dBm (~5 feet)
                // - Deauthenticate when RSSI < -80 dBm (~7+ feet)
                if (!bluetoothAuthenticated && rssi >= RSSI_AUTH_THRESHOLD) {
                    // Not authenticated but signal is strong enough - authenticate
                    bluetoothAuthenticated = true;
                } else if (bluetoothAuthenticated && rssi < RSSI_DEAUTH_THRESHOLD) {
                    // Currently authenticated but signal is too weak - deauthenticate
                    bluetoothAuthenticated = false;
                }
                // If between -80 and -75 dBm, maintain current state (hysteresis)
            } else {
                // No recent RSSI data - assume device is not in range
                bluetoothAuthenticated = false;
            }
        }
    }
    
    // Log authentication status changes
    if (wasAuthenticated != bluetoothAuthenticated) {
        Serial.printf("Bluetooth authentication: %s\n", bluetoothAuthenticated ? "Authenticated" : "Not authenticated");
        if (bluetoothAuthenticated) {
            int8_t rssi = 0;
            if (getDeviceRSSI(connectedDeviceAddr, &rssi)) {
                Serial.printf("Authenticated device: %02x:%02x:%02x:%02x:%02x:%02x (RSSI: %d dBm)\n",
                    connectedDeviceAddr[0], connectedDeviceAddr[1], connectedDeviceAddr[2],
                    connectedDeviceAddr[3], connectedDeviceAddr[4], connectedDeviceAddr[5], rssi);
            }
        }
    }
}

void updateSecurityState() {
    // Either RFID OR Bluetooth can provide authentication
    bool isAuthenticated = rfidState.isAuthenticated || bluetoothAuthenticated;
    
    // Security logic:
    // - Security is ENABLED by default when device is on
    // - Security is DISABLED only when RFID is scanned OR Bluetooth is actively connected
    // - Config mode can be accessed regardless of security state
    // - Engine running always disables security
    
    if (engineRunning) {
        // Engine running - always disable security
        securityEnabled = false;
    } else if (isAuthenticated) {
        // RFID scanned or Bluetooth connected - disable security
        securityEnabled = false;
    } else {
        // No authentication - enable security
        if (lastEngineShutdown > 0) {
            // Allow some time after engine shutdown before enabling security
            unsigned long timeSinceShutdown = millis() - lastEngineShutdown;
            if (timeSinceShutdown >= autoLockTimeout) {
                securityEnabled = true;
            }
        } else {
            // System just started or no engine shutdown recorded - enable security immediately
            // since no authentication is present
            securityEnabled = true;
        }
    }
    
    // Update security relays
    if (securityEnabled) {
        // Security active - all relays LOW
        digitalWrite(RELAY_SECURITY_POS, LOW);
        digitalWrite(RELAY_SECURITY_GND, LOW);
        digitalWrite(RELAY_SECURITY_OPEN, LOW);
    } else {
        // Security disabled - all relays HIGH
        digitalWrite(RELAY_SECURITY_POS, HIGH);
        digitalWrite(RELAY_SECURITY_GND, HIGH);
        digitalWrite(RELAY_SECURITY_OPEN, HIGH);
    }
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
    
    Serial.print("\nSecurity State: ");
    Serial.println(securityEnabled ? "ENABLED" : "DISABLED");
    Serial.print("Security Relays - POS: ");
    Serial.print(digitalRead(RELAY_SECURITY_POS) ? "HIGH" : "LOW");
    Serial.print(" GND: ");
    Serial.print(digitalRead(RELAY_SECURITY_GND) ? "HIGH" : "LOW");
    Serial.print(" OPEN: ");
    Serial.println(digitalRead(RELAY_SECURITY_OPEN) ? "HIGH" : "LOW");
    
    Serial.println("========================\n");
} 