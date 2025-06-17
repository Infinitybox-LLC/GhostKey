#include <BleKeyboard.h>
#include <BLEDevice.h>
#include <nvs_flash.h>
#include <esp_gap_ble_api.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "device_manager_html.h"

// Pin Definitions
#define PAIRING_BUTTON_PIN 19
#define CONNECTED_LED_PIN 23
#define AUTH_LED_PIN 18  // New pin for authentication
#define RSSI_AUTH_THRESHOLD -60  // RSSI threshold for authentication

// BLE Configuration
#define MAX_BONDS 3  // Maximum number of devices that can be bonded
#define BOND_STORAGE_NAMESPACE "ble_bonds"
#define DEVICE_NAME_KEY "dev_name_"
#define DEVICE_PRIORITY_KEY "dev_priority_"
#define MAX_RSSI -30  // Strongest signal we expect
#define MIN_RSSI -100 // Weakest signal we expect

// Debug levels for reducing serial output
#define DEBUG_LEVEL_NONE 0
#define DEBUG_LEVEL_BASIC 1
#define DEBUG_LEVEL_VERBOSE 2
#define DEBUG_LEVEL_FULL 3
#define CURRENT_DEBUG_LEVEL DEBUG_LEVEL_BASIC  // Change this to control verbosity

// Optimized logging macros to reduce serial overhead
#define DEBUG_BASIC(x) do { if(CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_BASIC) { Serial.print(x); } } while(0)
#define DEBUG_BASICLN(x) do { if(CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_BASIC) { Serial.println(x); } } while(0)
#define DEBUG_BASICF(...) do { if(CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_BASIC) { Serial.printf(__VA_ARGS__); } } while(0)

#define DEBUG_VERBOSE(x) do { if(CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE) { Serial.print(x); } } while(0)
#define DEBUG_VERBOSELN(x) do { if(CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE) { Serial.println(x); } } while(0)
#define DEBUG_VERBOSEF(...) do { if(CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE) { Serial.printf(__VA_ARGS__); } } while(0)

#define DEBUG_FULL(x) do { if(CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_FULL) { Serial.print(x); } } while(0)
#define DEBUG_FULLLN(x) do { if(CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_FULL) { Serial.println(x); } } while(0)
#define DEBUG_FULLF(...) do { if(CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_FULL) { Serial.printf(__VA_ARGS__); } } while(0)

// Timing definitions
#define PAIRING_MODE_TIMEOUT 60000  // 60 seconds
#define BLINK_INTERVAL 500          // 500ms for pairing mode blink
#define STATUS_PRINT_INTERVAL 5000  // Print status every 5 seconds
#define DEVICE_SELECT_TIMEOUT 10000 // 10 seconds to select device
#define RSSI_UPDATE_INTERVAL 3000 // Update RSSI every 3 seconds (reduced frequency)

// Connection parameters
#define CONNECTION_TIMEOUT 30000  // 30 seconds connection timeout
#define MIN_CONN_INTERVAL 0x10    // 20ms
#define MAX_CONN_INTERVAL 0x20    // 40ms
#define SLAVE_LATENCY 0
#define SUPERVISION_TIMEOUT 0x100 // 1 second

// WiFi credentials
#define WIFI_SSID "GhostKey_Config"
#define WIFI_PASSWORD "12345678"

// Web server
WebServer server(80);

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
    
    // DO NOT erase the entire NVS flash - this destroys BLE bonding data
    Serial.println("=== CUSTOM NVS CLEANUP COMPLETE ===");
    Serial.println("Note: BLE bonding data preserved");
}

// States
bool isPairingMode = false;
bool isSelectingDevice = false;
bool isConnected = false;
bool isAuthenticated = false;  // New state for authentication
esp_bd_addr_t connectedDeviceAddr = {0};  // Store currently connected device address
bool hasConnectedDevice = false;  // Track if we have a device connected
unsigned long pairingModeStartTime = 0;
unsigned long lastBlinkTime = 0;
unsigned long lastStatusPrint = 0;
unsigned long deviceSelectStartTime = 0;
bool ledState = false;
int selectedDeviceIndex = 0;

BleKeyboard bleKeyboard("Ghost-Key Secure", "Ghost-Key Inc.", 100);
BLEServer* pServer = nullptr;

// Structure to store device info
struct DeviceInfo {
    esp_bd_addr_t address;
    char name[32];  // Maximum name length of 31 characters + null terminator
    bool isPriority; // Priority flag
    int8_t rssi;    // Signal strength
};

// In-memory RSSI cache (no NVS storage for RSSI)
struct RSSICache {
    esp_bd_addr_t address;
    int8_t rssi;
    unsigned long lastUpdate;
    bool valid;
};

// In-memory device name cache (only save to NVS when changed)
struct DeviceNameCache {
    esp_bd_addr_t address;
    char name[32];
    bool hasCustomName;  // True if user set a custom name
    bool nvsStored;      // True if stored in NVS
    bool valid;
};

// In-memory device priority cache (only save to NVS when changed)
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

// HTML page is now defined in device_manager_html.h

// Shared buffer for bonded device operations to reduce malloc calls
static esp_ble_bond_dev_t sharedBondedDevicesBuffer[MAX_BONDS];
static bool sharedBufferInUse = false;

// Cache for JSON generation
static String cachedDevicesJson = "";
static unsigned long lastJsonUpdate = 0;
static int lastDeviceCount = -1;
static bool jsonCacheValid = false;

// Helper function to get bonded devices using shared buffer
esp_ble_bond_dev_t* getBondedDevicesSafe(int* count) {
    if (sharedBufferInUse) {
        // If shared buffer is in use, allocate temporary buffer
        // This should rarely happen with proper code structure
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
        // Use shared buffer
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

// Function to find or create name cache entry - simple version without NVS loading
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
    
    // No empty slot, replace oldest entry (simple replacement)
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
    
    // Always save user-set custom names to NVS (this is from web interface)
    bool shouldSaveToNVS = nameCache[cacheIndex].hasCustomName;
    
    if (shouldSaveToNVS) {
        nvs_handle_t handle;
        esp_err_t err = nvs_open(BOND_STORAGE_NAMESPACE, NVS_READWRITE, &handle);
        if (err != ESP_OK) {
            Serial.printf("Error opening NVS handle: %d\n", err);
            return;
        }

        char key[32];
        snprintf(key, sizeof(key), "%s%02x%02x%02x%02x%02x%02x", 
                 DEVICE_NAME_KEY, address[0], address[1], address[2], 
                 address[3], address[4], address[5]);

        err = nvs_set_str(handle, key, name);
        if (err != ESP_OK) {
            Serial.printf("Error saving device name to NVS: %d - ", err);
            if (err == 4361) {  // ESP_ERR_NVS_NOT_ENOUGH_SPACE
                Serial.println("Not enough space, cleaning up NVS and retrying...");
                nvs_close(handle);
                cleanupNVSStorage();
                
                // Reopen and retry
                err = nvs_open(BOND_STORAGE_NAMESPACE, NVS_READWRITE, &handle);
                if (err == ESP_OK) {
                    err = nvs_set_str(handle, key, name);
                    if (err == ESP_OK) {
                        Serial.println("Device name saved to NVS successfully after cleanup!");
                        nameCache[cacheIndex].nvsStored = true;
                    } else {
                        Serial.printf("Still failed to save device name to NVS after cleanup: %d\n", err);
                    }
                } else {
                    Serial.printf("Failed to reopen NVS after cleanup: %d\n", err);
                }
            } else {
                Serial.printf("Other NVS error: %d\n", err);
            }
        } else {
            Serial.printf("Device name saved to NVS successfully: %s\n", name);
            nameCache[cacheIndex].nvsStored = true;
        }
        
        if (err == ESP_OK) {
            err = nvs_commit(handle);
            if (err != ESP_OK) {
                Serial.printf("Error committing device name to NVS: %d\n", err);
            }
        }
        nvs_close(handle);
    } else {
        Serial.printf("Device name saved to NVS successfully: %s\n", name);
    }
}

// Function to save auto-detected name (advertising name) - cache only, no NVS
void saveAutoDetectedDeviceName(esp_bd_addr_t address, const char* name) {
    int cacheIndex = findOrCreateNameCacheEntry(address);
    
    // Only update if we don't already have a custom name
    if (!nameCache[cacheIndex].hasCustomName) {
        strncpy(nameCache[cacheIndex].name, name, sizeof(nameCache[cacheIndex].name) - 1);
        nameCache[cacheIndex].name[sizeof(nameCache[cacheIndex].name) - 1] = '\0';
        nameCache[cacheIndex].hasCustomName = false; // This is auto-detected, not custom
        Serial.printf("Auto-detected device name cached (no NVS): %s\n", name);
    }
}

// Function to get device name - check cache first, then NVS if needed
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
        Serial.printf("getDeviceName: Failed to open NVS namespace: %d\n", err);
        return false;
    }

    char key[32];
    snprintf(key, sizeof(key), "%s%02x%02x%02x%02x%02x%02x", 
             DEVICE_NAME_KEY, address[0], address[1], address[2], 
             address[3], address[4], address[5]);

    Serial.printf("getDeviceName: Looking for key: %s\n", key);

    size_t required_size;
    err = nvs_get_str(handle, key, NULL, &required_size);
    if (err != ESP_OK) {
        Serial.printf("getDeviceName: Key not found or error: %d\n", err);
        nvs_close(handle);
        return false;
    }

    if (required_size > max_len) {
        Serial.printf("getDeviceName: Name too long (%d > %d)\n", required_size, max_len);
        nvs_close(handle);
        return false;
    }

    err = nvs_get_str(handle, key, name, &max_len);
    nvs_close(handle);
    
    if (err == ESP_OK) {
        Serial.printf("getDeviceName: Successfully loaded '%s' from NVS\n", name);
        // Store in cache for future access
        int cacheIndex = findOrCreateNameCacheEntry(address);
        strncpy(nameCache[cacheIndex].name, name, sizeof(nameCache[cacheIndex].name) - 1);
        nameCache[cacheIndex].name[sizeof(nameCache[cacheIndex].name) - 1] = '\0';
        nameCache[cacheIndex].hasCustomName = true;  // From NVS means it was user-set
        nameCache[cacheIndex].nvsStored = true;
        return true;
    } else {
        Serial.printf("getDeviceName: Failed to read string: %d\n", err);
    }
    
    return false;
}

