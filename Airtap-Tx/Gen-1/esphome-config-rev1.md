```yaml
esphome:
  name: my-vent
  friendly_name: My Vent

esp32:
  board: esp32-c3-devkitm-1
  framework:
    type: arduino

# Enable logging
logger:

# Enable Home Assistant API
api:
  encryption:
    key: 

ota:
  - platform: esphome
    password: 

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

  # Enable fallback hotspot (captive portal) in case wifi connection fails
  ap:
    ssid: "My-Vent Fallback Hotspot"
    password: ""

captive_portal:

i2c:
  sda: GPIO6
  scl: GPIO7
  scan: True

globals:
  - id: fan_speed
    type: int
    restore_value: yes
    initial_value: '0'

number:
  - platform: template
    name: "Fan Speed"
    id: fan_speed_number
    min_value: 0
    max_value: 10
    step: 1
    restore_value: true
    set_action:
      - lambda: |-
          id(fan_speed) = (int)(x);
    on_value:
      - lambda: |-
          if (id(fan_speed_number).state != id(fan_speed)) {
            id(fan_speed_number).publish_state(id(fan_speed));
          }

sensor:
  - platform: ntc
    sensor: temp_resistance_reading
    calibration:      
      #Inversed for PTC
      - 3.389kOhm -> 0°C
      - 10.0kOhm -> 25°C
      - 27.219kOhm -> 50°C
    name: "AC Infinity Temperature"
    id: temp_sensor
    unit_of_measurement: "°C"
    accuracy_decimals: 2
    icon: "mdi:thermometer"

  - platform: resistance
    id: temp_resistance_reading
    sensor: adc_sensor
    configuration: DOWNSTREAM
    resistor: 10kOhm

  - platform: adc
    pin: GPIO4
    id: adc_sensor
    attenuation: 11db

output:
  - platform: ledc
    pin: GPIO2
    id: pwm_output_1
    frequency: 1000 Hz
    # power_supply: fan_speed

  - platform: ledc
    pin: GPIO3
    id: pwm_output_2
    frequency: 1000 Hz
    # power_supply: fan_speed

display:
  - platform: ssd1306_i2c
    model: "SSD1306 128x64"
    address: 0x3C
    # Celcius
    # lambda: |-
    #   it.printf(1, 0, id(myfont), "Airtap T-Series");
    #   it.printf(0, 14, id(myfont), "Temp: %.1f°C", id(temp_sensor).state);
    #   it.printf(0, 28, id(myfont), "Fan Speed: %d", id(fan_speed));
    # Fahrenheit
    lambda: |-
      it.printf(1, 0, id(myfont), "Airtap T-Series");
      float temp_f = (id(temp_sensor).state * 9.0 / 5.0) + 32.0;
      it.printf(0, 14, id(myfont), "Temp: %.1f°F", temp_f);
      it.printf(0, 28, id(myfont), "Fan Speed: %d", id(fan_speed));


# You will need to download the font file available online and place it in the same directory as your configuration file
# https://font.download/font/arial
font:
  - file: "fonts/Arial.ttf"
    id: myfont
    size: 12

binary_sensor:
  - platform: gpio
    pin: GPIO10
    name: "Mode Button"
    on_press:
      then:
        - lambda: |-
            if (id(fan_speed) == 0) {
              id(fan_speed) = 10;
            } else {
              id(fan_speed) = 0;
            }

  - platform: gpio
    pin: GPIO8
    name: "Up Button"
    on_press:
      then:
        - lambda: |-
            if (id(fan_speed) < 10) id(fan_speed) += 1;

  - platform: gpio
    pin: GPIO9
    name: "Down Button"
    on_press:
      then:
        - lambda: |-
            if (id(fan_speed) > 0) id(fan_speed) -= 1;

light:
  - platform: monochromatic
    output: pwm_output_1
    name: "Fan 1 Speed"
    id: fan_1
    internal: true  # Hide this from Home Assistant

  - platform: monochromatic
    output: pwm_output_2
    name: "Fan 2 Speed"
    id: fan_2
    internal: true  # Hide this from Home Assistant

interval:
  - interval: 1s
    then:
      - light.turn_on:
          id: fan_1
          brightness: !lambda |-
            if (id(fan_speed) == 0) {
              return 0.0;
            } else if (id(fan_speed) == 1) {
              return 0.38;  // Minimum value to start the fan
            } else {
              return (id(fan_speed) + 3) / 13.0;
            }
      - light.turn_on:
          id: fan_2
          brightness: !lambda |-
            if (id(fan_speed) == 0) {
              return 0.0;
            } else if (id(fan_speed) == 1) {
              return 0.38;  // Minimum value to start the fan
            } else {
              return (id(fan_speed) + 3) / 13.0;
            }
```