# AirTap Fan Controller - Zigbee2MQTT Setup

## Overview
This is a custom ESP32-C6 fan controller that appears as a standard Zigbee light device but controls a fan instead. It uses standard on/off and level control clusters for maximum compatibility.

## Device Features
- **On/Off Control**: Turn fan on (speed 10) or off (speed 0)
- **Speed Control**: Adjust fan speed from 0-10 using brightness control
- **Local Buttons**: Physical buttons for manual control
- **Temperature Display**: Shows current temperature on OLED
- **Zigbee Integration**: Full Zigbee2MQTT compatibility

## Zigbee2MQTT Setup

### 1. Install External Converter
Copy the `zigbee2mqtt_converter.js` file to your Zigbee2MQTT data directory:
```bash
cp zigbee2mqtt_converter.js /path/to/zigbee2mqtt/data/external_converters/
```

### 2. Restart Zigbee2MQTT
Restart your Zigbee2MQTT service to load the new converter.

### 3. Pair the Device
1. Press the **MODE** button on your AirTap device
2. The device will start pairing mode and scan for networks
3. In Zigbee2MQTT, the device should appear as "AirTap Fan Controller"

### 4. Device Control
Once paired, you can control the device via:

#### MQTT Commands
```bash
# Turn fan on (speed 10)
mosquitto_pub -h localhost -t "zigbee2mqtt/0x98a316fffe850858/set" -m '{"state": "ON"}'

# Turn fan off
mosquitto_pub -h localhost -t "zigbee2mqtt/0x98a316fffe850858/set" -m '{"state": "OFF"}'

# Set fan speed to 5 (50%)
mosquitto_pub -h localhost -t "zigbee2mqtt/0x98a316fffe850858/set" -m '{"brightness": 128}'

# Set fan speed to 7 (70%)
mosquitto_pub -h localhost -t "zigbee2mqtt/0x98a316fffe850858/set" -m '{"brightness": 179}'
```

#### Home Assistant Integration
The device will appear in Home Assistant as a light entity with:
- **On/Off Switch**: Turn fan on/off
- **Brightness Slider**: Control fan speed (0-100%)
- **Fan Speed Number**: Direct fan speed control (0-10)

## Local Controls
- **MODE Button**: Enter/exit Zigbee pairing mode
- **UP Button**: Increase fan speed
- **DOWN Button**: Decrease fan speed  
- **TOGGLE Button**: Toggle between speed 0 and 10
- **Long Press TOGGLE**: Factory reset

## Technical Details
- **Device Type**: Standard Zigbee Light (HA_ON_OFF_LIGHT_DEVICE_ID)
- **Clusters**: 
  - `genOnOff` (0x0006) - On/off control
  - `genLevelCtrl` (0x0008) - Speed control
- **Mapping**: Brightness 0-255 maps to fan speed 0-10
- **MAC Address**: `98:a3:16:ff:fe:85:08:58`

## Troubleshooting

### Device Not Found
- Ensure Zigbee2MQTT is in pairing mode
- Press MODE button on device
- Check Zigbee2MQTT logs for pairing attempts

### Fan Not Responding
- Check physical connections
- Verify PWM fan is connected to GPIO 4
- Check serial monitor for error messages

### Zigbee Connection Issues
- Press MODE button to re-enter pairing mode
- Check Zigbee2MQTT logs for connection status
- Ensure device is within range of Zigbee coordinator

## Future Improvements
- Add temperature sensor integration
- Implement fan speed feedback
- Add energy monitoring
- Support for multiple fan zones
