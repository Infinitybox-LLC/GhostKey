# GhostKey Developer Documentation

This document provides a comprehensive, in-depth overview of the GhostKey ESP32 project, covering firmware architecture, core logic, and the web configuration interface. It is intended for developers working on or extending the GhostKey system.

## 1. Project Overview

The GhostKey system is an advanced car security and convenience system built on the ESP32 platform. It replaces traditional key-based ignition with a modern, multi-factor authentication system, including push-to-start functionality. The primary design philosophy is to enhance vehicle security while providing a seamless, modern user experience.

### 1.1 Core Features

*   **Push-to-Start:** Modernize vehicle ignition with a simple push-button start system, replacing the mechanical key.
*   **Multi-Factor Authentication:**
    *   **Bluetooth Proximity:** Employs a sophisticated, confidence-based statistical algorithm to authenticate a paired smartphone. This goes beyond simple RSSI thresholds to create a reliable proximity detection system, allowing for seamless, hands-free unlocking and operation.
    *   **RFID:** Supports passive RFID key fobs as a primary or backup authentication method, providing a reliable alternative to Bluetooth.
    *   **Ghost Power Mode:** A security-focused mode that immobilizes the vehicle by controlling a dedicated security relay. It requires a specific, configurable input sequence (e.g., brake pedal + accessory input) for authentication, making it extremely difficult to bypass.
*   **Web Configuration Interface:** The device hosts a WiFi access point and a web server, allowing for easy configuration of all system parameters from any web browser on a phone or laptop. This removes the need for a dedicated mobile app.
*   **Modular System:** The firmware is architected to be modular. The **Ghost Key** (push-to-start and primary authentication) and **Ghost Power** (security immobilization) systems can be enabled or disabled independently via the web interface, allowing the user to tailor the system's functionality to their needs.
*   **Failsafe and Security:** Includes features like an auto-locking timer, a physical factory reset sequence, and robust BLE (Bluetooth Low Energy) security with bonding and encryption to prevent unauthorized access.

### 1.2 High-Level Architecture

The system consists of three main components:
1.  **ESP32 Firmware (`GhostKey.ino`):** The core C++ application running on the ESP32. It manages hardware I/O, the primary state machine, complex authentication logic, and the web server API.
2.  **Hardware:** A custom PCB integrating an ESP32 module, high-current relays for switching vehicle ignition circuits (Accessory, Ignition 1, Ignition 2, Starter), inputs for user actions (Brake Pedal, Start Button), an RFID reader module, and user feedback peripherals (Beeper, LEDs).
3.  **Web Interface (`config_html.h`, `setup_html.h`):** A client-side application built with HTML, CSS, and JavaScript, stored directly in the firmware's PROGMEM. This self-contained web app is served to any connected client, providing a rich, user-friendly configuration experience without requiring an internet connection.

## 2. Core Firmware (`src/GhostKey.ino`)

This is the main firmware file, containing all the logic for the GhostKey system.

### 2.1 Libraries and Includes

The firmware leverages several key libraries to manage the ESP32's hardware and features effectively:

| Library               | Purpose                                                              |
| --------------------- | -------------------------------------------------------------------- |
| `WiFi.h`              | Manages the WiFi access point for the device's configuration mode.   |
| `WebServer.h`         | Runs the lightweight web server to serve the configuration interface and handle API requests. |
| `Preferences.h`       | Provides a simple key-value store API for persistent settings in the ESP32's non-volatile storage (NVS). |
| `BLEDevice.h`         | The core library for all Bluetooth Low Energy (BLE) functionality.   |
| `BLEServer.h`         | Creates and manages the BLE server for device connections, advertising, and services. |
| `BleKeyboard.h`       | Implements the BLE Human Interface Device (HID) profile. While not used for typing, its inclusion helps with device recognition and compatibility. |
| `nvs_flash.h`         | Provides low-level access to the Non-Volatile Storage system, which is essential for storing BLE bonding information. |
| `esp_gap_ble_api.h`   | Offers low-level access to the BLE GAP (Generic Access Profile) layer for fine-grained control over advertising and security events. |
| `ArduinoJson.h`       | An efficient library used for parsing and generating JSON data for communication between the web interface and the firmware API. |
| `config_html.h`       | Contains the HTML/CSS/JS for the main configuration page as a C-style string literal. |
| `setup_html.h`        | Contains the HTML/CSS/JS for the first-time setup page.              |

### 2.2 Configuration & Constants

#### 2.2.1 Pin Definitions

All hardware pin connections are defined as constants for clarity and ease of maintenance.