// Function to remove device name from both cache and NVS
void removeDeviceName(esp_bd_addr_t address) {
    // Remove from cache
    for (int i = 0; i < NAME_CACHE_SIZE; i++) {
        if (nameCache[i].valid && 
            memcmp(nameCache[i].address, address, sizeof(esp_bd_addr_t)) == 0) {
            nameCache[i].valid = false;
            break;
        }
    }
    
    // Remove from NVS
    nvs_handle_t handle;
    esp_err_t err = nvs_open(BOND_STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return;
    }

    char key[32];
    snprintf(key, sizeof(key), "%s%02x%02x%02x%02x%02x%02x", 
             DEVICE_NAME_KEY, address[0], address[1], address[2], 
             address[3], address[4], address[5]);

    nvs_erase_key(handle, key);
    nvs_close(handle);
}

// Function to find or create priority cache entry
int findOrCreatePriorityCacheEntry(esp_bd_addr_t address) {
    // First, look for existing entry
    for (int i = 0; i < PRIORITY_CACHE_SIZE; i++) {
        if (priorityCache[i].valid && 
            memcmp(priorityCache[i].address, address, sizeof(esp_bd_addr_t)) == 0) {
            return i;
        }
    }
    
    // Look for empty slot
    for (int i = 0; i < PRIORITY_CACHE_SIZE; i++) {
        if (!priorityCache[i].valid) {
            memcpy(priorityCache[i].address, address, sizeof(esp_bd_addr_t));
            priorityCache[i].isPriority = false;
            priorityCache[i].nvsStored = false;
            priorityCache[i].valid = true;
            return i;
        }
    }
    
    // No empty slot, replace oldest entry (simple replacement)
    memcpy(priorityCache[0].address, address, sizeof(esp_bd_addr_t));
    priorityCache[0].isPriority = false;
    priorityCache[0].nvsStored = false;
    priorityCache[0].valid = true;
    return 0;
}

// Function to clear all device priorities (both cache and NVS)
void clearAllDevicePriorities() {
    // Clear cache

    for (int i = 0; i < PRIORITY_CACHE_SIZE; i++) {
        if (priorityCache[i].valid) {
            priorityCache[i].isPriority = false;
            priorityCache[i].nvsStored = false;
        }
    }
    
    // Clear NVS priorities
    nvs_handle_t handle;
    if (nvs_open(BOND_STORAGE_NAMESPACE, NVS_READWRITE, &handle) == ESP_OK) {
        // Get all bonded devices
        int bondedCount = esp_ble_get_bond_device_num();
        if (bondedCount > 0) {
            esp_ble_bond_dev_t* bondedDevices = (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * bondedCount);
            if (bondedDevices) {
                esp_ble_get_bond_device_list(&bondedCount, bondedDevices);
                for (int i = 0; i < bondedCount; i++) {
                    char key[32];
                    snprintf(key, sizeof(key), "%s%02x%02x%02x%02x%02x%02x", 
                            DEVICE_PRIORITY_KEY,
                            bondedDevices[i].bd_addr[0], bondedDevices[i].bd_addr[1],
                            bondedDevices[i].bd_addr[2], bondedDevices[i].bd_addr[3],
                            bondedDevices[i].bd_addr[4], bondedDevices[i].bd_addr[5]);
                    nvs_erase_key(handle, key);
                }
                free(bondedDevices);
            }
        }
        nvs_commit(handle);
        nvs_close(handle);
    }
}

// Function to save device priority to cache and NVS
void saveDevicePriority(esp_bd_addr_t address, bool isPriority) {
    // If setting a new priority device, first clear all existing priorities
    if (isPriority) {
        DEBUG_BASICLN("Clearing all existing priority devices");
        clearAllDevicePriorities();
        invalidateDeviceCache();
    }
    
    // Now set the new priority
    int cacheIndex = findOrCreatePriorityCacheEntry(address);
    priorityCache[cacheIndex].isPriority = isPriority;
    
    // Save to NVS
    nvs_handle_t handle;
    esp_err_t err = nvs_open(BOND_STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        char key[32];
        snprintf(key, sizeof(key), "%s%02x%02x%02x%02x%02x%02x", 
                 DEVICE_PRIORITY_KEY,
                 address[0], address[1], address[2], 
                 address[3], address[4], address[5]);
        
        err = nvs_set_u8(handle, key, isPriority ? 1 : 0);
        if (err == ESP_OK) {
            err = nvs_commit(handle);
            if (err == ESP_OK) {
                priorityCache[cacheIndex].nvsStored = true;
                Serial.printf("Device priority %s for %02x:%02x:%02x:%02x:%02x:%02x\n",
                    isPriority ? "set" : "removed",
                    address[0], address[1], address[2],
                    address[3], address[4], address[5]);
            }
        }
        nvs_close(handle);
    }
}

// Function to get device priority - check cache first, then NVS if needed
bool getDevicePriority(esp_bd_addr_t address) {
    // First check cache
    for (int i = 0; i < PRIORITY_CACHE_SIZE; i++) {
        if (priorityCache[i].valid && 
            memcmp(priorityCache[i].address, address, sizeof(esp_bd_addr_t)) == 0) {
            return priorityCache[i].isPriority;
        }
    }
    
    // Not in cache, try to load from NVS
    nvs_handle_t handle;
    esp_err_t err = nvs_open(BOND_STORAGE_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        return false;
    }

    char key[32];
    snprintf(key, sizeof(key), "%s%02x%02x%02x%02x%02x%02x", 
             DEVICE_PRIORITY_KEY, address[0], address[1], address[2], 
             address[3], address[4], address[5]);

    uint8_t priority = 0;
    err = nvs_get_u8(handle, key, &priority);
    nvs_close(handle);
    
    if (err == ESP_OK) {
        // Store in cache for future access
        int cacheIndex = findOrCreatePriorityCacheEntry(address);
        priorityCache[cacheIndex].isPriority = (priority == 1);
        priorityCache[cacheIndex].nvsStored = true;
        return (priority == 1);
    }
    
    return false;
}

// Function to remove device priority from both cache and NVS
void removeDevicePriority(esp_bd_addr_t address) {
    // Remove from cache
    for (int i = 0; i < PRIORITY_CACHE_SIZE; i++) {
        if (priorityCache[i].valid && 
            memcmp(priorityCache[i].address, address, sizeof(esp_bd_addr_t)) == 0) {
            priorityCache[i].valid = false;
            break;
        }
    }
    
    // Remove from NVS
    nvs_handle_t handle;
    esp_err_t err = nvs_open(BOND_STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return;
    }

    char key[32];
    snprintf(key, sizeof(key), "%s%02x%02x%02x%02x%02x%02x", 
             DEVICE_PRIORITY_KEY, address[0], address[1], address[2], 
             address[3], address[4], address[5]);

    nvs_erase_key(handle, key);
    nvs_close(handle);
}

