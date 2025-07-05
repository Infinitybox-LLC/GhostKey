# SPIFFS Data Directory

This directory contains files that will be uploaded to the ESP32's SPIFFS filesystem.

## Logo Setup

To use your actual JDI logo:

1. **Save your logo** as `jdi_logo.png` in this `data/` directory
2. **Resize** to approximately 120x120 pixels for best performance
3. **Upload to SPIFFS** using one of these methods:

### Method 1: PlatformIO (Recommended)
```bash
pio run --target uploadfs
```

### Method 2: Arduino IDE
1. Install "ESP32 Sketch Data Upload" tool
2. Tools → ESP32 Sketch Data Upload

### Method 3: Manual Upload
Use the ESP32 filesystem uploader web interface (if available)

## File Structure
```
data/
├── README.md (this file)
└── jdi_logo.png (your logo - place here)
```

## Notes
- The system will automatically serve your logo from `/logo` endpoint
- If no logo file is found, it falls back to a text-based SVG
- Logo should be PNG format for best compatibility
- Keep file size reasonable (<100KB) for ESP32 memory constraints 