| Constant              | Pin | Purpose                                     |
| --------------------- | --- | ------------------------------------------- |
| `BUTTON_PIN`          | 32  | Push-to-start button input (active low with internal pullup).    |
| `BRAKE_PIN`           | 18  | Brake pedal sensor input (active low with internal pullup).      |
| `ACCESSORY_INPUT_PIN` | 19  | Ghost Power accessory input (expects an external pullup). |
| `RELAY_ACCESSORY`     | 15  | Controls the vehicle's Accessory relay.     |
| `RELAY_IGNITION1`     | 5   | Controls the vehicle's Ignition 1 relay.    |
| `RELAY_IGNITION2`     | 4   | Controls the vehicle's Ignition 2 relay.    |
| `RELAY_START`         | 16  | Controls the vehicle's Starter relay.       |
| `RELAY_SECURITY`      | 17  | Controls the security immobilization relay. |

#### 2.2.2 Bluetooth & Confidence Authentication Constants

This is the most complex set of constants, governing the statistical authentication model. They have been tuned to provide a balance between responsiveness and stability.

*   **RSSI Buffers:** `RSSI_SHORT_TERM_SIZE` (10), `RSSI_MEDIUM_TERM_SIZE` (50), `RSSI_LONG_TERM_SIZE` (300) define the number of readings stored for different time windows of analysis (approx. 1s, 5s, 30s).
*   **Confidence Thresholds:**
    *   `CONFIDENCE_AUTH_THRESHOLD` (65.0f): The confidence level required to *gain* authentication. This value is set reasonably high to prevent false authentications from passersby.
    *   `CONFIDENCE_DEAUTH_THRESHOLD` (50.0f): The confidence level below which authentication is *lost*. This creates a **hysteresis gap** ($65\% \rightarrow 50\%$), preventing the authentication state from rapidly cycling if the user is hovering around the authentication edge.
*   **Scoring Weights:** `STABILITY_WEIGHT` (35.0f), `TREND_WEIGHT` (25.0f), `STRENGTH_WEIGHT` (40.0f) define the contribution of each metric to the total confidence score. Signal strength is weighted highest as it's the most direct indicator of proximity.
*   **Momentum & Smoothing:** `CONFIDENCE_MOMENTUM_RATE` (0.25f) and `CONFIDENCE_MOMENTUM_RATE_FAST` (0.40f) control the "learning rate" of the confidence score. A higher rate allows the score to change more quickly, which is desirable for big changes (like a user suddenly walking away), while a lower rate provides a smoother, more stable score during normal operation.
*   **Sigmoid Function:** `SIGMOID_MIDPOINT` (-60.0f) and `SIGMOID_STEEPNESS` (0.3f) are parameters for the sigmoid function used to calculate the signal strength score. This mathematical function translates the raw dBm RSSI value into a smooth, S-shaped curve, which is more representative of real-world proximity than a linear scale.

### 2.3 Data Structures

#### `SystemState`

A simple but critical enum to represent the vehicle's ignition state, forming the basis of the main state machine.

```cpp
enum SystemState {
    OFF,
    ACCESSORY,
    IGNITION,
    RUNNING,
    CONFIG_MODE
};
```

#### `RSSIAnalysis` and related structs

These structs are the heart of the confidence-based authentication system, holding all the data required for statistical analysis.

```cpp
// A single, time-stamped RSSI reading
struct RSSIReading {
    int8_t rssi;
    unsigned long timestamp;
    bool valid;
};

// Calculated statistics for a set of readings
struct RSSIStatistics {
    float mean;
    float standardDeviation;
    float variance;
    int8_t minimum;
    int8_t maximum;
    int validSamples;
};

// The main analysis structure, holding all data for a single tracked device
struct RSSIAnalysis {
    // Circular buffers for different time windows
    RSSIReading shortTerm[RSSI_SHORT_TERM_SIZE];
    RSSIReading mediumTerm[RSSI_MEDIUM_TERM_SIZE];
    RSSIReading longTerm[RSSI_LONG_TERM_SIZE];

    // Statistical results for each window
    RSSIStatistics shortTermStats, mediumTermStats, longTermStats;

    // Confidence scoring components
    float stabilityScore, trendScore, strengthScore, totalConfidence;
    
    // ... other fields for trend, quality, timing, and device tracking
};
```

### 2.4 Core Functions

#### `setup()`

This function runs once on boot and is responsible for initializing the entire system in a specific order.