// Function to preload all bonded device names DIRECTLY from NVS into cache
void preloadDeviceNamesFromNVS() {
    Serial.println("=== PRELOADING DEVICE NAMES FROM NVS ===");
    int bondedCount;
    esp_ble_bond_dev_t* bondedDevices = getBondedDevicesSafe(&bondedCount);
    if (!bondedDevices) {
        Serial.println("No bonded devices found for name preloading");
        return;
    }
    
    // Open NVS handle once for all operations
    nvs_handle_t handle;
    esp_err_t err = nvs_open(BOND_STORAGE_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        Serial.printf("Failed to open NVS namespace for preloading: %d\n", err);
        releaseBondedDevicesBuffer(bondedDevices);
        return;
    }
    
    Serial.printf("Attempting to preload names for %d bonded devices:\n", bondedCount);
    
    // First, let's iterate through existing NVS entries to match them with current bonded devices
    nvs_iterator_t it = nvs_entry_find(BOND_STORAGE_NAMESPACE, NULL, NVS_TYPE_STR);
    String foundNames[bondedCount];
    bool nameFound[bondedCount] = {false};
    
    // Scan through all stored names
    while (it != NULL) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);
        
        if (strncmp(info.key, DEVICE_NAME_KEY, strlen(DEVICE_NAME_KEY)) == 0) {
            char storedName[32];
            size_t required_size = sizeof(storedName);
            esp_err_t err = nvs_get_str(handle, info.key, storedName, &required_size);
            
            if (err == ESP_OK) {
                Serial.printf("Found stored name: %s -> %s\n", info.key, storedName);
                
                // Store this name for the first available bonded device that doesn't have a name yet
                for (int i = 0; i < bondedCount; i++) {
                    if (!nameFound[i]) {
                        foundNames[i] = String(storedName);
                        nameFound[i] = true;
                        Serial.printf("  Assigning to device %d: %02x:%02x:%02x:%02x:%02x:%02x\n", i + 1,
                            bondedDevices[i].bd_addr[0], bondedDevices[i].bd_addr[1], bondedDevices[i].bd_addr[2],
                            bondedDevices[i].bd_addr[3], bondedDevices[i].bd_addr[4], bondedDevices[i].bd_addr[5]);
                        break;
                    }
                }
            }
        }
        
        it = nvs_entry_next(it);
    }
    nvs_release_iterator(it);
    
    // Now populate cache with found names
    for (int i = 0; i < bondedCount; i++) {
        Serial.printf("Device %d: %02x:%02x:%02x:%02x:%02x:%02x\n", i + 1,
            bondedDevices[i].bd_addr[0], bondedDevices[i].bd_addr[1], bondedDevices[i].bd_addr[2],
            bondedDevices[i].bd_addr[3], bondedDevices[i].bd_addr[4], bondedDevices[i].bd_addr[5]);
        
        if (nameFound[i]) {
            // Load the found name into cache
            int cacheIndex = findOrCreateNameCacheEntry(bondedDevices[i].bd_addr);
            strncpy(nameCache[cacheIndex].name, foundNames[i].c_str(), sizeof(nameCache[cacheIndex].name) - 1);
            nameCache[cacheIndex].name[sizeof(nameCache[cacheIndex].name) - 1] = '\0';
            nameCache[cacheIndex].hasCustomName = true;
            nameCache[cacheIndex].nvsStored = true;
            nameCache[cacheIndex].valid = true;
            
            Serial.printf("  ✓ Loaded name from NVS: %s\n", foundNames[i].c_str());
            
            // Update NVS with current address for future access
            char currentKey[32];
            snprintf(currentKey, sizeof(currentKey), "%s%02x%02x%02x%02x%02x%02x", 
                     DEVICE_NAME_KEY, 
                     bondedDevices[i].bd_addr[0], bondedDevices[i].bd_addr[1], bondedDevices[i].bd_addr[2], 
                     bondedDevices[i].bd_addr[3], bondedDevices[i].bd_addr[4], bondedDevices[i].bd_addr[5]);
            
            esp_err_t saveErr = nvs_set_str(handle, currentKey, foundNames[i].c_str());
            if (saveErr == ESP_OK) {
                Serial.printf("  ✓ Updated NVS with current address key\n");
            }
        } else {
            Serial.printf("  ✗ No stored name found for this device\n");
        }
    }
    
    nvs_close(handle);
    Serial.println("========================================");
    
    releaseBondedDevicesBuffer(bondedDevices);
}

// Function to preload all bonded device priorities from NVS into cache
void preloadDevicePrioritiesFromNVS() {
    int bondedCount;
    esp_ble_bond_dev_t* bondedDevices = getBondedDevicesSafe(&bondedCount);
    if (!bondedDevices) return;
    
    Serial.println("Preloading device priorities from NVS...");
    
    // Open NVS handle for priority operations
    nvs_handle_t handle;
    esp_err_t err = nvs_open(BOND_STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        Serial.printf("Failed to open NVS for priority preloading: %d\n", err);
        releaseBondedDevicesBuffer(bondedDevices);
        return;
    }
    
    // Scan through all stored priorities and assign to available devices
    nvs_iterator_t it = nvs_entry_find(BOND_STORAGE_NAMESPACE, NULL, NVS_TYPE_U8);
    bool priorityFound[bondedCount] = {false};
    
    while (it != NULL) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);
        
        if (strncmp(info.key, DEVICE_PRIORITY_KEY, strlen(DEVICE_PRIORITY_KEY)) == 0) {
            uint8_t priority = 0;
            esp_err_t priorityErr = nvs_get_u8(handle, info.key, &priority);
            
            if (priorityErr == ESP_OK && priority == 1) {
                // Find first device without priority assigned
                for (int i = 0; i < bondedCount; i++) {
                    if (!priorityFound[i]) {
                        priorityFound[i] = true;
                        
                        // Load into cache
                        int cacheIndex = findOrCreatePriorityCacheEntry(bondedDevices[i].bd_addr);
                        priorityCache[cacheIndex].isPriority = true;
                        priorityCache[cacheIndex].nvsStored = true;
                        
                        Serial.printf("Loaded priority for device %d: %02x:%02x:%02x:%02x:%02x:%02x = HIGH\n", i + 1,
                            bondedDevices[i].bd_addr[0], bondedDevices[i].bd_addr[1], bondedDevices[i].bd_addr[2],
                            bondedDevices[i].bd_addr[3], bondedDevices[i].bd_addr[4], bondedDevices[i].bd_addr[5]);
                        
                        // Update NVS with current address
                        char currentKey[32];
                        snprintf(currentKey, sizeof(currentKey), "%s%02x%02x%02x%02x%02x%02x", 
                                 DEVICE_PRIORITY_KEY,
                                 bondedDevices[i].bd_addr[0], bondedDevices[i].bd_addr[1], bondedDevices[i].bd_addr[2],
                                 bondedDevices[i].bd_addr[3], bondedDevices[i].bd_addr[4], bondedDevices[i].bd_addr[5]);
                        
                        esp_err_t saveErr = nvs_set_u8(handle, currentKey, 1);
                        if (saveErr == ESP_OK) {
                            Serial.printf("  ✓ Updated priority NVS with current address key\n");
                        }
                        break;
                    }
                }
            }
        }
        
        it = nvs_entry_next(it);
    }
    nvs_release_iterator(it);
    
    nvs_close(handle);
    releaseBondedDevicesBuffer(bondedDevices);
}

// Function to save device RSSI in memory cache only (no NVS writes)
void saveDeviceRSSI(esp_bd_addr_t address, int8_t rssi) {
    // Find existing entry or oldest entry to replace
    int targetIndex = -1;
    unsigned long oldestTime = ULONG_MAX;
    
    for (int i = 0; i < RSSI_CACHE_SIZE; i++) {
        if (!rssiCache[i].valid) {
            // Found empty slot
            targetIndex = i;
            break;
        }
        
        if (memcmp(rssiCache[i].address, address, sizeof(esp_bd_addr_t)) == 0) {
            // Found existing entry for this device
            targetIndex = i;
            break;
        }
        
        if (rssiCache[i].lastUpdate < oldestTime) {
            // Track oldest entry for potential replacement
            oldestTime = rssiCache[i].lastUpdate;
            targetIndex = i;
        }
    }
    
    if (targetIndex >= 0) {
        memcpy(rssiCache[targetIndex].address, address, sizeof(esp_bd_addr_t));
        rssiCache[targetIndex].rssi = rssi;
        rssiCache[targetIndex].lastUpdate = millis();
        rssiCache[targetIndex].valid = true;
        
        // Only log occasionally to reduce serial spam
        static unsigned long lastLogTime = 0;
        if (millis() - lastLogTime > 10000) {  // Log every 10 seconds (reduced frequency)
            DEBUG_VERBOSEF("RSSI cache updated: %02x:%02x:%02x:%02x:%02x:%02x = %d dBm\n",
                address[0], address[1], address[2], address[3], address[4], address[5], rssi);
            lastLogTime = millis();
        }
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
                // Data is too old, mark as invalid
                rssiCache[i].valid = false;
                break;
            }
        }
    }
    return false;
}

// Function to calculate approximate distance from RSSI
float calculateDistance(int8_t rssi) {
    if (rssi == 0 || rssi >= -10) {  // Invalid or too strong signal
        return -1.0;
    }
    
    // Simple distance calculation based on RSSI
    // This is approximate and can vary greatly based on environment
    if (rssi >= -50) return 1.0;       // Very close (< 1m)
    else if (rssi >= -60) return 2.0;  // Close (1-2m)
    else if (rssi >= -70) return 5.0;  // Medium (2-5m)
    else if (rssi >= -80) return 10.0; // Far (5-10m)
    else return 15.0;                  // Very far (10m+)
}

// Function to sort devices by priority and proximity
void sortDevicesByPriorityAndProximity(esp_ble_bond_dev_t* devices, int count) {
    // Create array of device info
    DeviceInfo* deviceInfos = (DeviceInfo*)malloc(sizeof(DeviceInfo) * count);
    if (!deviceInfos) return;

    // Fill device info
    for (int i = 0; i < count; i++) {
        memcpy(deviceInfos[i].address, devices[i].bd_addr, sizeof(esp_bd_addr_t));
        getDeviceName(devices[i].bd_addr, deviceInfos[i].name, sizeof(deviceInfos[i].name));
        deviceInfos[i].isPriority = getDevicePriority(devices[i].bd_addr);
        deviceInfos[i].rssi = 0; // Placeholder for RSSI
    }

    // Sort devices (priority first, then by RSSI)
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            bool swap = false;
            
            // First sort by priority
            if (deviceInfos[j].isPriority != deviceInfos[j + 1].isPriority) {
                swap = !deviceInfos[j].isPriority;
            }
            // Then by RSSI
            else if (deviceInfos[j].rssi < deviceInfos[j + 1].rssi) {
                swap = true;
            }

            if (swap) {
                // Swap device info
                DeviceInfo temp = deviceInfos[j];
                deviceInfos[j] = deviceInfos[j + 1];
                deviceInfos[j + 1] = temp;
                
                // Swap bonded devices
                esp_ble_bond_dev_t tempDev = devices[j];
                devices[j] = devices[j + 1];
                devices[j + 1] = tempDev;
            }
        }
    }

    free(deviceInfos);
}

