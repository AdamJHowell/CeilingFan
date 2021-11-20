/**
 * Ceiling Fan
 * An Arduino controlled ceiling fan made from a R/C helicopter.
 * Create 2020-07-28
 * by Adam Howell
 * https://github.com/AdamJHowell/CeilingFan
 *
 * See the associated README.md for hardware and API information.
 * The safest pins to use for output with the ESP8266 are D1, D2, D5, D6, D7, and D8.
 * Note that the board will fail to boot if D8 is pulled high.
 * Note that the board will fail to boot if D3, D4, or TX are pulled low.
 * Note that pins D0, D4, RX, TX, SD2, and SD3 are high on boot.
 */


#include <Arduino.h>				// The built-in Arduino library.
#include <Servo.h>				// The built-in servo library.
#include <Wire.h>					// The built-in I2C library.
#include <ESP8266WiFi.h>		// Network Client for the WiFi chipset.  This is added when the 8266 is added in board manager: https://github.com/esp8266/Arduino
#include <PubSubClient.h>		// PubSub is the MQTT API maintained by Nick O'Leary: https://github.com/knolleary/pubsubclient
#include "Adafruit_PWMServoDriver.h"	// This is required to use the PCA9685 I2C PWM/Servo driver: https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library
#include "workNetworkVariables.h"	// I use this file to hide my network information from random people browsing my GitHub repo.


/**
 * Network data
 * If you do not use the networkVariables.h file to hold your network information, you will need to set these four consts to suit your needs.
 */
//const char* wifiSsid = "nunya";
//const char* wifiPassword = "nunya";
//const char* mqttBroker = "127.0.0.1";
//const int mqttPort = 1883;
const char* mqttTopic = "mqttServo";
char macAddress[18];
char clientAddress[16];


/*
 * Adafruit PWM settings for the PCA9685.
 */
// Called this way, it uses the default address 0x40.
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Depending on your servo make, the pulse width min and max may vary.
// Adjust these to be as small/large as possible without hitting the hard stop for max range.
#define SERVOMIN 150	 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX 600	 // This is the 'maximum' pulse length count (out of 4096)
#define USMIN 600		 // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
#define USMAX 2400	 // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

// The servo # counter.
uint8_t servonum = 0;


/**
 * ESP-8266 GPIO port assignments
 * Avoid using GPIO16 for servos with the ESP-8266.  It is an onboard LED, and seems to cause problems when hooked up to a servo.
 * Do not use GPIO1, GPIO6, GPIO7, GPIO8, or GPIO11.  I did not test beyond GPIO12.
 * Collective servos need to each be on unique GPIOs because at least one of them will be reversed.
 */
const int ESP12LED = 2;
const int MCULED = 16;
const int throttlePin = 5;			// Use GPIO5 (D1) for the throttle (ESC).
const int collective1Pin = 4;		// Use GPIO4 (D2) for collective1.
const int collective2Pin = 14;	// Use GPIO14 (D5) for collective2.
const int collective3Pin = 12;	// Use GPIO12 (D6) for collective3.
const int rudderPin = 15;			// Use GPIO15 (D8) for the rudder.
const int floodLEDPin = 13;		// Use GPIO13 (D7) for the floodlights.
const int tlofLEDPin = 13;			// Use GPIO13 (D7) for the green TLOF circle LEDs.
const int fatoLEDPin = 13;			// Use GPIO13 (D7) for the white FATO square LEDs.
// Misc values.
const int LED_ON = 1;
const int LED_OFF = 0;
const int escArmValue = 10; // The value to send to the ESC in order to "arm" it.
/**
 * Initial servo positions.
 */
int throttlePos = 0;
int rudderPos = 90;
int collective1Pos = 90;
int collective2Pos = 90;
int collective3Pos = 90;

WiFiClient espClient;					  // Create a WiFiClient to connect to the local network.
PubSubClient mqttClient( espClient ); // Create a PubSub MQTT client object that uses the WiFiClient.
Servo throttleServo;						  // Create servo object to control the ESC.
Servo rudderServo;						  // Create servo object to control the rudder.
Servo collective1Servo;					  // Create servo object to control one of the three collective servos.
Servo collective2Servo;					  // Create servo object to control one of the three collective servos.
Servo collective3Servo;					  // Create servo object to control one of the three collective servos.


