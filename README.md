# ISS Tracker

A PlatformIO-based project for ESP8266 that tracks the International Space Station (ISS) and indicates its proximity to your location using colored LEDs.

## Description

This project uses an ESP8266 microcontroller to periodically fetch the current position of the International Space Station from an online API and calculates its distance from your configured location. Based on this distance, different LED indicators provide visual feedback:

- **Red LED (Solid)**: ISS is far away (over 1000 km from your location)
- **Blue LED (Blinking)**: ISS is approaching (between 500-1000 km from your location)
- **Green LED (Fast Blinking)**: ISS is nearby/potentially visible (within 500 km of your location)

The system operates non-blockingly, meaning all LED animations and periodic checks happen without using delay functions that would pause the program execution.

## Hardware Requirements

- ESP8266 development board (NodeMCU, Wemos D1 Mini, or similar)
- 3 LEDs (Red, Green, Blue)
- 3 resistors (220Ω-330Ω) for the LEDs
- Breadboard and jumper wires
- USB cable for programming and power

## Circuit Diagram

Connect the LEDs to the ESP8266 as follows:
- Red LED: D5 pin (with resistor to ground)
- Green LED: D6 pin (with resistor to ground)
- Blue LED: D7 pin (with resistor to ground)

## Software Requirements

- [PlatformIO](https://platformio.org/) (recommended to install as an extension in VS Code)
- ESP8266 board support for PlatformIO

## Setup and Configuration

### 1. Clone/Download the Repository

```bash
git clone https://github.com/charan-271/ISS_Tracker.git
cd ISS_Tracker
```

### 2. Configure Your Credentials

Create or edit the `include/credentials.h` file with your WiFi credentials and geographic location:

```cpp
#ifndef CREDENTIALS_H
#define CREDENTIALS_H

// Your Wi-Fi credentials
#define WIFI_SSID "YourWiFiName"
#define WIFI_PASS "YourWiFiPassword"

// Your location (in decimal degrees)
#define MY_LAT 17.9949  // Replace with your latitude
#define MY_LON 83.2377  // Replace with your longitude

#endif
```

### 3. Finding Your Geographic Coordinates

You need accurate coordinates for the tracker to work properly. To find your latitude and longitude:

1. Visit [Google Maps](https://www.google.com/maps)
2. Right-click on your location on the map
3. The coordinates will appear at the top of the context menu as decimal values
4. Use these values for MY_LAT and MY_LON in your credentials.h file

Alternatively, use a website like [latlong.net](https://www.latlong.net/) to search for your location.

### 4. Build and Upload

Using PlatformIO:

1. Open the project in VS Code with PlatformIO extension installed
2. Click the PlatformIO icon in the sidebar
3. Click "Build" to compile the project
4. Connect your ESP8266 via USB
5. Click "Upload" to flash the code to your device

## Usage

After uploading the code:

1. Open the Serial Monitor at 115200 baud to see debugging information
2. The device will automatically connect to WiFi using your credentials
3. Every 30 seconds, it will check the ISS position and update the LEDs accordingly
4. Monitor the serial output to see the actual distance to the ISS

## Customization

You can modify several parameters in the code:

- `VISIBLE_RADIUS` (default: 500 km): Distance threshold for when the ISS is considered "nearby"
- `SLIGHTLY_FAR_RADIUS` (default: 1000 km): Distance threshold for when the ISS is "approaching"
- `CHECK_ISS_INTERVAL` (default: 30000 ms): How often to check the ISS position
- `BLINK_FAST` and `BLINK_MEDIUM`: LED blinking speeds

## API Information

This project uses the free [Open Notify API](http://open-notify.org/) which provides real-time location data for the International Space Station. The API has no authentication requirements and does not require an API key.

## Troubleshooting

### WiFi Connection Issues
- Ensure your WiFi credentials are correct
- Check if the ESP8266 is within range of your WiFi network
- The code includes automatic reconnection logic if WiFi disconnects

### LED Not Working
- Verify correct pin connections
- Check LED polarity (longer leg should connect to the ESP8266 pin)
- Ensure resistors are properly connected

### No ISS Data
- Check your internet connection
- The API might be temporarily unavailable
- Check Serial Monitor for HTTP error codes

## Development Challenges and Solutions

### Non-blocking Operation
- Traditional Arduino code often uses `delay()` which blocks program execution
- This project implements non-blocking timing using `millis()` to allow multiple operations to occur simultaneously

### Memory Management
- The ESP8266 has limited RAM
- Used StaticJsonDocument instead of DynamicJsonDocument to avoid heap fragmentation
- Careful management of string resources to prevent memory leaks

### Distance Calculation
- Implemented the Haversine formula to accurately calculate the great circle distance between two points on Earth
- Accounts for Earth's curvature when determining ISS distance

## Future Enhancements

- Add a small OLED display to show exact ISS coordinates and distance
- Implement a prediction system to alert when ISS will be overhead during night hours (visible passes)
- Add a web interface for configuration without needing to reprogram the device
- Implement deep sleep mode to conserve power when ISS is far away

## License

This project is open source and available under the MIT License.

## Credits

- ISS position data provided by [Open Notify API](http://open-notify.org/)
- Built using the Arduino core for ESP8266