void removeBondedDevice(int index) {
    int bondedCount;
    esp_ble_bond_dev_t* bondedDevices = getBondedDevicesSafe(&bondedCount);
    if(!bondedDevices || index < 0 || index >= bondedCount) {
        if (bondedDevices) releaseBondedDevicesBuffer(bondedDevices);
        return;
    }
        
    // Remove the device name and clear caches before removing the bond
    removeDeviceName(bondedDevices[index].bd_addr);
    
    // Clear RSSI cache entry for this device
    for (int i = 0; i < RSSI_CACHE_SIZE; i++) {
        if (rssiCache[i].valid && 
            memcmp(rssiCache[i].address, bondedDevices[index].bd_addr, sizeof(esp_bd_addr_t)) == 0) {
            rssiCache[i].valid = false;
            break;
        }
    }
    
    // Clear name cache entry for this device
    for (int i = 0; i < NAME_CACHE_SIZE; i++) {
        if (nameCache[i].valid && 
            memcmp(nameCache[i].address, bondedDevices[index].bd_addr, sizeof(esp_bd_addr_t)) == 0) {
            nameCache[i].valid = false;
            break;
        }
    }
    
    // Clear priority cache entry for this device
    for (int i = 0; i < PRIORITY_CACHE_SIZE; i++) {
        if (priorityCache[i].valid && 
            memcmp(priorityCache[i].address, bondedDevices[index].bd_addr, sizeof(esp_bd_addr_t)) == 0) {
            priorityCache[i].valid = false;
            break;
        }
    }
    
    Serial.printf("\nRemoving device %d: %02x:%02x:%02x:%02x:%02x:%02x\n", index + 1,
        bondedDevices[index].bd_addr[0], bondedDevices[index].bd_addr[1],
        bondedDevices[index].bd_addr[2], bondedDevices[index].bd_addr[3],
        bondedDevices[index].bd_addr[4], bondedDevices[index].bd_addr[5]);
        
    // Remove from whitelist before removing bond
    removeFromWhitelist(bondedDevices[index].bd_addr, BLE_WL_ADDR_TYPE_PUBLIC);
    DEBUG_BASICLN("Device removed from whitelist");
        
    // Invalidate caches
    invalidateDeviceCache();
    
    // Stop BLE before removing bond
    if(pServer != nullptr) {
        pServer->getAdvertising()->stop();
    }
    bleKeyboard.end();
    BLEDevice::deinit(true);
    
    // Remove the bond
    esp_ble_remove_bond_device(bondedDevices[index].bd_addr);
    releaseBondedDevicesBuffer(bondedDevices);
    
    // Visual feedback - three quick blinks
    for(int i = 0; i < 3; i++) {
        digitalWrite(CONNECTED_LED_PIN, HIGH);
        delay(100);
        digitalWrite(CONNECTED_LED_PIN, LOW);
        delay(100);
    }
    
    // Restart to reinitialize BLE cleanly
    Serial.println("Device removed. Restarting...");
    delay(500);
    ESP.restart();
}

void addToWhitelist(esp_bd_addr_t address, esp_ble_wl_addr_type_t addr_type) {
    esp_err_t err = esp_ble_gap_update_whitelist(true, address, addr_type);
    if (err == ESP_OK) {
        Serial.println("Device added to whitelist successfully");
    } else {
        Serial.printf("Failed to add device to whitelist, error: %d\n", err);
    }
}

void removeFromWhitelist(esp_bd_addr_t address, esp_ble_wl_addr_type_t addr_type) {
    esp_err_t err = esp_ble_gap_update_whitelist(false, address, addr_type);
    if (err == ESP_OK) {
        Serial.println("Device removed from whitelist successfully");
    } else {
        Serial.printf("Failed to remove device from whitelist, error: %d\n", err);
    }
}

void clearWhitelist() {
    esp_err_t err = esp_ble_gap_clear_whitelist();
    if (err == ESP_OK) {
        Serial.println("Whitelist cleared successfully");
    } else {
        Serial.printf("Failed to clear whitelist, error: %d\n", err);
    }
}

void printBondedDevices() {
    int bondedCount;
    esp_ble_bond_dev_t* bondedDevices = getBondedDevicesSafe(&bondedCount);
    uint16_t whitelistSize = 0;
    esp_ble_gap_get_whitelist_size(&whitelistSize);
    
    Serial.println("\n=== BONDED DEVICES ===");
    Serial.printf("Total bonded devices: %d (Maximum: %d)\n", bondedCount, MAX_BONDS);
    Serial.printf("Whitelist size: %d\n", whitelistSize);
    Serial.printf("Pairing mode: %s\n", isPairingMode ? "ACTIVE (All devices)" : "INACTIVE (Whitelist only)");
    
    if (whitelistSize > 0) {
        DEBUG_VERBOSELN("\n=== WHITELIST STATUS ===");
        DEBUG_VERBOSEF("Whitelist contains %d devices\n", whitelistSize);
        DEBUG_VERBOSELN("When NOT in pairing mode: Only whitelisted devices can connect");
        DEBUG_VERBOSELN("When IN pairing mode: All devices can connect and bond");
        DEBUG_VERBOSELN("=======================\n");
    }
    
    if (bondedDevices && bondedCount > 0) {
        // Sort devices by priority and proximity
        sortDevicesByPriorityAndProximity(bondedDevices, bondedCount);
        
        for (int i = 0; i < bondedCount; i++) {
            char deviceName[32] = "Unknown Device";
            getDeviceName(bondedDevices[i].bd_addr, deviceName, sizeof(deviceName));
            bool isPriorityDevice = getDevicePriority(bondedDevices[i].bd_addr);
            int8_t rssi = 0;
            bool hasRSSI = getDeviceRSSI(bondedDevices[i].bd_addr, &rssi);
            
            if(isSelectingDevice && i == selectedDeviceIndex) {
                Serial.print("-> ");
            } else {
                Serial.print("   ");
            }
            
            Serial.printf("Device %d: %s (MAC: %02x:%02x:%02x:%02x:%02x:%02x)\n", i + 1,
                deviceName,
                bondedDevices[i].bd_addr[0], bondedDevices[i].bd_addr[1], 
                bondedDevices[i].bd_addr[2], bondedDevices[i].bd_addr[3],
                bondedDevices[i].bd_addr[4], bondedDevices[i].bd_addr[5]);
                
            Serial.printf("   Priority: %s\n", isPriorityDevice ? "Yes" : "No");
            
            if (hasRSSI) {
                float distance = calculateDistance(rssi);
                DEBUG_VERBOSEF("   Signal Strength: %d dBm\n", rssi);
                if (distance >= 0) {
                    DEBUG_VERBOSEF("   Approximate Distance: %.1f meters\n", distance);
                } else {
                    DEBUG_VERBOSELN("   Approximate Distance: Unknown");
                }
            } else {
                DEBUG_VERBOSELN("   Signal Strength: No recent data");
                DEBUG_VERBOSELN("   Approximate Distance: Unknown");
            }
        }
        
        releaseBondedDevicesBuffer(bondedDevices);
    } else {
        Serial.println("No bonded devices found.");
    }
    
    if (bondedCount >= MAX_BONDS) {
        Serial.println("WARNING: Maximum number of bonds reached!");
        Serial.println("Remove a device before adding new ones.");
    }
    
    if (isSelectingDevice) {
        Serial.println("\nDevice Selection Mode:");
        Serial.println("- Short press: Select next device");
        Serial.println("- Long press: Remove selected device");
        Serial.println("- Double press: Exit selection mode");
        Serial.println("- Triple press: Toggle priority for selected device");
    }
    Serial.println("=====================\n");
}

void clearAllBonds() {
    Serial.println("Clearing all BLE bonds...");
    
    // Get list of bonded devices before clearing
    int bondedCount = esp_ble_get_bond_device_num();
    Serial.printf("Clearing %d bonded devices\n", bondedCount);
    
    if (bondedCount > 0) {
        esp_ble_bond_dev_t* bondedDevices = (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * bondedCount);
        if (bondedDevices) {
            esp_ble_get_bond_device_list(&bondedCount, bondedDevices);
            
            // Remove each bonded device individually
            for (int i = 0; i < bondedCount; i++) {
                Serial.printf("Removing bond %d: %02x:%02x:%02x:%02x:%02x:%02x\n", i + 1,
                    bondedDevices[i].bd_addr[0], bondedDevices[i].bd_addr[1],
                    bondedDevices[i].bd_addr[2], bondedDevices[i].bd_addr[3],
                    bondedDevices[i].bd_addr[4], bondedDevices[i].bd_addr[5]);
                
                esp_ble_remove_bond_device(bondedDevices[i].bd_addr);
            }
            free(bondedDevices);
        }
    }
    
    // Clear whitelist
    clearWhitelist();
    Serial.println("Whitelist cleared");
    
    // Clear only our custom NVS data, not the entire NVS
    cleanupNVSStorage();
    
    // Visual feedback
    for(int i = 0; i < 5; i++) {
        digitalWrite(CONNECTED_LED_PIN, HIGH);
        delay(100);
        digitalWrite(CONNECTED_LED_PIN, LOW);
        delay(100);
    }
    
    Serial.println("All bonds cleared. Restarting...");
    delay(1000);
    ESP.restart();
}