/**
 * throttleChange() will handle the scaling and limits needed, and pass real-world values to moveServo().
 */
void throttleChange( int receivedValue )
{
	// Multiply by 20 to scale from 1-9 up to 1-180.
	throttleServo.write( receivedValue * 20 );
} // End of throttleChange() function.


/**
 * collectiveChange() will handle the scaling and limits needed, and pass real-world values to moveServo().
 */
void collectiveChange( int receivedValue )
{
	int finalValue = receivedValue * 20;
	// ToDo: These next 3 lines may need significant tweaking of finalValue before calling moveServo().
	// This is because the mechanical linkage lengths and servo arm positions vary.
	// At least one servo will need to send an inverted finalValue (180 - finalValue).
	collective1Servo.write( finalValue );
	collective2Servo.write( finalValue );
	collective3Servo.write( finalValue );
} // End of collectiveChange() function.


/**
 * rudderChange() will handle the scaling and limits needed, and pass real-world values to moveServo().
 */
void rudderChange( int receivedValue )
{
	// Multiply by 20 to scale from 1-9 up to 1-180.
	rudderServo.write( receivedValue * 20 );
} // End of rudderChange() function.


/**
 * floodLightChange() will toggle the floodlights on or off.
 */
void floodLightChange( int receivedValue )
{
	if( receivedValue == 0 )
		digitalWrite( floodLEDPin, LED_OFF ); // Turn the LED off.
	else
	{
		digitalWrite( floodLEDPin, LED_ON ); // Turn the LED on.
		digitalWrite( MCULED, LED_ON );		 // Turn the LED on.
		digitalWrite( ESP12LED, LED_ON );	 // Turn the LED on.
	}
} // End of floodLightChange() function.


/**
 * tlofLightChange() will toggle the Touchdown Liftoff lights on or off.
 */
void tlofLightChange( int receivedValue )
{
	// Note that some boards consider 'HIGH' to be off.
	if( receivedValue == 0 )
		digitalWrite( tlofLEDPin, LED_OFF ); // Turn the LED off.
	else
		digitalWrite( tlofLEDPin, LED_ON ); // Turn the LED on.
} // End of tlofLightChange() function.


/**
 * fatoLightChange() will toggle the Final Approach Liftoff lights on or off.
 */
void fatoLightChange( int receivedValue )
{
	// Note that some boards consider 'HIGH' to be off.
	if( receivedValue == 0 )
		digitalWrite( fatoLEDPin, LED_OFF ); // Turn the LED off.
	else
		digitalWrite( fatoLEDPin, LED_ON ); // Turn the LED on.
} // End of fatoLightChange() function.


/**
 * killSwitch() will turn everything off.
 */
void killSwitch()
{
	Serial.println( "\nKill switch!\n" );
	// Turn the ESC off.
	throttleServo.write( throttlePos );
	// Turn the LED on.
	digitalWrite( floodLEDPin, LED_OFF );
	// Turn the LED on.
	digitalWrite( tlofLEDPin, LED_OFF );
	// Turn the LED on.
	digitalWrite( fatoLEDPin, LED_OFF );
	// Center the rudder servo and collective servos.
	rudderServo.write( rudderPos );
	collective1Servo.write( collective1Pos );
	collective2Servo.write( collective2Pos );
	collective3Servo.write( collective3Pos );
	delay( 1000 );
} // End of killSwitch() function.


/**
 * callback() handles MQTT subscriptions.
 * When a message comes in on a topic we have subscribed to, this function is executed.
 */
