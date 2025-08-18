# AIRTap Zigbee 4-Button Remote

A Zigbee-enabled 4-button remote control device for AC Infinity fan controllers, built on ESP32-C6 with native IEEE 802.15.4 radio support.

## Overview

The AIRTap Zigbee 4-Button Remote is a smart remote control that allows wireless control of AC Infinity fan systems through Zigbee networks. The device features:

- **4 Physical Buttons**: Mode, Up, Down, and Toggle controls
- **Zigbee Connectivity**: Native IEEE 802.15.4 radio on ESP32-C6
- **Fan Speed Control**: 10-speed fan control (0-100%)
- **Temperature Monitoring**: Built-in NTC temperature sensor
- **Battery Efficient**: Optimized for long battery life
- **Easy Pairing**: Simple pairing process with Zigbee hubs

## Technical Specifications

### Hardware
- **Microcontroller**: ESP32-C6 (160MHz, 320KB RAM, 8MB Flash)
- **Radio**: Native IEEE 802.15.4 (Zigbee) support
- **Buttons**: 4 tactile buttons with debouncing
- **Temperature Sensor**: NTC thermistor with ADC
- **PWM Output**: Fan speed control via PWM
- **Power**: Battery powered with low-power modes

### Zigbee Specifications
- **Profile**: Zigbee Home Automation (ZHA)
- **Device Type**: On/Off Light (acts as fan controller)
- **Role**: End Device (ZED)
- **Channels**: Channel 11 (2.405 GHz)
- **Security**: Standard Zigbee security
- **Network Size**: Supports up to 64 devices

### Firmware Details
- **Framework**: ESP-IDF 5.3.2
- **Zigbee SDK**: ESP Zigbee SDK
- **Build System**: PlatformIO
- **Flash Usage**: ~850KB (92.3%)
- **RAM Usage**: ~27KB (8.3%)

## Button Functions

### Normal Operation
- **MODE Button**: 
  - Short press (1-10s): Trigger pairing mode
  - Long press (10s+): Factory reset
- **TOGGLE Button**: Turn fan on/off
- **UP Button**: Increase fan speed (1-10)
- **DOWN Button**: Decrease fan speed (1-10)

### Special Functions
- **Pairing Mode**: Press MODE button for 1-10 seconds
- **Factory Reset**: Hold MODE button for 10+ seconds
- **Speed Memory**: Device remembers last non-zero speed setting

## Zigbee Pairing Instructions

### Automatic Pairing (Recommended)
1. **Power On**: Simply turn on the device
2. **Auto-Discovery**: Device automatically searches for open Zigbee networks
3. **Join Network**: Device joins the first available network
4. **Ready**: Device is ready for use (LED indicators may show status)

### Manual Pairing
1. **Prepare Hub**: Ensure your Zigbee hub is in pairing mode
2. **Trigger Pairing**: Press and hold the MODE button for 1-10 seconds
3. **Wait for Join**: Device will attempt to join the network
4. **Confirmation**: Check your hub's app for successful pairing

### Factory Reset
1. **Reset Device**: Hold MODE button for 10+ seconds
2. **Wait for Reset**: Device will leave current network
3. **Restart Pairing**: Device automatically starts pairing process
4. **Re-pair**: Follow pairing instructions above

## Integration with Zigbee Hubs

### Supported Hubs
- **Home Assistant** (with ZHA or Zigbee2MQTT)
- **Philips Hue Bridge**
- **Samsung SmartThings**
- **Amazon Echo Plus/Show**
- **IKEA TRÅDFRI Gateway**
- **Tuya Zigbee Gateway**

### Device Recognition
The device appears as an **On/Off Light** in most Zigbee hubs:
- **Device Type**: On/Off Light
- **Manufacturer**: AC Infinity
- **Model**: AIRTap-4BTN
- **Endpoints**: 1 (Light control)

### Control Methods
- **On/Off**: Toggle fan power
- **Level Control**: Adjust fan speed (0-100%)
- **Direct Control**: Use physical buttons
- **Remote Control**: Use hub's mobile app

## Troubleshooting

### Pairing Issues
1. **Check Hub**: Ensure hub is in pairing mode
2. **Distance**: Keep device within 10-30 feet of hub
3. **Interference**: Move away from WiFi routers
4. **Reset**: Try factory reset if pairing fails
5. **Retry**: Device automatically retries pairing every 5 seconds

### Connection Issues
1. **Battery**: Check battery level
2. **Range**: Move device closer to hub
3. **Obstacles**: Remove metal objects between device and hub
4. **Network**: Check if hub's network is stable

### Button Not Responding
1. **Reset**: Try factory reset
2. **Battery**: Replace batteries
3. **Physical**: Check button contacts
4. **Firmware**: Device may need firmware update

## Development Information

### Building the Firmware
```bash
# Install PlatformIO
pip install platformio

# Build for ESP32-C6
pio run -e esp32c6

# Upload to device
pio run -e esp32c6 -t upload
```

### Project Structure
```
zigbee-4btn-rev3/
├── src/
│   └── main.c              # Main application code
├── components/
│   └── esp-zigbee-sdk/     # ESP Zigbee SDK
├── platformio.ini          # PlatformIO configuration
├── CMakeLists.txt          # CMake configuration
├── partitions.csv          # Flash partition table
└── sdkconfig.defaults      # ESP-IDF configuration
```

### Key Features Implemented
- **Zigbee End Device**: Optimized for battery life
- **Network Steering**: Automatic network discovery and joining
- **Factory Reset**: Complete network removal
- **Button Handling**: Debounced button input with special functions
- **Fan Control**: PWM-based speed control
- **Temperature Sensing**: ADC-based temperature monitoring
- **Error Handling**: Robust error recovery and retry logic

### Zigbee Stack Integration
- **BDB Commissioning**: Base Device Behavior for network joining
- **ZDO Signals**: Zigbee Device Object signal handling
- **ZCL Clusters**: Zigbee Cluster Library for device functionality
- **Security**: Standard Zigbee security implementation

## Support

For technical support or questions:
- **Documentation**: Check this README and code comments
- **Issues**: Report bugs through the project repository
- **Community**: Join AC Infinity user forums

---

**Note**: This device is designed for use with AC Infinity fan systems. Ensure compatibility with your specific fan model before use.