// Add this function to handle connection parameters
void setConnectionParameters(esp_bd_addr_t address) {
    // Set connection parameters directly
    esp_ble_gap_set_prefer_conn_params(address, 
        MIN_CONN_INTERVAL,  // min_conn_int
        MAX_CONN_INTERVAL,  // max_conn_int
        SLAVE_LATENCY,      // latency
        SUPERVISION_TIMEOUT // timeout
    );
}

// Enhanced onGapEvent to ensure RSSI is saved with debugging
void onGapEvent(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        case ESP_GAP_BLE_SCAN_RESULT_EVT:
            if (param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) {
                int bondedCount = esp_ble_get_bond_device_num();
                if (bondedCount > 0) {
                    esp_ble_bond_dev_t* bondedDevices = (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * bondedCount);
                    if (bondedDevices) {
                        esp_ble_get_bond_device_list(&bondedCount, bondedDevices);
                        for (int i = 0; i < bondedCount; i++) {
                            if (memcmp(bondedDevices[i].bd_addr, param->scan_rst.bda, sizeof(esp_bd_addr_t)) == 0) {
                                // Update RSSI for this device
                                saveDeviceRSSI(param->scan_rst.bda, param->scan_rst.rssi);
                                
                                // Try to extract device name from advertising data
                                if (param->scan_rst.adv_data_len > 0) {
                                    uint8_t* adv_data = param->scan_rst.ble_adv;
                                    uint8_t name_len = 0;
                                    char* device_name = (char*)esp_ble_resolve_adv_data(adv_data, ESP_BLE_AD_TYPE_NAME_CMPL, &name_len);
                                    
                                    if (!device_name) {
                                        // Try short name if complete name not found
                                        device_name = (char*)esp_ble_resolve_adv_data(adv_data, ESP_BLE_AD_TYPE_NAME_SHORT, &name_len);
                                    }
                                    
                                    if (device_name && name_len > 0) {
                                        // Null-terminate the name
                                        char temp_name[32] = {0};
                                        if (name_len < sizeof(temp_name)) {
                                            memcpy(temp_name, device_name, name_len);
                                            saveAutoDetectedDeviceName(param->scan_rst.bda, temp_name);
                                        }
                                    }
                                }
                                break;
                            }
                        }
                        free(bondedDevices);
                    }
                }
            }
            break;

        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
            // Restart scanning after a short delay
            delay(100);
            startRSSIScan();
            break;

        case ESP_GAP_BLE_AUTH_CMPL_EVT:
            if (param->ble_security.auth_cmpl.success) {
                Serial.println("Authentication successful");
                
                // Get the authenticated device address
                esp_bd_addr_t addr;
                memcpy(addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
                
                // If we're in pairing mode, add this device to the whitelist
                if (isPairingMode) {
                    Serial.println("New device bonded in pairing mode - adding to whitelist");
                    addNewDeviceToWhitelist(addr);
                }
                
                // Check if this is a priority device
                bool isDevicePriority = getDevicePriority(addr);
                
                // If it's not priority and a priority device is available, disconnect
                if (!isDevicePriority && isPriorityDeviceAvailable()) {
                    Serial.println("Disconnecting non-priority device - priority device in range");
                    if (pServer != nullptr) {
                        pServer->disconnect(pServer->getConnId());
                    }
                    return;
                }
                
                // Accept the connection
                memcpy(connectedDeviceAddr, addr, sizeof(esp_bd_addr_t));
                hasConnectedDevice = true;
                
                // Set connection parameters
                setConnectionParameters(addr);
            } else {
                Serial.printf("Authentication failed, error: %d\n", param->ble_security.auth_cmpl.fail_reason);
                if (pServer != nullptr) {
                    pServer->disconnect(pServer->getConnId());
                }
            }
            break;
            
        case ESP_GAP_BLE_SEC_REQ_EVT:
            {
                // Get the requesting device's address
                esp_bd_addr_t addr;
                memcpy(addr, param->ble_security.ble_req.bd_addr, sizeof(esp_bd_addr_t));
                
                Serial.printf("Security request from: %02x:%02x:%02x:%02x:%02x:%02x\n",
                    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
                
                // If NOT in pairing mode, check if device is whitelisted BEFORE allowing authentication
                if (!isPairingMode) {
                    if (!isDeviceWhitelisted(addr)) {
                        Serial.println("Device not whitelisted - rejecting security request");
                        Serial.printf("Rejected device: %02x:%02x:%02x:%02x:%02x:%02x\n",
                            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
                        
                        // Reject the security request
                        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, false);
                        return;
                    } else {
                        Serial.println("Device is whitelisted - accepting security request");
                    }
                } else {
                    Serial.println("In pairing mode - accepting security request from any device");
                }
                
                // Accept security request
                esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
            }
            break;

        default:
            break;
    }
}

// Function to get device info as JSON with caching and improved efficiency
String getDevicesJson() {
    int bondedCount = esp_ble_get_bond_device_num();
    
    // Check if we can use cached JSON
    if (jsonCacheValid && 
        lastDeviceCount == bondedCount && 
        (millis() - lastJsonUpdate < 5000)) {  // Cache valid for 5 seconds
        DEBUG_FULLLN("Using cached JSON");
        return cachedDevicesJson;
    }
    
    DEBUG_VERBOSELN("\n=== Generating Device JSON ===");
    DEBUG_VERBOSEF("Found %d bonded devices\n", bondedCount);
    
    String json = "[";
    
    if (bondedCount == 0) {
        DEBUG_VERBOSELN("No devices found, returning empty array");
        json += "]";
        // Cache the result
        cachedDevicesJson = json;
        jsonCacheValid = true;
        lastDeviceCount = bondedCount;
        lastJsonUpdate = millis();
        return json;
    }
    
    esp_ble_bond_dev_t* bondedDevices = getBondedDevicesSafe(&bondedCount);
    if (!bondedDevices) {
        DEBUG_VERBOSELN("Failed to get bonded devices");
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
        
        // Get RSSI from cache - optimized search
        if (getDeviceRSSI(bondedDevices[i].bd_addr, &rssi)) {
            hasRssi = true;
            distance = calculateDistance(rssi);
        }
        
        bool priority = getDevicePriority(bondedDevices[i].bd_addr);
        
        DEBUG_FULLF("\nDevice %d: MAC: %s, Name: %s, Priority: %s, RSSI: %d, Distance: %.1f\n", 
                   i + 1, mac, name, priority ? "true" : "false", hasRssi ? rssi : -99, distance);
        
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
    
    DEBUG_FULLLN("\nFinal JSON generated and cached");
    return json;
}

// Enhanced web server handlers with more debugging
void handleRoot() {
    Serial.println("\n=== Serving Root Page ===");
    Serial.println("Client IP: " + server.client().remoteIP().toString());
    Serial.println("Sending HTML page size: " + String(strlen(html_page)));
    Serial.println("First 100 chars of HTML: ");
    Serial.println(String(html_page).substring(0, 100));
    
    // Set content type and CORS headers
    server.sendHeader("Content-Type", "text/html");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    
    // Send the HTML page from PROGMEM
    server.send_P(200, "text/html", html_page);
    Serial.println("HTML page sent");
}

void handleDevices() {
    Serial.println("\n=== Serving Devices List ===");
    Serial.println("Client IP: " + server.client().remoteIP().toString());
    String json = getDevicesJson();
    Serial.println("Generated JSON size: " + String(json.length()));
    Serial.println("JSON content: " + json);
    server.send(200, "application/json", json);
    Serial.println("JSON data sent");
}

void handleTogglePairing() {
    Serial.println("Web request: GET /pairing");
    Serial.println("Current pairing mode before toggle: " + String(isPairingMode ? "ACTIVE" : "INACTIVE"));
    
    isPairingMode = !isPairingMode;
    Serial.printf("Web interface pairing toggle: %s\n", isPairingMode ? "ACTIVE" : "INACTIVE");
    
    if (isPairingMode) {
        pairingModeStartTime = millis();
        Serial.println("\n=== WEB: ENTERING PAIRING MODE ===");
        // Start advertising in pairing mode
        startAdvertising(true);
        server.send(200, "text/plain", "Pairing mode active");
    } else {
        Serial.println("\n=== WEB: EXITING PAIRING MODE ===");
        // Stop advertising in pairing mode
        startAdvertising(false);
        server.send(200, "text/plain", "Pairing mode inactive");
    }
    printBondedDevices();
}

void handleTogglePriority() {
    String json = server.arg("plain");
    Serial.println("Web request: POST /priority with data: " + json);
    
    // Parse JSON
    DynamicJsonDocument doc(200);
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        Serial.println("Failed to parse JSON");
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }
    
    const char* macStr = doc["mac"];
    if (!macStr) {
        Serial.println("No MAC address provided");
        server.send(400, "text/plain", "MAC address required");
        return;
    }
    
    esp_bd_addr_t address;
    int parsed = sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
        &address[0], &address[1], &address[2],
        &address[3], &address[4], &address[5]);
    
    if (parsed == 6) {
        bool currentPriority = getDevicePriority(address);
        Serial.printf("Current priority before toggle: %s\n", currentPriority ? "HIGH" : "NORMAL");
        saveDevicePriority(address, !currentPriority);
        Serial.printf("Priority toggled for device %s: %s\n", macStr, 
                     !currentPriority ? "HIGH" : "NORMAL");
        server.send(200, "text/plain", "Priority updated");
    } else {
        Serial.println("Invalid MAC address format");
        server.send(400, "text/plain", "Invalid MAC address format");
    }
}