1.  **Serial & Pins:** Initializes `Serial` for debugging and calls `setupPins()` to configure all GPIOs to their correct modes and initial states.
2.  **Preferences:** Calls `preferences.begin()` to initialize the NVS abstraction layer, then loads all saved settings (passwords, timeouts, enabled features).
3.  **NVS:** Initializes the underlying NVS flash system (`nvs_flash_init()`), which is required for the BLE stack to store bonding keys.
4.  **Caches:** Initializes the in-memory caches used for BLE device data to `false` or empty states.
5.  **RSSI Analysis:** Calls `resetRSSIAnalysis()` to ensure the confidence system starts in a clean state.
6.  **Bluetooth:** If Bluetooth is enabled in the loaded settings, it calls `initializeBluetooth()` to set up the BLE stack and server, then starts the first RSSI scan. It also records timing metrics for performance analysis and debugging.
7.  **RFID:** Calls `loadStoredRfidKeys()` to populate the in-memory array of authorized RFID keys from NVS.

#### `loop()`

The main loop runs continuously and orchestrates all ongoing operations, acting as the system's "heartbeat".

1.  **State Updates:** Calls `updateSystemState()` to manage time-based state transitions, such as the starter relay pulse duration or LED fading animations.
2.  **Authentication Handling:**
    *   If `ghostKeyEnabled` is true, it calls `updateBluetoothAuthentication()` to run the confidence algorithm and `handleButtonPress()` to manage the push-to-start logic. RFID scanning is also performed directly in the loop.
    *   If `ghostKeyEnabled` is false (i.e., Ghost Power only mode), it calls `updateAccessoryAuthentication()` to check for the brake + accessory input sequence.
3.  **Relay Control:** Calls `controlRelays()` to physically switch the vehicle's ignition relays based on the current `systemState`.
4.  **Security Checks:** Periodically calls `checkAutoLock()` and `updateSecurityState()` to manage the security relay and re-arm the system after the vehicle has been turned off.
5.  **Bluetooth Management:** Checks the BLE connection status, starts/stops RSSI scanning as needed to conserve power, and handles the timeout for pairing mode.
6.  **Web Server:** If `currentState` is `CONFIG_MODE`, it calls `server.handleClient()` to process any incoming HTTP requests from the configuration web page.
7.  **Factory Reset:** Continuously calls `checkFactoryReset()` to monitor for the physical button combination that triggers a full system reset.
8.  **Status Printing:** Periodically prints a detailed system status summary to the Serial monitor for debugging.

### 2.5 Subsystems Explained

#### 2.5.1 Confidence-Based Authentication

This is the most innovative part of the GhostKey system. It provides a robust solution to the unreliability of using raw RSSI values for proximity detection. The system builds a "confidence" score from 0-100% that the authenticated device is genuinely near the vehicle.

The total confidence score, $C_{total}$, is a weighted sum of three components: Signal Strength ($S_{strength}$), Signal Stability ($S_{stability}$), and Signal Trend ($S_{trend}$).

$$C_{total} = w_{strength}S_{strength} + w_{stability}S_{stability} + w_{trend}S_{trend}$$

**1. Data Collection (`addRSSIReading`)**

*   All incoming RSSI readings from the `onGapEvent` callback are passed to this function.
*   To reject noise and spurious readings, values are first passed through a **median filter**. This is more effective at rejecting single, sharp outliers than a simple moving average.
*   The filtered reading is then stored in three separate circular buffers: short-term, medium-term, and long-term, allowing for analysis over different time scales.

**2. Statistical Calculation (`calculateBufferStatistics`)**

*   This function is called for each of the three buffers to calculate key statistical metrics: the mean ($\mu$), variance ($\sigma^2$), and standard deviation ($\sigma$). This statistical data provides a much richer picture of the signal behavior than a single RSSI value.

**3. Scoring Functions**

*   **Strength Score (`calculateStrengthScore`)**: This score is calculated using a sigmoid (logistic) function. This provides a smooth, non-linear "S-curve" mapping from the mean RSSI to a score between 0 and 40, which better reflects how signal strength corresponds to physical distance.

    The formula used is:
    $$S_{strength} = W_{strength} \times \frac{1}{1 + e^{-k(\mu_{rssi} - x_0)}}$$
    Where $W_{strength}$ is `STRENGTH_WEIGHT`, $k$ is `SIGMOID_STEEPNESS`, and $x_0$ is `SIGMOID_MIDPOINT`.

*   **Stability Score (`calculateStabilityScore`)**: This score measures the signal's volatility using the standard deviation ($\sigma$). A low $\sigma$ indicates a stable signal (e.g., a user sitting still in the car) and results in a high score. The function uses an *adaptive threshold* that is more lenient at very close ranges, as minor movements can cause larger RSSI swings when the phone is close to the antenna.

