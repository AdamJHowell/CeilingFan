/**
 * Ceiling Fan
 * A ceiling fan made from a R/C helicopter.
 * Create 2020-07-28
 * by Adam Howell
 * https://github.com/AdamJHowell/CeilingFan
 *
 * See the associated README.md for hardware and API information.
 */

#include <ESP8266WiFi.h>  // Network Client for the WiFi chipset.
#include <PubSubClient.h> // PubSub is the MQTT API.
#include <Servo.h>


// WiFi and MQTT constants.
const char* ssid = "Red";
const char* password = "8012254722";
const char* mqttServer = "192.168.55.200";
const int mqttPort = 2112;
const char* mqttTopic = "mqttServo";
const int throttlePin = 16;	// Use GPIO16 (D0) for the throttle (ESC).
const int rudderPin = 5;		// Use GPIO5 (D1) for the rudder.
const int collective1Pin = 4; // Use GPIO4 (D2) for the collective1.
const int collective2Pin = 0; // Use GPIO0 (D3) for the collective2.
const int collective3Pin = 2; // Use GPIO2 (D4) for the collective3.
const int floodLEDPin = 1;		// Use GPIO1 () for the floodlights.
const int tlofLEDPin = 3;		// Use GPIO3 () for the green TLOF circle LEDs.
const int fatoLEDPin = 15;		// Use GPIO15 () for the white FATO square LEDs.
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
 * moveServo() reads the current position, and gradually moves the servo to the new position.
 */
int moveServo( Servo thisServo, int curPos, int newPos )
{
	if( newPos < 0 ) // Check for an invalid value.
	{
		Serial.print( "moveServo() was given an invalid servo position: " );
		Serial.println( newPos );
		newPos = 5; // Set the servo to a low, but valid, position.
	}
	else if( newPos > 180 ) // Check for an invalid value.
	{
		Serial.print( "moveServo() was given an invalid servo position: " );
		Serial.println( newPos );
		newPos = 175; // Set the servo to a low, but valid, position.
	}
	else
	{
		Serial.print( "\nMoving from " );
		Serial.print( curPos );
		Serial.print( " to " );
		Serial.println( newPos );
		if( curPos < newPos )
		{
			for( ; curPos < newPos; curPos++ )
			{
				thisServo.write( curPos );
				delay( 15 );
			}
		}
		else
		{
			for( ; curPos > newPos; curPos-- )
			{
				thisServo.write( curPos );
				delay( 15 );
			}
		}
	}
	delay( 100 ); // This delay may need to be tweaked, and may vary from servo to servo.
	return newPos;
} // End of moveServo() function.


/**
 * throttleChange() will handle the scaling and limits needed, and pass real-world values to
 * moveServo().
 */
void throttleChange( int receivedValue )
{
	int finalValue = receivedValue * 20; // Multiply by 20 to scale from 1-9 up to 1-180.
	throttlePos = moveServo( throttleServo, throttlePos, finalValue );
} // End of throttleChange() function.


/**
 * collectiveChange() will handle the scaling and limits needed, and pass real-world values to
 * moveServo().
 */
void collectiveChange( int receivedValue )
{
	int finalValue = receivedValue * 20;
	// ToDo: These next 3 lines may need significant tweaking of finalValue before calling
	// moveServo(). Some servos will need finalValue inverted (180 - finalValue). These moves happen
	// sequentially (1, 2, 3), not simultaneously.
	collective1Pos = moveServo( collective1Servo, collective1Pos, finalValue );
	collective2Pos = moveServo( collective2Servo, collective2Pos, finalValue );
	collective3Pos = moveServo( collective3Servo, collective3Pos, finalValue );
} // End of collectiveChange() function.


/**
 * rudderChange() will handle the scaling and limits needed, and pass real-world values to
 * moveServo().
 */
void rudderChange( int receivedValue )
{
	int finalValue = receivedValue * 20;
	rudderPos = moveServo( rudderServo, rudderPos, finalValue );
} // End of rudderChange() function.


/**
 * floodLightChange() will toggle the floodlights on or off.
 */
void floodLightChange( int receivedValue )
{
	// Note that some boards consider 'HIGH' to be off.
	if( receivedValue == 0 )
		digitalWrite( floodLEDPin, HIGH ); // Turn the LED off.
	else
		digitalWrite( floodLEDPin, LOW ); // Turn the LED on.
} // End of floodLightChange() function.


/**
 * tlofLightChange() will toggle the Touchdown Liftoff lights on or off.
 */
