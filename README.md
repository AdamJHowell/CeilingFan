# CeilingFan

This project uses an Arduino to control an inexpensive radio-controlled helicopter.

My intent is to use this as a ceiling fan, with a 450-class electric helicopter fastened upside down to the ceiling.  I can then control the helicopter using MQTT.

https://github.com/AdamJHowell

**Hardware:**
 - HiLetgo ESP8266 NodeMCU ESP-12E
 - Hobby King HK450 CCPM radio-controlled helicopter
 - Turnigy Typhoon 450H 2215H Heli Motor 3550kv
 - Hobbyking SS Series 25-30A ESC
 - 4x Corona 919MG Digital Metal Gear Servo
 - Antec NEO480 PSU
 - Electronics-Salon 24/20-pin ATX DC Power Supply Breakout Board Module
 - Radio Shack Experimentor 350 breadboard
 - Hookup wire (various)

**CONNECTIONS:**
 - Cyclic/Collective Pitch Mixing (CCPM) Servos plugged into digital pins D2, D3, and D4.
 - ESC plugged into digital pin D0.

If separate wires:
 - Servo Black or Brown to ground.
 - Servo Red or Orange (center wire) to +5V
 - Servo White or Yellow to signal

##API
The API for this project is a simple two-character string, where the first character is a key designating the device to be controlled, and the second character (a digit) is a value representing the setting for that device.
For lighting, zero extinguishes the light, and any non-zero value will turn the light on.

**First character:**
 - t = throttle servo
 - c = collective servo
 - r = rudder servo
 - f = floodlight LEDs
 - l = green TLOF circle LEDs
 - a = white FATO square LEDs

**Second character:**

| Value | LED | Servos | ESC |
|---|---|---|---|
| 0 | off (LEDs and ESC) | maximum CCW position (servos) |
| 1 | on (LEDs) | 20° from CCW position (servos) | 11% of maximum speed (ESC) |
| 2 | on (LEDs) | 40° from CCW position (servos) | 22% of maximum speed (ESC) |
| 3 | on (LEDs) | 60° from CCW position (servos) | 33% of maximum speed (ESC) |
| 4 | on (LEDs) | 80° from CCW position (servos) | 44% of maximum speed (ESC) |
| 5 | on (LEDs) | 100° from CCW position (servos) | 55% of maximum speed (ESC) |
| 6 | on (LEDs) | 120° from CCW position (servos) | 66% of maximum speed (ESC) |
| 7 | on (LEDs) | 140° from CCW position (servos) | 77% of maximum speed (ESC) |
| 8 | on (LEDs) | 160° from CCW position (servos) | 88% of maximum speed (ESC) |
| 9 | on (LEDs) | maximum CW position (servos) | maximum speed (ESC) |

For most servos, the 0-9 value is multiplied by 20 to put it in a 0-180 range needed for the Arduino servo API.
