/**
 * Created by Adam on 2022-08-30.
 */

#ifndef CEILINGFAN_CEILINGFAN_H
#define CEILINGFAN_CEILINGFAN_H

#include <Arduino.h>						 // The built-in Arduino library.
#include "Adafruit_PWMServoDriver.h" // This is required to use the PCA9685 I2C PWM/Servo driver: https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library
#include "privateInfo.h"				 // I use this file to hide my network information from random people on GitHub.
#include <ESP8266WiFi.h>				 // Network Client for the Wi-Fi chipset.  This is added when the 8266 is added in board manager: https://github.com/esp8266/Arduino
#include <ESP8266mDNS.h>				 // OTA - mDNSResponder (Multicast DNS) for the ESP8266 family.
#include <WiFiUdp.h>						 // OTA
#include <ArduinoOTA.h>					 // OTA - The Arduino OTA library.  Specific version of this are installed along with specific boards in board manager.
#include <ArduinoJson.h>				 // A JSON processing library.  Author: Benoît Blanchon  https://arduinojson.org/
#include <PubSubClient.h>				 // PubSub is the MQTT API maintained by Nick O'Leary: https://github.com/knolleary/pubsubclient
#include <Servo.h>						 // The built-in servo library.
#include <Wire.h>							 // The built-in I2C library.
#include <WiFiClient.h>					 // Provides the WiFiClient class needed for MQTT.
#include "SuperServo.h"					 // My upgraded Servo class.


/**
 * Servo parameters.
 * Depending on your servo make, the pulse width min and max may vary.
 * Adjust these to be as small/large as possible without hitting the hard stop for max range.
 */
#define SERVO_MIN	 150	// This is the 'minimum' pulse length count (out of 4096)
#define SERVO_MAX	 600	// This is the 'maximum' pulse length count (out of 4096)
#define US_MIN		 600	// This is the rounded 'minimum' microseconds length based on the minimum pulse of 150
#define US_MAX		 2400 // This is the rounded 'maximum' microseconds length based on the maximum pulse of 600
#define SERVO_FREQ 50	// Analog servos run at ~50 Hz updates
/**
 * PCA9685 assignments.
 */
#define C1			 1 // PCA9685 Servo header 1 will control collective servo 1
#define C2			 2 // PCA9685 Servo header 2 will control collective servo 2
#define C3			 3 // PCA9685 Servo header 3 will control collective servo 3
#define THROTTLE	 4 // PCA9685 Servo header 4 will control the throttle
#define RUDDER		 5 // PCA9685 Servo header 5 will control the rudder
/**
 * ESP8266 GPIO assignments.
 */
#define TDPC		 12 // GPIO 12 (D6) controls the green Touchdown Positioning Circle LEDs
#define TLOF		 13 // GPIO 13 (D7) controls the white Touchdown Liftoff area LEDs
#define FLOOD		 14 // GPIO 14 (D5) controls the Floodlights


/**
 * Network data
 * If you do not use the networkVariables.h file to hold your network information, you will need to set these four consts to suit your needs.
 */
//const char *wifiSsid = "nunya";
//const char *wifiPassword = "nunya";
//const char *mqttBroker = "127.0.0.1";
//const int mqttPort = 1883;
const char *HOST_NAME = "CeilingFan";					  // The hostname used for OTA access.
const char *SKETCH_NAME = "CeilingFan.ino";			  // The name used when publishing stats.
const char *NOTES = "The helicopter ceiling fan";	  // The hostname used for OTA access.
const char *MQTT_STATS_TOPIC = "heli/stats";			  // The topic this device will publish to upon connection to the broker.
const char *MQTT_COMMAND_TOPIC = "heli/commands";	  // The command topic this device will respond to.
const char *sketchTopic = "heli/sketch";				  // The topic used to publish the sketch name (__FILE__).
const char *macTopic = "heli/mac";						  // The topic used to publish the MAC address.
const char *ipTopic = "heli/ip";							  // The topic used to publish the IP address.
const char *rssiTopic = "heli/rssi";					  // The topic used to publish the WiFi Received Signal Strength Indicator.
const char *publishCountTopic = "heli/publishCount"; // The topic used to publish the loop count.
const char *notesTopic = "heli/notes";					  // The topic used to publish notes relevant to this project.
const char *mqttTopic = "espWeather";					  // The topic used to publish a single JSON message containing all data.
const unsigned long JSON_DOC_SIZE = 512;				  // The ArduinoJson document size, and size of some buffers.
char ipAddress[16];											  // A character array to hold the IP address.
char macAddress[18];											  // A character array to hold the MAC address, and append a dash and 3 numbers.
long rssi;														  // A global to hold the Received Signal Strength Indicator.
unsigned int networkIndex = 2112;						  // An unsigned integer to hold the correct index for the network arrays: wifiSsidArray[], wifiPassArray[], mqttBrokerArray[], and mqttPortArray[].
unsigned int callbackCount = 0;							  // The number of times a callback was received.
unsigned int wifiConnectionTimeout = 10000;			  // Set the Wi-Fi connection timeout to 10 seconds.
unsigned int mqttReconnectInterval = 3000;			  // Set the delay between MQTT broker connection attempts to 3 seconds.
unsigned long publishCount = 0;							  // A counter of how many times the stats have been published.
unsigned long lastPollTime = 0;
unsigned long lastPublishTime = 0;
unsigned int sensorPollDelay = 10000;
unsigned int publishDelay = 60000;


