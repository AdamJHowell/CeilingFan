# CeilingFan

This project uses an Arduino to control an inexpensive radio-controlled helicopter.

My intent is to use this as a ceiling fan, with a 450-class electric helicopter fastened upside down to the ceiling.  I can then control the helicopter using MQTT.

This would also work well as a bench test for any radio-controlled project.

<https://github.com/AdamJHowell/CeilingFan>

Tailoring this code to your Electronic Speed Control (ESC) may require trial and error.  I found this side helpful: <http://techvalleyprojects.blogspot.com/2012/06/arduino-control-escmotor-tutorial.html>  Perhaps the most important takeaway from that is to NEVER connect your Arduino power pins to an ESC!  Other important takeaways are that the motor may not start turning until a value such as 50 is used, and the motor may stop turning at values greater than zero (like 20).

## Physical Setup

### Hardware

- HiLetgo ESP8266 NodeMCU ESP-12E.
- Hobby King HK450 CCPM radio-controlled helicopter.
- Turnigy Typhoon 450H 2215H Heli Motor 3550kv.
- Hobbyking SS Series 25-30A ESC.
- 4x Corona 919MG Digital Metal Gear Servo.
- Antec NEO480 PSU.
- Electronics-Salon 24/20-pin ATX DC Power Supply Breakout Board Module.
- Radio Shack Experimentor 350 breadboard.
- Hookup wire (various).

### Connections

- Cyclic/Collective Pitch Mixing (CCPM) Servos plugged into digital pins D2, D3, and D4.  At least one servo will need to be reversed, which is done in software.
- ESC plugged into digital pin D0.
- Rudder servo plugged into digital pin D1.
- Connect black or brown servo wires to ground.
- Connect red servo wires to +5V.
- Connect white, orange, or yellow servo wires to signal.
- Ensure the ESC is provided enough amperage.
- Floolight LEDs connected to GPIO1.
- TLOF (Touchdown LiftOFf area) circular LED string connected to GPIO3.
- FATO (Final Approach/Take Off) square LED string connected to GPIO15.

## Software

### Libraries

- **ESP8266WiFi.h** provides the network client for MQTT to use, and WiFi functions used to connect to a local area network in station mode.
- **PubSubClient.h** provides the MQTT client.  This program only uses MQTT to subscribe to a broker, it does not publish any messages.
- **Servo.h** provides the methods used to configure and control servos in software.

### Networking

The in-board WiFi module handles all IP layer communications.
MQTT is used

### API

The API for this project is a simple two-character string, where the first character is a key designating the device to be controlled or the action to be performed, and the second character (a decimal digit) is a value representing the setting for that device.
For lighting, zero extinguishes the light, and any non-zero value will illuminate the light.

**First character (key):**

| Key | Component or action |
|---|---|
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