void handleRemoveDevice() {
    String json = server.arg("plain");
    Serial.println("Web request: POST /remove with data: " + json);
    
    // Parse JSON
    DynamicJsonDocument doc(200);
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        Serial.println("Failed to parse JSON");
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }
    
    const char* macStr = doc["mac"];
    if (!macStr) {
        Serial.println("No MAC address provided");
        server.send(400, "text/plain", "MAC address required");
        return;
    }
    
    esp_bd_addr_t address;
    int parsed = sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
        &address[0], &address[1], &address[2],
        &address[3], &address[4], &address[5]);
    
    if (parsed == 6) {
        Serial.printf("Attempting to remove device: %s\n", macStr);
        
        // Remove from whitelist first
        removeFromWhitelist(address, BLE_WL_ADDR_TYPE_PUBLIC);
        Serial.println("Device removed from whitelist");
        
        esp_err_t err = esp_ble_remove_bond_device(address);
        if (err == ESP_OK) {
            DEBUG_BASICLN("Bond removal successful");
            removeDeviceName(address);
            removeDevicePriority(address);
            invalidateDeviceCache();
            
            // Clear cache entries for this device
            for (int i = 0; i < RSSI_CACHE_SIZE; i++) {
                if (rssiCache[i].valid && 
                    memcmp(rssiCache[i].address, address, sizeof(esp_bd_addr_t)) == 0) {
                    rssiCache[i].valid = false;
                    Serial.println("RSSI cache entry cleared");
                    break;
                }
            }
            for (int i = 0; i < NAME_CACHE_SIZE; i++) {
                if (nameCache[i].valid && 
                    memcmp(nameCache[i].address, address, sizeof(esp_bd_addr_t)) == 0) {
                    nameCache[i].valid = false;
                    Serial.println("Name cache entry cleared");
                    break;
                }
            }
            for (int i = 0; i < PRIORITY_CACHE_SIZE; i++) {
                if (priorityCache[i].valid && 
                    memcmp(priorityCache[i].address, address, sizeof(esp_bd_addr_t)) == 0) {
                    priorityCache[i].valid = false;
                    Serial.println("Priority cache entry cleared");
                    break;
                }
            }
            Serial.printf("Device %s removed successfully\n", macStr);
            server.send(200, "text/plain", "Device removed");
        } else {
            Serial.printf("Failed to remove device %s, error: %d\n", macStr, err);
            server.send(500, "text/plain", "Failed to remove device");
        }
    } else {
        Serial.println("Invalid MAC address format");
        server.send(400, "text/plain", "Invalid MAC address format");
    }
}

void handleSetName() {
    String json = server.arg("plain");
    Serial.println("Web request: POST /setname with data: " + json);
    
    // Parse JSON
    DynamicJsonDocument doc(200);
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        Serial.println("Failed to parse JSON");
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }
    
    const char* macStr = doc["mac"];
    const char* name = doc["name"];
    
    if (!macStr || !name) {
        Serial.println("Missing MAC address or name");
        server.send(400, "text/plain", "MAC address and name required");
        return;
    }
    
    esp_bd_addr_t address;
    int parsed = sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
        &address[0], &address[1], &address[2],
        &address[3], &address[4], &address[5]);
    
    if (parsed == 6) {
        Serial.printf("Setting name for device %s to: %s\n", macStr, name);
        saveDeviceName(address, name);
        Serial.println("Name saved successfully");
        server.send(200, "text/plain", "Device name updated");
    } else {
        Serial.println("Invalid MAC address format");
        server.send(400, "text/plain", "Invalid MAC address format");
    }
}

// Enhanced startAdvertising function for better discoverability
void startAdvertising(bool discoverable) {
    if(pServer == nullptr) return;
    
    // Check if we can accept new bonds
    int currentBonds = esp_ble_get_bond_device_num();
    if (discoverable && currentBonds >= MAX_BONDS) {
        Serial.println("Cannot enter pairing mode - maximum bonds reached!");
        Serial.println("Please remove a device first.");
        isPairingMode = false;
        return;
    }
    
    BLEAdvertising* pAdvertising = pServer->getAdvertising();
    pAdvertising->stop();
    delay(100); // Give time for stop to complete
    
    if(discoverable) {
        Serial.println("Setting up advertising for pairing mode:");
        Serial.println("- Fast advertising for discovery");
        Serial.println("- General discoverable mode");
        Serial.println("- Interval: 20ms to 40ms");
        Serial.println("- Allowing ALL devices (whitelist disabled)");
        
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
        
        // For pairing mode, we'll use regular advertising and filter in the GAP event handler
        
    } else {
        Serial.println("Setting up advertising for normal mode:");
        Serial.println("- Slow advertising to save power");
        Serial.println("- Limited discoverable mode");
        Serial.println("- Interval: 160ms to 320ms");
        Serial.println("- WHITELIST ENABLED - only bonded devices allowed");
        
        // Set up advertising for normal mode - whitelist only
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
        
        // For normal mode, we'll filter connections in the GAP event handler using whitelist
    }
    
    pAdvertising->start();
    Serial.println("Advertising started");
}



// Create a proper callback class
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        Serial.println("Device attempting to connect...");
        
        // Get the connecting device's address
        esp_bd_addr_t peerAddr;
        esp_ble_gap_cb_param_t* param = nullptr; // We'll need to get this from the connection event
        
        // If not in pairing mode, check if device is whitelisted
        if (!isPairingMode) {
            // Note: In the Arduino BLE library, we don't have direct access to the connecting device's
            // address in the onConnect callback. The whitelist filtering will be handled in the
            // GAP event handler during the connection/authentication process.
            Serial.println("Not in pairing mode - will verify whitelist during authentication");
        } else {
            Serial.println("In pairing mode - accepting connection from any device");
        }
        
        // Accept connection initially
        Serial.println("Device connected");
        isConnected = true;
        digitalWrite(CONNECTED_LED_PIN, HIGH);
        
        // The actual device address and whitelist checking will be handled in the GAP event handler
        // when we receive ESP_GAP_BLE_AUTH_CMPL_EVT
    }
    
    void onDisconnect(BLEServer* pServer) {
        Serial.println("Device disconnected");
        hasConnectedDevice = false;
        isConnected = false;
        digitalWrite(CONNECTED_LED_PIN, LOW);
        
        // Restart advertising
        startAdvertising(isPairingMode);
    }
};

// Enhanced server route setup with more debugging
void setupWebServer() {
    // Set up WiFi in AP mode with more debugging
    Serial.println("\n=== Setting up WiFi Access Point ===");
    WiFi.mode(WIFI_AP); // Explicitly set AP mode
    bool apStarted = WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
    
    if (apStarted) {
        Serial.println("WiFi AP Started Successfully");
        Serial.print("SSID: ");
        Serial.println(WIFI_SSID);
        Serial.print("Password: ");
        Serial.println(WIFI_PASSWORD);
        Serial.print("AP IP Address: ");
        Serial.println(WiFi.softAPIP());
        
        // Print connected stations every 30 seconds
        Serial.println("\nStation connection status will be printed every 30 seconds");
    } else {
        Serial.println("Failed to start WiFi AP!");
        return;
    }

    // Set up web server routes with enhanced debugging
    server.enableCORS(true); // This is sufficient for basic CORS support
    
    // Handle OPTIONS requests for CORS preflight
    server.on("/", HTTP_OPTIONS, []() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        server.send(204);
    });
    
    server.on("/", HTTP_GET, []() {
        // Add CORS headers directly in the handler
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        
        Serial.println("\n=== Serving Root Page ===");
        Serial.println("Client IP: " + server.client().remoteIP().toString());
        Serial.println("Sending HTML page size: " + String(strlen(html_page)));
        Serial.println("First 100 chars of HTML: ");
        Serial.println(String(html_page).substring(0, 100));
        handleRoot();
        Serial.println("HTML page sent");
    });
    
    server.on("/devices", HTTP_GET, []() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        Serial.println("\n=== Handling Devices Request ===");
        Serial.println("Client IP: " + server.client().remoteIP().toString());
        handleDevices();
        Serial.println("Devices response sent");
    });
    
    server.on("/pairing", HTTP_GET, []() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        Serial.println("\n=== Handling Pairing Request ===");
        Serial.println("Client IP: " + server.client().remoteIP().toString());
        handleTogglePairing();
        Serial.println("Pairing response sent");
    });
    
    // Add OPTIONS handlers for all POST endpoints
    server.on("/setname", HTTP_OPTIONS, []() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        server.send(204);
    });
    
    server.on("/priority", HTTP_OPTIONS, []() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        server.send(204);
    });
    
    server.on("/remove", HTTP_OPTIONS, []() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        server.send(204);
    });
    
    // Add POST handlers
    server.on("/setname", HTTP_POST, []() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        DEBUG_VERBOSELN("\n=== Handling Set Name Request ===");
        DEBUG_VERBOSE("Client IP: " + server.client().remoteIP().toString());
        handleSetName();
        DEBUG_VERBOSELN("Set name response sent");
    });
    
    server.on("/priority", HTTP_POST, []() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        DEBUG_VERBOSELN("\n=== Handling Priority Request ===");
        DEBUG_VERBOSE("Client IP: " + server.client().remoteIP().toString());
        handleTogglePriority();
        DEBUG_VERBOSELN("Priority response sent");
    });
    
    server.on("/remove", HTTP_POST, []() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        DEBUG_VERBOSELN("\n=== Handling Remove Request ===");
        DEBUG_VERBOSE("Client IP: " + server.client().remoteIP().toString());
        handleRemoveDevice();
        DEBUG_VERBOSELN("Remove response sent");
    });
    
    // Add a test endpoint for basic connectivity testing
    server.on("/test", HTTP_GET, []() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        Serial.println("\n=== Handling Test Request ===");
        Serial.println("Client IP: " + server.client().remoteIP().toString());
        server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"test endpoint working\"}");
        Serial.println("Test response sent");
    });
    
    // Start the server
    server.begin();
    Serial.println("\n=== Web Server Started ===");
    Serial.println("Ready to accept connections");
}