/**
 * ESP-8266 GPIO port assignments
 * Avoid using GPIO16 for servos with the ESP-8266.  It is an onboard LED, and seems to cause problems when hooked up to a servo.
 * Do not use GPIO1, GPIO6, GPIO7, GPIO8, or GPIO11.  I did not test beyond GPIO12.
 * Collective servos need to each be on unique GPIOs because at least one of them will be reversed.
 * More info: https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
 * GPIO16: pin is high at BOOT
 * GPIO0: boot failure if pulled LOW
 * GPIO2: pin is high on BOOT, boot failure if pulled LOW
 * GPIO15: boot failure if pulled HIGH
 * GPIO3: pin is high at BOOT
 * GPIO1: pin is high at BOOT, boot failure if pulled LOW
 * GPIO10: pin is high at BOOT
 * GPIO9: pin is high at BOOT
 * GPIO4 and GPIO5 are the most safe to use GPIOs if you want to operate relays.
 */
const int MCU_LED = 2;			 // Boot fails if pulled low!
const int ESP12LED = 16;		 // Pin is high at boot.
const int TLOF_LED_PIN = 12;	 // Use GPIO12 (D7) for the green TLOF circle LEDs.
const int FATO_LED_PIN = 13;	 // Use GPIO13 (D7) for the white FATO square LEDs.
const int FLOOD_LED_PIN = 14;	 // Use GPIO14 (D5) for the floodlights.
const int throttlePin = 5;		 // Use GPIO5 (D1) for the throttle (ESC).
const int collective1Pin = 4;	 // Use GPIO4 (D2) for collective1.
const int collective2Pin = 14; // Use GPIO14 (D5) for collective2.
const int collective3Pin = 12; // Use GPIO12 (D6) for collective3.
const int rudderPin = 15;		 // Use GPIO15 (D8) for the rudder.
// Misc values.
const int LED_ON = 1;
const int LED_OFF = 0;

/**
 * Initial servo positions.
 */
int throttlePos = 0;
const int escArmValue = 10; // The value to send to the ESC in order to "arm" it.
int rudderPos = 90;
int collective1Pos = 90;
int collective2Pos = 90;
int collective3Pos = 90;

/**
 * Other Globals
 */
uint8_t servoNumber = 0; // The servo # counter.

/**
 * Class objects.
 */
Adafruit_PWMServoDriver pca9685 = Adafruit_PWMServoDriver(); // Called this way, it uses the default address of 0x40.
struct WiFiClient espClient;											 // Create a WiFiClient to connect to the local network.
PubSubClient mqttClient( espClient );								 // Create a PubSub MQTT client object that uses the WiFiClient.
Servo throttleServo;														 // Create servo object to control the ESC.
Servo rudderServo;														 // Create servo object to control the rudder.
Servo collective1Servo;													 // Create servo object to control one of the three collective servos.
Servo collective2Servo;													 // Create servo object to control one of the three collective servos.
Servo collective3Servo;													 // Create servo object to control one of the three collective servos.


/**
 * Function declarations.
 */
void throttleChange( int receivedValue );
void collectiveChange( int receivedValue );
void rudderChange( int receivedValue );
void floodLightChange( int receivedValue );
void tlofLightChange( int receivedValue );
void fatoLightChange( int receivedValue );
void killSwitch();
void onReceiveCallback( char *topic, byte *payload, unsigned int length );
void configureOTA();
void wifiMultiConnect();
int checkForSSID( const char *ssidName );
int mqttMultiConnect( int maxAttempts );
void publishStats();
void readTelemetry();
void printTelemetry();

#endif //CEILINGFAN_CEILINGFAN_H