void tlofLightChange( int receivedValue )
{
	// Note that some boards consider 'HIGH' to be off.
	if( receivedValue == 0 )
		digitalWrite( tlofLEDPin, HIGH ); // Turn the LED off.
	else
		digitalWrite( tlofLEDPin, LOW ); // Turn the LED on.
} // End of tlofLightChange() function.


/**
 * fatoLightChange() will toggle the Final Approach Liftoff lights on or off.
 */
void fatoLightChange( int receivedValue )
{
	// Note that some boards consider 'HIGH' to be off.
	if( receivedValue == 0 )
		digitalWrite( fatoLEDPin, HIGH ); // Turn the LED off.
	else
		digitalWrite( fatoLEDPin, LOW ); // Turn the LED on.
} // End of fatoLightChange() function.


/**
 * killSwitch() will turn everything off.
 */
void killSwitch()
{
	// Turn the ESC on.
	throttlePos = moveServo( throttleServo, throttlePos, 0 );
	// Turn the LED on.
	digitalWrite( floodLEDPin, LOW );
	// Turn the LED on.
	digitalWrite( tlofLEDPin, LOW );
	// Turn the LED on.
	digitalWrite( fatoLEDPin, LOW );
	// Center the rudder servo.
	rudderPos = moveServo( rudderServo, rudderPos, 90 );
	// Center the collective servos.
	collective1Pos = moveServo( collective1Servo, collective1Pos, 90 );
	collective2Pos = moveServo( collective2Servo, collective2Pos, 90 );
	collective3Pos = moveServo( collective3Servo, collective3Pos, 90 );
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
		Serial.print( receivedKey );

		if( receivedKey == 't' ) // Process throttle changes.
		{
			if( length > 1 )
			{
				// Store the 0-9 value in receivedValue.
				char receivedValue = ( char )payload[i + 1];
				// Increment the index since it was consumed.
				i++;
				// Send the 0-9 value.
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
				// Send the 1-9 value.
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
				// Send the 1-9 value.
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
				// Send the 0-1 value.
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
				// Send the 0-1 value.
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
				// Send the 0-1 value.
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
				// Send the 1-9 value.
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
 * reconnect() will attempt to reconnect the MQTT and WiFi clients.
 */
void reconnect()
{
	// Loop until we're reconnected.
	while( !mqttClient.connected() )
	{
		Serial.print( "Attempting MQTT connection..." );
		// Attempt to connect using the designated clientID.
		if( mqttClient.connect( "ESP8266 Client" ) )
		{
			Serial.println( "connected" );
			// Subscribe to the designated MQTT topic.
			mqttClient.subscribe( mqttTopic );
		}
		else
		{
			Serial.print( " failed, return code: " );
			Serial.print( mqttClient.state() );
			Serial.println( " try again in 5 seconds" );
			// Wait 5 seconds before retrying.
			delay( 5000 );
		}
	}
} // End of reconnect() function.


/**
 * The setup() function runs once when the device is booted, and then loop() takes over.
 */
void setup()
{
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
	// Set the throttle to zero.
	throttleServo.write( throttlePos );
	// Move the servo to its center position.
	rudderServo.write( rudderPos );
	// Move the collective to neutral.
	collective1Servo.write( collective1Pos );
	// Move the collective to neutral.
	collective2Servo.write( collective2Pos );
	// Move the collective to neutral.
	collective3Servo.write( collective3Pos );
	// Start the Serial communication to send messages to the computer.
	Serial.begin( 115200 );
	delay( 10 );
	Serial.println();
	Serial.println();

	// Set the MQTT client parameters.
	mqttClient.setServer( mqttServer, mqttPort );
	// Assign the callback() function to handle MQTT callbacks.
	mqttClient.setCallback( callback );

	// Initialize the floodlight pin as an output.
	pinMode( floodLEDPin, OUTPUT );
	// Initialize the TLOF pin as an output.
	pinMode( tlofLEDPin, OUTPUT );
	// Initialize the FATO pin as an output.
	pinMode( fatoLEDPin, OUTPUT );

	Serial.printf( "Wi-Fi mode set to WIFI_STA %s\n", WiFi.mode( WIFI_STA ) ? "" : "Failed!" );
	// Connect to the WiFi network.
	WiFi.begin( ssid, password );
	Serial.print( "WiFi connecting to " );
	Serial.println( ssid );

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

	Serial.println( '\n' );
	Serial.println( "Connection established!" );
	Serial.print( "IP address:\t" );
	// Send the IP address of the ESP8266 to the serial port.
	Serial.println( WiFi.localIP() );
} // End of setup() function.


/**
 * The loop() function begins after setup(), and repeats as long as the unit is powered.
 */
void loop()
{
	if( !mqttClient.connected() )
	{
		reconnect();
	}
	mqttClient.loop();
} // End of loop() function.