// Add a function to monitor WiFi status
void printWiFiStatus() {
    Serial.println("\n=== WiFi Status ===");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("Connected stations: ");
    Serial.println(WiFi.softAPgetStationNum());
    Serial.println("=================\n");
}

// Enhanced setup function to properly initialize BLE for discoverability
void setup() {
    Serial.begin(115200);
    pinMode(PAIRING_BUTTON_PIN, INPUT_PULLUP);
    pinMode(CONNECTED_LED_PIN, OUTPUT);
    pinMode(AUTH_LED_PIN, OUTPUT);  // Initialize auth LED pin
    digitalWrite(CONNECTED_LED_PIN, LOW);
    digitalWrite(AUTH_LED_PIN, LOW);  // Start with auth LED off
    
    // Initialize RSSI cache
    for (int i = 0; i < RSSI_CACHE_SIZE; i++) {
        rssiCache[i].valid = false;
    }
    
    // Initialize device name cache
    for (int i = 0; i < NAME_CACHE_SIZE; i++) {
        nameCache[i].valid = false;
    }
    
    // Initialize device priority cache
    for (int i = 0; i < PRIORITY_CACHE_SIZE; i++) {
        priorityCache[i].valid = false;
    }
    
    Serial.println("\n\n=== Starting Secure BLE Car Control ===");
    Serial.println("Button Controls:");
    Serial.println("- Short press: Toggle pairing mode");
    Serial.println("- Double press: Enter device selection mode");
    Serial.println("- In selection mode:");
    Serial.println("  * Short press: Select next device");
    Serial.println("  * Long press: Remove selected device");
    Serial.println("- Very long press (3s): Clear all bonds");
    
    // Initialize NVS with careful handling to preserve BLE bonding data
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        Serial.println("NVS storage issue detected - attempting recovery...");
        // Only erase if absolutely necessary and try to preserve bonding data
        Serial.println("WARNING: This may clear bonding data. Attempting selective recovery...");
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        Serial.printf("Error initializing NVS: %d\n", err);
    } else {
        Serial.println("NVS initialized successfully");
        
        // Try to open our namespace to ensure it's working
        nvs_handle_t test_handle;
        err = nvs_open(BOND_STORAGE_NAMESPACE, NVS_READWRITE, &test_handle);
        if (err == ESP_OK) {
            Serial.println("Custom NVS namespace accessible");
            nvs_close(test_handle);
        } else {
            Serial.printf("Warning: Cannot access custom NVS namespace: %d\n", err);
        }
        
        // Check if BLE bonding data exists
        int bondCount = esp_ble_get_bond_device_num();
        Serial.printf("Found %d existing bonded devices in NVS\n", bondCount);
    }
    
    // Initialize BLE with proper configuration
    BLEDevice::init("Ghost-Key Secure");
    
    // Set BLE power to maximum for better discoverability
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
    
    pServer = BLEDevice::createServer();
    
    // Set up connection callbacks
    pServer->setCallbacks(new MyServerCallbacks());
    
    // Set up security with proper bonding
    BLESecurity* pSecurity = new BLESecurity();
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
    pSecurity->setCapability(ESP_IO_CAP_NONE);
    pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
    
    // Start keyboard
    bleKeyboard.begin();
    
    // Register GAP callback for events
    esp_ble_gap_register_callback(onGapEvent);
    
    // Start RSSI scanning
    startRSSIScan();
    
    // Start advertising in normal mode initially
    startAdvertising(false);
    
    // Check bonded devices immediately after BLE initialization
    Serial.println("\n=== CHECKING BONDED DEVICES AFTER BLE INIT ===");
    int initialBondCount = esp_ble_get_bond_device_num();
    Serial.printf("ESP32 reports %d bonded devices after BLE init\n", initialBondCount);
    
    if (initialBondCount > 0) {
        esp_ble_bond_dev_t* bondedDevices = (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * initialBondCount);
        if (bondedDevices) {
            esp_ble_get_bond_device_list(&initialBondCount, bondedDevices);
            Serial.println("Bonded devices found:");
            for (int i = 0; i < initialBondCount; i++) {
                Serial.printf("  %d: %02x:%02x:%02x:%02x:%02x:%02x\n", i + 1,
                    bondedDevices[i].bd_addr[0], bondedDevices[i].bd_addr[1],
                    bondedDevices[i].bd_addr[2], bondedDevices[i].bd_addr[3],
                    bondedDevices[i].bd_addr[4], bondedDevices[i].bd_addr[5]);
            }
            free(bondedDevices);
        }
    } else {
        Serial.println("No bonded devices found - this may indicate NVS corruption");
    }
    Serial.println("=== END BOND CHECK ===\n");
    
    // Preload existing device data from NVS into cache BEFORE setting up other components
    Serial.println("Preloading device data from NVS...");
    preloadDeviceNamesFromNVS();
    preloadDevicePrioritiesFromNVS();
    
    // Show what we loaded
    Serial.println("\n=== DEVICE NAME LOADING RESULTS ===");
    bool foundAnyNames = false;
    for (int i = 0; i < NAME_CACHE_SIZE; i++) {
        if (nameCache[i].valid) {
            Serial.printf("Cache[%d]: %02x:%02x:%02x:%02x:%02x:%02x -> %s (custom: %s, nvs: %s)\n", i,
                nameCache[i].address[0], nameCache[i].address[1], nameCache[i].address[2],
                nameCache[i].address[3], nameCache[i].address[4], nameCache[i].address[5],
                nameCache[i].name, 
                nameCache[i].hasCustomName ? "YES" : "NO",
                nameCache[i].nvsStored ? "YES" : "NO");
            foundAnyNames = true;
        }
    }
    if (!foundAnyNames) {
        Serial.println("No device names found in cache after preload");
    }
    Serial.println("===================================\n");
    
    // Populate whitelist with existing bonded devices
    Serial.println("Initializing whitelist with bonded devices...");
    populateWhitelistFromBondedDevices();
    
    printBondedDevices();
    
    // Initialize web server
    setupWebServer();
    
    Serial.println("Setup complete. Device ready for connections.");
}

void startRSSIScan() {
    esp_ble_scan_params_t scan_params = {
        .scan_type = BLE_SCAN_TYPE_ACTIVE,  // Changed to active scanning
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval = 0x50,  // Reduced interval for more frequent updates
        .scan_window = 0x30,    // Increased window for better reception
        .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE
    };
    esp_ble_gap_set_scan_params(&scan_params);
    esp_ble_gap_start_scanning(0);
}

void stopRSSIScan() {
    esp_ble_gap_stop_scanning();
}

// Add this function to check authentication status
void updateAuthenticationStatus() {
    bool wasAuthenticated = isAuthenticated;
    isAuthenticated = false;  // Start with false, will set to true if conditions met
    
    if (isConnected) {
        // Get the most recent RSSI for the connected device
        int bondedCount = esp_ble_get_bond_device_num();
        if (bondedCount > 0) {
            esp_ble_bond_dev_t* bondedDevices = (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * bondedCount);
            if (bondedDevices) {
                esp_ble_get_bond_device_list(&bondedCount, bondedDevices);
                
                // Check RSSI for each connected device
                for (int i = 0; i < bondedCount; i++) {
                    int8_t rssi = 0;
                    if (getDeviceRSSI(connectedDeviceAddr, &rssi)) {
                        if (rssi >= RSSI_AUTH_THRESHOLD) {
                            isAuthenticated = true;
                            break;
                        }
                    }
                }
                free(bondedDevices);
            }
        }
    }
    
    // Update LED if authentication status changed
    if (wasAuthenticated != isAuthenticated) {
        digitalWrite(AUTH_LED_PIN, isAuthenticated ? HIGH : LOW);
        Serial.printf("Authentication status changed: %s\n", isAuthenticated ? "Authenticated" : "Not authenticated");
    }
}

// Helper function to check if a device is priority and in range
bool isPriorityDeviceAvailable() {
    int bondedCount = esp_ble_get_bond_device_num();
    if (bondedCount == 0) return false;

    esp_ble_bond_dev_t* bondedDevices = (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * bondedCount);
    if (!bondedDevices) return false;

    bool found = false;
    esp_ble_get_bond_device_list(&bondedCount, bondedDevices);
    
    for (int i = 0; i < bondedCount; i++) {
        if (getDevicePriority(bondedDevices[i].bd_addr)) {
            int8_t rssi;
            if (getDeviceRSSI(bondedDevices[i].bd_addr, &rssi)) {
                found = true;
                break;
            }
        }
    }
    
    free(bondedDevices);
    return found;
}

