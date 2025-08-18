// Temporary external converter for AirTap Fan Controller
// Place this file in your Zigbee2MQTT data/external_converters/ directory

const definition = {
    zigbeeModel: ['airtap-4btn-rev2'],
    model: 'airtap-4btn-rev2',
    vendor: 'SiloCityLabs',
    // Also match devices with undefined manufacturer/model (fallback)
    fingerprint: [
        {modelID: 'undefined', manufacturerName: 'undefined'},
        {modelID: 'airtap-4btn-rev2', manufacturerName: 'SiloCityLabs'},
    ],
    description: 'Custom ESP32-C6 Fan Controller',
    supports: 'on/off, fan speed control',
    fromZigbee: [{
        key: ['state', 'brightness'],
        fn: (entity, key, value, meta) => {
            if (key === 'state') {
                return {state: value};
            }
            if (key === 'brightness') {
                // Convert brightness (0-255) to fan speed (0-10)
                const fanSpeed = Math.round((value * 10) / 255);
                return {
                    brightness: value,
                    fan_speed: fanSpeed
                };
            }
        }
    }],
    toZigbee: [{
        key: ['state'],
        fn: (entity, key, value, meta) => {
            if (key === 'state') {
                return {
                    cluster: 'genOnOff',
                    payload: {onOff: value === 'ON' ? 1 : 0}
                };
            }
        }
    }, {
        key: ['brightness', 'fan_speed'],
        fn: (entity, key, value, meta) => {
            if (key === 'brightness') {
                return {
                    cluster: 'genLevelCtrl',
                    payload: {moveToLevel: value}
                };
            }
            if (key === 'fan_speed') {
                // Convert fan speed (0-10) to brightness (0-255)
                const brightness = Math.round((value * 255) / 10);
                return {
                    cluster: 'genLevelCtrl',
                    payload: {moveToLevel: brightness}
                };
            }
        }
    }],
    exposes: [
        e.switch(),
        e.brightness(),
        e.numeric('fan_speed', ea.STATE_SET)
            .withDescription('Fan speed (0-10)')
            .withValueMin(0)
            .withValueMax(10)
    ]
};

module.exports = definition;