*   **Trend Score (`calculateTrendScore`)**: This score analyzes the signal's direction over time. It uses linear regression to calculate the slope of recent RSSI readings. A positive slope (signal getting stronger) is rewarded, a stable signal is given a moderate score, and a negative slope (signal getting weaker) is penalized. This helps the system differentiate between a user approaching the car and a user walking away.

**4. Final Analysis (`performRSSIAnalysis`)**

*   This function orchestrates the entire process. It calls the statistical and scoring functions, sums the scores to get a base confidence level, and applies a user-defined **calibration offset**.
*   Crucially, it then applies **adaptive momentum smoothing**. This acts like a low-pass filter on the confidence score, preventing it from jumping erratically. The "adaptive" part means the smoothing factor changes based on context; it's less aggressive (smoother) when the signal is stable, and more aggressive (responsive) when the signal is changing rapidly.

**5. Authentication Logic (`authenticateByConfidence`)**

*   This function makes the final yes/no decision. It uses hysteresis to prevent state flapping:
    *   Confidence must be $\ge 65\%$ to **gain** authentication.
    *   Confidence must drop below $50\%$ to **lose** authentication.

#### 2.5.2 Bluetooth Subsystem

*   **Initialization (`initializeBluetooth`)**: Configures the BLE stack, including setting the device name, configuring security parameters (bonding required, no PIN entry), and registering the critical event callbacks.
*   **Advertising (`startBLEAdvertising`)**: Manages how the device appears to other BLE devices.
    *   In **Normal Mode**, it uses slow, limited-discoverability advertising. This saves power and ensures only already-bonded devices can see and connect to the GhostKey.
    *   In **Pairing Mode**, it switches to fast, general-discoverability advertising, making it visible to new phones for the initial pairing process.
*   **Callbacks (`onGapEvent`, `MyServerCallbacks`)**: These event-driven functions are at the core of BLE operations.
    *   `onConnect`/`onDisconnect` from `MyServerCallbacks`: Handle the basic connection and disconnection events, updating system state variables like `isBleConnected`.
    *   `ESP_GAP_BLE_SCAN_RESULT_EVT`: This GAP event is triggered when the ESP32's scanner receives an advertisement packet from a nearby device. The code checks if the device is a bonded one and, if so, extracts the RSSI value and passes it to the confidence analysis system.
    *   `ESP_GAP_BLE_SEC_REQ_EVT`: This is a critical security callback. When a device tries to connect, it triggers this event. The handler checks if the system is in pairing mode OR if the connecting device's MAC address is already in the list of bonded devices. If neither is true, the security request is rejected, and the device is disconnected, preventing unauthorized connections.

#### 2.5.3 Web Configuration Mode & API

*   **Entering/Exiting (`enterConfigMode`, `exitConfigMode`)**: A long press on the start button (without the brake) triggers `enterConfigMode`. This function starts a WiFi AP and the web server. `exitConfigMode` does the reverse.
*   **Web Server (`setupWebServer`)**: This function defines all the HTTP endpoints that form the API for the client-side web application. It uses a simple REST-like structure.

| Endpoint                  | Method | Description                                                                      |
| ------------------------- | ------ | -------------------------------------------------------------------------------- |
| `/`                       | GET    | Serves the `setup_html.h` or `config_html.h` page depending on system state.      |
| `/logo`                   | GET    | Serves the JDI logo SVG from PROGMEM.                                            |
| `/status`                 | GET    | Returns a JSON object with the current high-level system status.                 |
| `/devices`                | GET    | Returns a JSON array of all bonded Bluetooth devices and their current RSSI.     |
| `/pairing`                | POST   | Toggles Bluetooth pairing mode on or off.                                        |
| `/setname`                | POST   | Sets a custom, user-friendly name for a bonded device, stored in NVS.            |
| `/remove`                 | POST   | Removes a bonded Bluetooth device from the ESP32's memory.                       |
| `/rfid_keys`              | GET    | Returns a JSON array of all stored RFID keys.                                    |
| `/rfid_pair`              | POST   | Activates RFID pairing mode, allowing a new tag to be scanned and saved.         |
| `/rfid_remove`            | POST   | Removes a stored RFID key by its index.                                          |
| `/update_pulse`           | POST   | Updates the starter crank time.                                                  |
| `/update_web_password`    | POST   | Changes the password for accessing the web interface.                            |
| `/complete_setup`         | POST   | Marks the first-time setup as complete and saves all initial settings.           |
| `/calibration_start`      | POST   | Starts the confidence authentication calibration data collection process.        |
| `/calibration_stop`       | POST   | Stops the calibration process and calculates the optimal confidence offset.      |
| `/rssi_analysis`          | GET    | Returns a detailed JSON object with the current state of the confidence system for debugging and plotting. |

