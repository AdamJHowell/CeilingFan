# CeilingFan

![License](https://img.shields.io/badge/license-MIT-blue "License")
[![CodeFactor](https://www.codefactor.io/repository/github/adamjhowell/ceilingfan/badge)](https://www.codefactor.io/repository/github/adamjhowell/ceilingfan)

<https://github.com/AdamJHowell/CeilingFan>

This project uses an Arduino compatible device to control an inexpensive radio-controlled helicopter.

My intent is to use this as a ceiling fan, with a 450-class electric helicopter fastened upside down to the ceiling.
I can then control the helicopter using MQTT.

This could also work as a bench test device for any radio-controlled project.

When powered on, this device will connect to the Wi-Fi network defined in networkVariables.h (not included in this repo),
then connect to the MQTT broker configured in that same file, and finally subscribe to the "mqttServo" topic.
When messages are published to that topic, this device will receive those messages, and act accordingly.

## Physical Setup

### Hardware

| Item                                                                                                                                                           |  Cost | Quantity |  Total |
|----------------------------------------------------------------------------------------------------------------------------------------------------------------|------:|----------|-------:|
| [HiLetgo](http://www.hiletgo.com/) [ESP8266 NodeMCU ESP-12E](https://smile.amazon.com/gp/product/B010O1G1ES/)                                                  |  7.99 | 1        |   7.99 |
| [ELEGOO 4 channel 5v relay module](https://smile.amazon.com/ELEGOO-Channel-Optocoupler-Compatible-Raspberry/dp/B09ZQS2JRD/)                                    |  7.99 | 1        |   7.99 |
| [Hobby King HK450 CCPM radio-controlled helicopter](https://hobbyking.com/en_us/hk450-ccpm-3d-helicopter-kit-align-t-rex-compat-ver-2.html) (T-rex clone)      | 39.06 | 1        |  39.06 |
| [Turnigy Typhoon 450H 2215H Heli Motor 3550kv](https://hobbyking.com/en_us/turnigy-typhoon-450h-2215h-heli-motor-3550kv-450-class.html)                        | 13.75 | 1        |  13.75 |
| [Hobbyking SS Series 25-30A ESC](https://hobbyking.com/en_us/hobbyking-ss-series-25-30a-esc.html)                                                              |  7.70 | 1        |   7.70 |
| [Corona 919MG Digital Metal Gear Servo](https://hobbyking.com/en_us/corona-919mg-digital-metal-gear-servo-1-7kg-0-06sec-12g.html)                              |  7.09 | 4        |  28.36 |
| [Antec NEO480 PSU](https://www.newegg.com/antec-neopower-480-480w/p/N82E16817103924) (or any spare PSU you have)                                               | 80.00 | 1        |  80.00 |
| [Electronics-Salon 24/20-pin ATX DC Power Supply Breakout Board Module](https://www.amazon.com/Electronics-Salon-20-pin-Supply-Breakout-Module/dp/B01NBU2C64/) | 12.99 | 1        |  12.99 |
| Hookup wire (various)                                                                                                                                          |  2.00 | 1        |   2.00 |
| ~~[ARCELI L293D NodeMCU motor shield](https://www.amazon.com/ESP8266-NodeMCU-2-Channel-H-Bridge-ESP-12E/dp/B07KF9K293)~~ - No longer used                      | 13.30 | 0        |   0.00 |
| Total                                                                                                                                                          |       |          | 199.84 |

ESP8266 boards commonly have two different widths between rows of pins: 0.9" (23mm) and 1.1" (28mm). When purchasing a NodeMCU board holder (or motor shield), ensure the width of the shield matches the width of your ESP.

Tailoring this code to your Electronic Speed Control (ESC) may require trial and error. I found this side helpful: <http://techvalleyprojects.blogspot.com/2012/06/arduino-control-escmotor-tutorial.html>

Perhaps the most important takeaway from that is to NEVER connect your Arduino power pins to an ESC!  Modern ESCs have a Battery Elimination Circuitry (BEC) that can feedback to the ESP8266. The ESC should be powered by an XT60 connector, and the ESC servo pins should connect to ground and signal, but the middle power pin should NOT be connected to anything!  The BEC eliminates the need for power on this pin.

Other important takeaways are that the motor may not start turning until a value such as 50 is used (~2000 ms PWM), and the motor may stop turning at non-zero values (like 20).

### Connections

- Cyclic/Collective Pitch Mixing (CCPM) Servo lead signal pins connected to GPIO4 (D2), GPIO14 (D5), and GPIO12 (D6). At least one servo will need to be reversed, which is done in software.
- ESC servo lead signal pin connected to GPIO5 (D1).
- Rudder servo lead signal pin connected to GPIO15 (D8).  The rudder is optional, but if omitted, the rudder should be constrained to a centered position.
- Connect black or brown servo wires to ground.
- Connect red servo wires to +5V, but do not connect the red ESC servo wire to anything!
- Connect white, orange, or yellow servo wires to signal.
- Ensure the ESC is provided enough amperage via the XT60 connector.
- Floodlight LEDs connected to a relay that is connected to GPIO14 (D5).
- TLOF (Touchdown LiftOFf area) circular LED string connected to a relay that is connected to GPIO13 (D7).
- FATO (Final Approach/Take Off) square LED string connected to a relay that is connected to GPIO13 (D7).

## Software Setup

### Libraries

- [**ESP8266WiFi.h**](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi) provides the network client for MQTT to use, and Wi-Fi functions used to connect to a local area
  network in station mode.
- [**PubSubClient.h**](https://github.com/knolleary/pubsubclient) by Nick O'Leary, provides the MQTT client. This program only uses MQTT to subscribe to a broker, it does not publish
  any messages at this time.
- [**Adafruit_PWMServoDriver**](https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library) provides the methods used to configure and control servos in software.
- [**ArduinoJson**](https://arduinojson.org/) by Benoît Blanchon, wrangles our data into and out of JSON format.

### Networking

- A MQTT broker that this device can subscribe to is required. I will not cover how to set up a MQTT broker. I have
  tested with Mosquitto and FairCom Edge. Both are simple to install and configure.
- A client will publish MQTT messages to the broker. Those messages will be sent this device, which will react to the
  message according to the API detailed below.
- The ESP8266 Wi-Fi module handles all IP layer communications.

### API

The API for this project is a simple JSON object of root-level keys and values.
Any absent keys will be ignored.

Key definitions:

| Property   | Description                                        | Type    | Limits (inclusive) | Default |
|------------|----------------------------------------------------|---------|--------------------|---------|
| killswitch | Sets all properties to default values              | Boolean | `true` `false`     | false   |
| throttle   | Controls the speed of the main and tail rotors     | Integer | `0` to `180`       | 0       |
| collective | Controls the pitch of the main rotor blades        | Integer | `0` to `180`       | 90      |
| rudder     | Controls the pitch of the rudder                   | Integer | `0` to `180`       | 90      |
| floodlight | Controls the illumination of the floodlights       | Boolean | `true` `false`     | false   |
| TLOF       | Controls the illumination of the green Touchdown LiftOFf lights | Boolean | `true` `false`     | false   |
| FATO       | Controls the illumination of the Final Approach/Take Off lights | Boolean | `true` `false`     | false   |

Sample JSON with all properties:

```
{
    "killswitch": false,
    "throttle": 0,
    "collective": 50,
    "rudder": 100,
    "floodlight": true,
    "TLOF": false,
    "FATO": true
}
```

The killswitch command stops the motor, centers the collective (no positive or negative pitch on the main rotor), and centers the tail rotor.

The three servos: throttle, collective, and rudder take a positive integer from 0 to 180 which represents the servo position.  A value of 90 centers the servo.

The three lights: floodlight, TLOF, and FATO take a boolean value to indicate the illumination.

TLOF (Touchdown LiftOFf area) green LEDs in a circle, connected to a relay that is connected to GPIO12 (D7).
FATO (Final Approach/Take Off) white LEDs in a square, connected to a relay that is connected to GPIO13 (D7).

The default topic of "mqttServo" is set with the 'mqttTopic' global constant in the networkVariables.h file.
The default MQTT broker address and port are also set using global constants in that file.

A sample message using the Mosquitto command line utility, which will turn on all lights, set the throttle to 25% (45), and center the collective (no pitch):

```mosquitto_pub -h 127.0.0.1 -p 1883 -i testPublish -t mqttServo -m "{"killswitch":false,"throttle":45,"collective":90,"rudder":90,"floodlight":true,"TLOF":true,"FATO":true}"```

No attempt is made to use QoS levels greater than 0. This sketch is only a subscriber, and makes no attempt to respond with QoS acknowledgements.