void callback( char* topic, byte* payload, unsigned int length )
{
	Serial.println();
	Serial.print( "Message arrived [" );
	Serial.print( topic );
	Serial.print( "] " );
	for( int i = 0; i < length; i++ )
	{
		char receivedKey = ( char )payload[i];
		Serial.println( receivedKey );

		if( receivedKey == 't' ) // Process throttle changes.
		{
			if( length > 1 )
			{
				// Store the 0-9 value in receivedValue.
				char receivedValue = ( char )payload[i + 1];
				// Increment the index since it was consumed.
				i++;
				// Convert the ASCII value to decimal.
				throttleChange( receivedValue - '0' );
			}
		}
		else if( receivedKey == 'c' ) // Process collective changes.
		{
			if( length > 1 )
			{
				// Store the 1-9 value in receivedValue.
				char receivedValue = ( char )payload[i + 1];
				// Increment the index since it was consumed.
				i++;
				// Convert the ASCII value to decimal.
				collectiveChange( receivedValue - '0' );
			}
		}
		else if( receivedKey == 'r' ) // Process rudder changes.
		{
			if( length > 1 )
			{
				// Store the 1-9 value in receivedValue.
				char receivedValue = ( char )payload[i + 1];
				// Increment the index since it was consumed.
				i++;
				// Convert the ASCII value to decimal.
				rudderChange( receivedValue - '0' );
			}
		}
		else if( receivedKey == 'f' ) // Process floodlight changes.
		{
			if( length > 1 )
			{
				// Store the 0-1 value in receivedValue.
				char receivedValue = ( char )payload[i + 1];
				// Increment the index since it was consumed.
				i++;
				// Convert the ASCII value to decimal.
				floodLightChange( receivedValue - '0' );
			}
		}
		else if( receivedKey == 'l' ) // Process green TLOF circle LED changes.
		{
			if( length > 1 )
			{
				// Store the 0-1 value in receivedValue.
				char receivedValue = ( char )payload[i + 1];
				// Increment the index since it was consumed.
				i++;
				// Convert the ASCII value to decimal.
				tlofLightChange( receivedValue - '0' );
			}
		}
		else if( receivedKey == 'a' ) // Process white FATO square LED changes.
		{
			if( length > 1 )
			{
				// Store the 0-1 value in receivedValue.
				char receivedValue = ( char )payload[i + 1];
				// Increment the index since it was consumed.
				i++;
				// Convert the ASCII value to decimal.
				fatoLightChange( receivedValue - '0' );
			}
		}
		else if( receivedKey == 'c' ) // Process collective changes.
		{
			if( length > 1 )
			{
				// Store the 1-9 value in receivedValue.
				char receivedValue = ( char )payload[i + 1];
				// Increment the index since it was consumed.
				i++;
				// Convert the ASCII value to decimal.
				collectiveChange( receivedValue - '0' );
			}
		}
		else if( receivedKey == 'k' ) // Kill switch!
		{
			// Turn everything off.
			killSwitch();
		}
	}
} // End of callback() function.


/**
 * mqttConnect() will attempt to (re)connect the MQTT client.
 */
void mqttConnect()
{
	// Loop until MQTT has connected.
	while( !mqttClient.connected() )
	{
		Serial.print( "Connecting to MQTT broker at " );
    Serial.print( mqttBroker );
    Serial.print( ":" );
    Serial.print( mqttPort );
		if( mqttClient.connect( "ESP8266 Client" ) ) // Attempt to mqttConnect using the designated clientID.
		{
			Serial.println( "connected" );
			// Subscribe to the designated MQTT topic.
			if( mqttClient.subscribe( mqttTopic ) )
			{
				Serial.print( "Subscribed to topic \"" );
				Serial.print( mqttTopic );
				Serial.println( "\"\n" );
			}
			else
			{
				Serial.println( "MQTT topic subscription failed!" );
			}
		}
		else
		{
			Serial.print( " failed, return code: " );
			Serial.print( mqttClient.state() );
			Serial.println( ", will try again in 2 seconds" );
			// Wait 2 seconds before retrying.
			delay( 2000 );
		}
	}
	Serial.print( "MQTT is connected to " );
	Serial.println( mqttBroker );
} // End of mqttConnect() function.


/**
 * The setup() function runs once when the device is booted, and then loop() takes over.
 */