## 3. Web Configuration Interface

The web interface provides a user-friendly way to configure the GhostKey system. It's split into two main pages, both served directly from the ESP32's flash memory.

### 3.1 First-Time Setup (`src/setup_html.h`)

This page is served when the device is booted for the first time (`firstSetupComplete` is false).

*   **Purpose:** To guide the user through the essential initial configuration in a simple, unavoidable wizard format.
*   **UI:** A single-page, mobile-first form that prompts the user to:
    1.  Choose which core systems to enable (Ghost Key / Ghost Power).
    2.  Enable or disable Bluetooth functionality.
    3.  Set the initial WiFi and Web Interface passwords.
*   **JavaScript Logic:**
    *   `validateForm()`: Performs client-side validation to ensure that the user has entered valid settings (e.g., at least one system must be enabled, password length requirements are met).
    *   `completeSetup()`: Gathers all the settings from the form into a JSON object and sends it to the `/complete_setup` endpoint via a `POST` request. On success, it reloads the page, and since `firstSetupComplete` is now true on the device, the server will respond with the main configuration interface.

### 3.2 Main Configuration Page (`src/config_html.h`)

This is the main dashboard for managing all aspects of the GhostKey system, accessible after the initial setup is complete.

#### 3.2.1 Authentication

*   The page is initially hidden behind a splash screen which prompts for the web interface password.
*   The `handleLogin()` JavaScript function sends the entered password to the `/validate_web_password` endpoint.
*   If the server responds with a 200 OK status, the `isAuthenticated` flag is set to true, the splash screen is hidden, and the main app container is displayed. If not, an error message is shown.

#### 3.2.2 UI Structure

*   **Sidebar Navigation:** A sidebar allows the user to switch between different content sections (`Overview`, `Bluetooth`, `RFID`, etc.). The `showSection()` JS function handles hiding and showing the correct content panel and updating the active state of the navigation items.
*   **Content Sections:** Each section is a "card" that contains related settings and information, creating a clean, organized layout.
*   **Responsive Design:** The CSS includes multiple media queries to ensure the interface is highly usable on both desktop and mobile devices. On mobile, the sidebar collapses into a scrollable horizontal menu to save vertical space.

#### 3.2.3 JavaScript Logic (by section)

The client-side JavaScript is organized into modules of functions that correspond to the sections in the UI. Data is fetched from the ESP32's API endpoints and used to dynamically update the page.

*   **Overview:** `loadSystemStatus()` periodically fetches data from the `/status` endpoint to display the live state of the system (e.g., RFID authenticated, Bluetooth connected).
*   **Bluetooth:**
    *   `updateDevices()`: Fetches the list of bonded devices from `/devices` and dynamically builds the device card UI, including name inputs and remove buttons.
    *   `setDeviceName()`, `removeDevice()`: Send requests to the respective endpoints to manage devices.
    *   **Calibration:** The UI provides buttons to `startCalibration()`, `stopCalibration()`, and `resetCalibration()`. A timer polls the `/calibration_status` endpoint to show real-time progress and remaining time.
*   **Confidence Monitoring:**
    *   `loadConfidenceMonitoring()`: Fetches detailed data from `/rssi_analysis` at a high frequency (e.g., every 2 seconds).
    *   `updateConfidenceUI()`: Updates all the gauges and status indicators with the new data, providing real-time feedback on the authentication system's performance. This is primarily a debugging and tuning tool.
*   **Confidence Plotting:**
    *   A simple real-time chart is implemented using the HTML5 Canvas API to avoid heavy chart libraries.
    *   `startPlotting()`: Uses `setInterval` to poll the `/rssi_analysis` endpoint every second, adding the `$C_{total}$ value to a data array.
    *   `drawChart()`: Redraws the canvas with the latest data, showing the confidence trend over the last 60 seconds, along with lines for the authentication and de-authentication thresholds.
*   **RFID:** `loadRfidKeys()` fetches the list of keys and displays them. `toggleRfidPairing()` and `removeRfidKey()` provide the interface for adding and deleting keys.
*   **Security & Config:** These sections contain functions to update passwords and system settings (`updateWebPassword`, `updatePulseTime`, etc.), sending the new values to their respective endpoints.
*   **Exit:** The `exitConfig()` function calls the `/exit` endpoint, which instructs the ESP32 to shut down the web server and WiFi AP. The JavaScript then displays a confirmation splash screen to inform the user that the process is complete. 