# CeilingFan

This project uses an Arduino to control an inexpensive radio-controlled helicopter.

My intent is to use this as a ceiling fan, with a 450-class electric helicopter fastened upside down to the ceiling.  I can then control the helicopter using MQTT.

This would also work well as a bench test for any radio-controlled project.

<https://github.com/AdamJHowell/CeilingFan>

## Setup

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

- Cyclic/Collective Pitch Mixing (CCPM) Servos plugged into digital pins D2, D3, and D4
- ESC plugged into digital pin D0
- Rudder servo plugged into digital pin D1
- Connect black or brown servo wires to ground
- Connect red servo wires to +5V
- Connect white, orange, or yellow servo wires to signal
- Ensure the ESC is provided enough amperage
- Floolight LEDs connected to GPIO1.
- TLOF (Touchdown LiftOFf area) circular LED string connected to GPIO3.
- FATO (Final Approach/Take Off) square LED string connected to GPIO15.

## API

The API for this project is a simple two-character string, where the first character is a key designating the device to be controlled, and the second character (a digit) is a value representing the setting for that device.
For lighting, zero extinguishes the light, and any non-zero value will illuminate the light.

**First character (key):**

| Key | Component |
| t | throttle servo |
| c | collective servo |
| r | rudder servo |
| f | floodlight LEDs |
| l | green TLOF circle LEDs |
| a | white FATO square LEDs |
| k | kill switch (turn off ESC and all lights, move servos to neutral, second character is ignored) |

**Second character (value):**

| Value | LED effect | Servo effect | ESC effect |
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

For servos, the 0-9 API value is multiplied by 20 to put it in a 0-180 range needed for the Arduino servo API.

The default topic of "mqttServo" is set with the 'mqttTopic' global constant near the top of the file.  The default MQTT broker address and port are also set using global constants near the top of the file.

A sample message using the Mosquitto command line utility, sending a key of `c` and a value of `8`, which will set the collective to 160° (70° downward pitch, pushing air away from the helicopter, towards the floor):

```mosquitto_pub -h 192.168.55.200 -p 2112 -i testPublish -t mqttServo -m "c8"```

No attempt is made to use QoS levels greater than 0.  This sketch is only a subscriber, and makes no attempt to respond with QoS acknowledgements.
