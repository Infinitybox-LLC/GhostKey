# GhostKey

An ESP32-based automotive security and convenience system that provides keyless entry, push-to-start, and RFID authentication for vehicles.

## Features

- RFID-based authentication
- Push-to-start functionality
- Multiple system states (OFF, ACCESSORY, IGNITION, RUNNING)
- Configuration mode for system setup
- LED and buzzer feedback
- Security relay control

## Hardware Requirements

- ESP32 development board
- MFRC522 RFID reader
- 7 automotive relays
- Start button
- Brake pedal input
- Configuration button
- Status LEDs
- Buzzer

## Pin Configuration

### Inputs
- RFID_INPUT (IO4) - RFID input from MCP2030
- BUTTON_START (IO19) - Start button
- BRAKE_INPUT (IO22) - Brake pedal
- BUTTON_CONFIG (IO2) - Config button

### Outputs
- RELAY_ACCESSORY (IO26) - Accessory relay
- RELAY_IGNITION1 (IO27) - Ignition 1 relay
- RELAY_IGNITION2 (IO14) - Ignition 2 relay
- RELAY_START (IO12) - Start relay
- RELAY_SECURITY_POS (IO13) - Security POS relay
- RELAY_SECURITY_GND (IO15) - Security GND relay
- RELAY_SECURITY_OPEN (IO2) - Security Open relay
- LED_PIN (IO23) - Status LED
- BUTTON_LED_PIN (IO18) - Button LED
- BUZZER_PIN (IO34) - Buzzer

## System States

1. OFF - All systems off
2. ACCESSORY - Accessory power only
3. IGNITION - Accessory + Ignition power
4. RUNNING - Full system power
5. CONFIG_MODE - System configuration

## Configuration

The system can be configured through:
- RFID tag management
- Button press sequences
- Configuration mode

## Development

This project is built using PlatformIO and Arduino framework.

### Building

```bash
pio run
```

### Uploading

```bash
pio run -t upload
```

## License

[Your chosen license]

## Contributing

[Your contribution guidelines] 