void setup()
{
	// Start the Serial communication to send messages to the computer.
	Serial.begin( 115200 );
	delay( 10 );
	Serial.println( '\n' );

	// Initiate the PCA9685.
	pwm.begin();
	pwm.setOscillatorFrequency( 27000000 );
	pwm.setPWMFreq( SERVO_FREQ ); // Analog servos run at ~50 Hz updates.

	// Attach the throttle servo to the appropriate pin.
	throttleServo.attach( throttlePin );
	// Attach the rudder servo to the appropriate pin.
	rudderServo.attach( rudderPin );
	// Attach the collective1 servo to the appropriate pin.
	collective1Servo.attach( collective1Pin );
	// Attach the collective2 servo to the appropriate pin.
	collective2Servo.attach( collective2Pin );
	// Attach the collective3 servo to the appropriate pin.
	collective3Servo.attach( collective3Pin );
	// Set the throttle to zero and all servos to their default positions.
	killSwitch();
	// Arm the ESC
	throttleServo.write( escArmValue );

	// Set the MQTT client parameters.
	mqttClient.setServer( mqttBroker, mqttPort );
	// Assign the callback() function to handle MQTT callbacks.
	mqttClient.setCallback( callback );

	// Initialize the floodlight pin as an output.
	pinMode( floodLEDPin, OUTPUT );
	// Initialize the TLOF pin as an output.
	pinMode( tlofLEDPin, OUTPUT );
	// Initialize the FATO pin as an output.
	pinMode( fatoLEDPin, OUTPUT );

	// Connect to the WiFi network.
	Serial.printf( "Wi-Fi mode set to WIFI_STA %s\n", WiFi.mode( WIFI_STA ) ? "" : "Failed!" );
	WiFi.begin( wifiSsid, wifiPassword );
	Serial.print( "WiFi connecting to " );
	Serial.println( wifiSsid );

	int i = 0;
	/*
	WiFi.status() return values:
	0 : WL_IDLE_STATUS when WiFi is in process of changing between statuses
	1 : WL_NO_SSID_AVAIL in case configured SSID cannot be reached
	3 : WL_CONNECTED after successful connection is established
	4 : WL_CONNECT_FAILED if password is incorrect
	6 : WL_DISCONNECTED if module is not configured in station mode
  */
	while( WiFi.status() != WL_CONNECTED ) // Wait for the WiFi to connect.
	{
		delay( 1000 );
		Serial.println( "Waiting for a connection..." );
		Serial.print( "WiFi status: " );
		Serial.println( WiFi.status() );
		Serial.print( ++i );
		Serial.println( " seconds" );
	}

	// Print that WiFi has connected.
	Serial.println( '\n' );
	Serial.println( "WiFi connection established!" );
	snprintf( macAddress, 18, "%s", WiFi.macAddress().c_str() );
	Serial.print( "MAC address: " );
	Serial.println( macAddress );
	Serial.print( "IP address: " );
	snprintf( clientAddress, 16, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
	Serial.println( clientAddress );
} // End of setup() function.


/**
 * The loop() function begins after setup(), and repeats as long as the unit is powered.
 */
void loop()
{
	if( !mqttClient.connected() )
	{
		Serial.println( "Lost connection to the MQTT broker." );
		mqttConnect();
	}
	// Drive each servo one at a time using setPWM()
	Serial.println( servonum );
	for( uint16_t pulselen = SERVOMIN; pulselen < SERVOMAX; pulselen++ )
	{
		pwm.setPWM( servonum, 0, pulselen );
	}

	delay( 100 );
	for( uint16_t pulselen = SERVOMAX; pulselen > SERVOMIN; pulselen-- )
	{
		pwm.setPWM( servonum, 0, pulselen );
	}

	// Drive each servo one at a time using writeMicroseconds(), it's not precise due to calculation rounding!
	// The writeMicroseconds() function is used to mimic the Arduino Servo library writeMicroseconds() behavior.
	for( uint16_t microsec = USMIN; microsec < USMAX; microsec++ )
	{
		pwm.writeMicroseconds( servonum, microsec );
	}

	delay( 100 );
	for( uint16_t microsec = USMAX; microsec > USMIN; microsec-- )
	{
		pwm.writeMicroseconds( servonum, microsec );
	}

	servonum++;
	if( servonum > 7 ) servonum = 0; // Testing the first 8 servo channels.

	mqttClient.loop();
} // End of loop() function.