// Function to populate whitelist with all bonded devices
void populateWhitelistFromBondedDevices() {
    Serial.println("Populating whitelist with bonded devices...");
    
    // Clear existing whitelist first
    clearWhitelist();
    
    int bondedCount = esp_ble_get_bond_device_num();
    if (bondedCount == 0) {
        Serial.println("No bonded devices to add to whitelist");
        return;
    }
    
    esp_ble_bond_dev_t* bondedDevices = (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * bondedCount);
    if (!bondedDevices) {
        Serial.println("Failed to allocate memory for bonded devices");
        return;
    }
    
    esp_ble_get_bond_device_list(&bondedCount, bondedDevices);
    
    for (int i = 0; i < bondedCount; i++) {
        // Add each bonded device to whitelist
        // For bonded devices, we typically use BLE_WL_ADDR_TYPE_PUBLIC
        addToWhitelist(bondedDevices[i].bd_addr, BLE_WL_ADDR_TYPE_PUBLIC);
        
        Serial.printf("Added to whitelist: %02x:%02x:%02x:%02x:%02x:%02x\n",
            bondedDevices[i].bd_addr[0], bondedDevices[i].bd_addr[1], bondedDevices[i].bd_addr[2],
            bondedDevices[i].bd_addr[3], bondedDevices[i].bd_addr[4], bondedDevices[i].bd_addr[5]);
    }
    
    free(bondedDevices);
    Serial.printf("Whitelist populated with %d bonded devices\n", bondedCount);
}

// Function to add a newly bonded device to whitelist
void addNewDeviceToWhitelist(esp_bd_addr_t address) {
    Serial.printf("Adding newly bonded device to whitelist: %02x:%02x:%02x:%02x:%02x:%02x\n",
        address[0], address[1], address[2], address[3], address[4], address[5]);
    
    // Add to whitelist (assuming public address type for most devices)
    addToWhitelist(address, BLE_WL_ADDR_TYPE_PUBLIC);
}

// Function to check if a device is whitelisted (in bonded devices list)
bool isDeviceWhitelisted(esp_bd_addr_t address) {
    int bondedCount = esp_ble_get_bond_device_num();
    if (bondedCount == 0) return false;
    
    esp_ble_bond_dev_t* bondedDevices = (esp_ble_bond_dev_t*)malloc(sizeof(esp_ble_bond_dev_t) * bondedCount);
    if (!bondedDevices) return false;
    
    bool found = false;
    esp_ble_get_bond_device_list(&bondedCount, bondedDevices);
    
    for (int i = 0; i < bondedCount; i++) {
        if (memcmp(bondedDevices[i].bd_addr, address, sizeof(esp_bd_addr_t)) == 0) {
            found = true;
            break;
        }
    }
    
    free(bondedDevices);
    return found;
}

void loop() {
    static unsigned long lastPressTime = 0;
    static int pressCount = 0;
    static unsigned long lastRSSIUpdate = 0;
    static bool wasPressed = false;
    static unsigned long lastWebServerCheck = 0;
    static unsigned long lastWiFiCheck = 0;
    static unsigned long lastPriorityCheck = 0;
    
    isConnected = bleKeyboard.isConnected();
    
    // Check for priority device every 2 seconds
    if (hasConnectedDevice && (millis() - lastPriorityCheck > 2000)) {
        lastPriorityCheck = millis();
        
        // If current device is not priority
        if (!getDevicePriority(connectedDeviceAddr)) {
            // Check if priority device is available
            if (isPriorityDeviceAvailable()) {
                Serial.println("Priority device in range - disconnecting from non-priority device");
                pServer->disconnect(pServer->getConnId());
            }
        }
    }
    
    // Update authentication status
    updateAuthenticationStatus();
    
    // Handle web server requests with debugging
    if (millis() - lastWebServerCheck > 10) {  // Check every 10ms
        server.handleClient();
        checkWebServerStatus();  // Add this line
        lastWebServerCheck = millis();
    }
    
    // Print periodic status
    if (millis() - lastStatusPrint > STATUS_PRINT_INTERVAL) {
        Serial.println("\n=== STATUS UPDATE ===");
        Serial.printf("Connection: %s\n", isConnected ? "Connected" : "Disconnected");
        Serial.printf("Pairing Mode: %s\n", isPairingMode ? "Active" : "Inactive");
        Serial.printf("Device Selection Mode: %s\n", isSelectingDevice ? "Active" : "Inactive");
        printBondedDevices();
        lastStatusPrint = millis();
    }
    
    // Handle button press
    bool isPressed = (digitalRead(PAIRING_BUTTON_PIN) == LOW);
    
    // Detect button press
    if (isPressed && !wasPressed) {
        delay(50); // Debounce
        if (digitalRead(PAIRING_BUTTON_PIN) == LOW) {
            unsigned long pressStartTime = millis();
            while (digitalRead(PAIRING_BUTTON_PIN) == LOW) {
                delay(10);
            }
            unsigned long pressDuration = millis() - pressStartTime;
            
            // Check for double press (within 500ms)
            if (millis() - lastPressTime < 500) {
                pressCount++;
                if (pressCount == 2) {
                    // Double press: Enter device selection mode
                    isSelectingDevice = true;
                    isPairingMode = false;
                    selectedDeviceIndex = 0;
                    deviceSelectStartTime = millis();
                    Serial.println("\n=== ENTERING DEVICE SELECTION MODE ===");
                    printBondedDevices();
                    pressCount = 0;
                }
            } else {
                pressCount = 1;
            }
            lastPressTime = millis();
            
            if (pressDuration > 3000) {
                // Very long press: Clear all bonds
                clearAllBonds();
            } else if (pressDuration > 1000 && isSelectingDevice) {
                // Long press in selection mode: Remove selected device
                removeBondedDevice(selectedDeviceIndex);
                if (selectedDeviceIndex >= esp_ble_get_bond_device_num()) {
                    selectedDeviceIndex = 0;
                }
                printBondedDevices();
            } else if (isSelectingDevice) {
                // Short press in selection mode: Select next device
                selectedDeviceIndex++;
                if (selectedDeviceIndex >= esp_ble_get_bond_device_num()) {
                    selectedDeviceIndex = 0;
                }
                printBondedDevices();
                deviceSelectStartTime = millis(); // Reset timeout
            } else {
                // Normal short press: Toggle pairing mode
                isPairingMode = !isPairingMode;
                if (isPairingMode) {
                    Serial.println("\n=== ENTERING PAIRING MODE ===");
                    pairingModeStartTime = millis();
                } else {
                    Serial.println("\n=== EXITING PAIRING MODE ===");
                }
                printBondedDevices();
            }
        }
    }
    wasPressed = isPressed;
    
    // Handle mode timeouts
    if (isPairingMode && (millis() - pairingModeStartTime > PAIRING_MODE_TIMEOUT)) {
        isPairingMode = false;
        Serial.println("\n=== PAIRING MODE TIMEOUT ===");
        printBondedDevices();
    }
    
    if (isSelectingDevice && (millis() - deviceSelectStartTime > DEVICE_SELECT_TIMEOUT)) {
        isSelectingDevice = false;
        Serial.println("\n=== DEVICE SELECTION MODE TIMEOUT ===");
        printBondedDevices();
    }
    
    // Handle LED states
    if (isConnected) {
        digitalWrite(CONNECTED_LED_PIN, HIGH);
    } else if (isPairingMode) {
        if (millis() - lastBlinkTime > BLINK_INTERVAL) {
            ledState = !ledState;
            digitalWrite(CONNECTED_LED_PIN, ledState);
            lastBlinkTime = millis();
        }
    } else if (isSelectingDevice) {
        // Double blink pattern for device selection mode
        unsigned long cycle = millis() % 1000;
        if (cycle < 100) digitalWrite(CONNECTED_LED_PIN, HIGH);
        else if (cycle < 200) digitalWrite(CONNECTED_LED_PIN, LOW);
        else if (cycle < 300) digitalWrite(CONNECTED_LED_PIN, HIGH);
        else digitalWrite(CONNECTED_LED_PIN, LOW);
    } else {
        digitalWrite(CONNECTED_LED_PIN, LOW);
    }
    
    // Update RSSI values periodically
    if (millis() - lastRSSIUpdate > RSSI_UPDATE_INTERVAL) {
        startRSSIScan();
        lastRSSIUpdate = millis();
    }
    
    // Check WiFi status every 30 seconds
    if (millis() - lastWiFiCheck > 30000) {
        printWiFiStatus();
        lastWiFiCheck = millis();
    }
    
    delay(10);
}

// Add this function to monitor web server activity
void checkWebServerStatus() {
    static unsigned long lastCheck = 0;
    static unsigned long uptime = 0;
    
    if (millis() - lastCheck >= 5000) { // Check every 5 seconds
        uptime = millis() / 1000;
        
        Serial.println("\n=== Web Server Status ===");
        Serial.printf("Server uptime: %lu seconds\n", uptime);
        Serial.printf("Connected clients: %u\n", WiFi.softAPgetStationNum());
        Serial.printf("Server IP: %s\n", WiFi.softAPIP().toString().c_str());
        Serial.println("======================");
        
        lastCheck = millis();
    }
}


