/**
 * Ceiling Fan
 * @brief An Arduino controlled ceiling fan made from a R/C helicopter.
 * Created 2020-07-28
 * by Adam Howell
 * https://github.com/AdamJHowell/CeilingFan
 *
 * See the associated README.md for hardware and API information.
 * The safest pins to use for output with the ESP8266 are D1, D2, D5, D6, D7, and D8.
 * Note that the board will fail to boot if D8 is pulled high.
 * Note that the board will fail to boot if D3, D4, or TX are pulled low.
 * Note that pins D0, D4, RX, TX, SD2, and SD3 are high on boot.
 */
#include "CeilingFan.h"


/**
 * The setup() function runs once when the device is booted, and then loop() takes over.
 */
void setup()
{
	// Start the Serial communication to send messages to the computer.
	Serial.begin( 115200 );
	while( !Serial )
		delay( 500 );

	Serial.println( "Setup is initializing the I2C bus." );
	Wire.begin();

	Serial.println( __FILE__ );

	// Initiate the PCA9685.
	pca9685.begin();
	pca9685.setOscillatorFrequency( 27000000 );
	pca9685.setPWMFreq( SERVO_FREQ ); // Analog servos run at ~50 Hz updates.

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

	wifiMultiConnect();
	configureOTA();

	// The networkIndex variable is initialized to 2112.  If it is still 2112 at this point, then WiFi failed to connect.
	if( networkIndex != 2112 )
	{
		const char *mqttBroker = mqttBrokerArray[networkIndex];
		const int mqttPort = mqttPortArray[networkIndex];
		// Set the MQTT client parameters.
		mqttClient.setServer( mqttBroker, mqttPort );
		// Assign the onReceiveCallback() function to handle MQTT callbacks.
		mqttClient.setCallback( onReceiveCallback );
		Serial.print( "Using MQTT broker: " );
		Serial.println( mqttBroker );
		Serial.print( "Using MQTT port: " );
		Serial.println( mqttPort );
	}

	// Initialize the floodlight pin as an output.
	pinMode( FLOOD_LED_PIN, OUTPUT );
	// Initialize the TLOF pin as an output.
	pinMode( TLOF_LED_PIN, OUTPUT );
	// Initialize the FATO pin as an output.
	pinMode( FATO_LED_PIN, OUTPUT );

	snprintf( macAddress, 18, "%s", WiFi.macAddress().c_str() );
	Serial.printf( "MAC address: %s\n", macAddress );
	Serial.print( "IP address: " );
	snprintf( ipAddress, 16, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3] );
	Serial.println( ipAddress );

	readTelemetry();
	printTelemetry();

	Serial.println( "Function setup() has completed.\n\n" );
} // End of setup() function.


/**
 * This is meant to query any data available to the device.
 */
void readTelemetry()
{
	rssi = WiFi.RSSI();
}


void printTelemetry()
{
	Serial.println();
	Serial.printf( "Sketch: %s\n", SKETCH_NAME );
	Serial.printf( "File: %s\n", __FILE__ );
	Serial.printf( "Notes: %s\n", NOTES );
	Serial.printf( "Wi-Fi SSID: %s\n", wifiSsidArray[networkIndex] );
	Serial.printf( "Wi-Fi RSSI: %ld\n", rssi );
	Serial.printf( "IP address: %s\n", ipAddress );
	Serial.printf( "MAC address: %s\n", macAddress );
	Serial.printf( "Broker: %s:%d\n", mqttBrokerArray[networkIndex], mqttPortArray[networkIndex] );
	Serial.printf( "Publish count: %lu\n", publishCount );
	Serial.printf( "Callback count: %u\n", callbackCount );
	Serial.printf( "MQTT stats topic: %s\n", MQTT_STATS_TOPIC );
	Serial.printf( "MQTT command topic: %s\n", MQTT_COMMAND_TOPIC );
	Serial.printf( "TLOF lights: %d\n", digitalRead( TLOF_LED_PIN ) );
	Serial.printf( "FATO lights: %d\n", digitalRead( FATO_LED_PIN ) );
	Serial.printf( "Floodlights: %d\n", digitalRead( FLOOD_LED_PIN ) );
	Serial.println();
}


/*
 * publishTelemetry() will publish the sensor and device data over MQTT.
 */
void publishTelemetry()
{
	// ToDo: Include all servo positions and all LED states!
	// ToDo: Include everything from printTelemetry()

	// Create a JSON Document on the stack.
	StaticJsonDocument<JSON_DOC_SIZE> publishDocument;
	// Add data: __FILE__, macAddress, ipAddress, tempC, tempF, humidity, rssi, publishCount, notes
	publishDocument["sketch"] = __FILE__;
	publishDocument["mac"] = macAddress;
	publishDocument["ip"] = ipAddress;
	publishDocument["rssi"] = rssi;
	publishDocument["publishCount"] = publishCount;

	// Prepare a String to hold the JSON.
	char mqttString[JSON_DOC_SIZE];
	// Serialize the JSON into mqttString, with indentation and line breaks.
	serializeJsonPretty( publishDocument, mqttString );
	// Publish the JSON to the MQTT broker.
	bool success = mqttClient.publish( MQTT_STATS_TOPIC, mqttString, false );
	if( success )
	{
		publishCount++;
		Serial.println( "Successfully published to:" );
		char buffer[20];
		// New format: <location>/<device>/<sensor>/<metric>
		if( mqttClient.publish( sketchTopic, __FILE__, false ) )
			Serial.printf( "  %s\n", sketchTopic );
		if( mqttClient.publish( macTopic, macAddress, false ) )
			Serial.printf( "  %s\n", macTopic );
		if( mqttClient.publish( ipTopic, ipAddress, false ) )
			Serial.printf( "  %s\n", ipTopic );
		ltoa( rssi, buffer, 10 );
		if( mqttClient.publish( rssiTopic, buffer, false ) )
			Serial.printf( "  %s\n", rssiTopic );
		snprintf( buffer, 11, "%lu", publishCount );
		if( mqttClient.publish( publishCountTopic, buffer, false ) )
			Serial.printf( "  %s\n", publishCountTopic );
		if( mqttClient.publish( notesTopic, NOTES, false ) )
			Serial.printf( "  %s\n", notesTopic );

		Serial.printf( "Successfully published to '%s', this JSON:\n", MQTT_STATS_TOPIC );
	}
	else
		Serial.println( "MQTT publish failed!  Attempted to publish this JSON to the broker:" );
	// Print the JSON to the Serial port.
	Serial.println( mqttString );
	lastPublishTime = millis();
} // End of publishTelemetry() function.


/**
 * The loop() function begins after setup(), and repeats as long as the unit is powered.
 */
void loop()
{
	ArduinoOTA.handle();
	if( !mqttClient.connected() )
		mqttMultiConnect( 5 );
	// The loop() function facilitates the receiving of messages and maintains the connection to the broker.
	mqttClient.loop();

	unsigned long time = millis();
	if( lastPollTime == 0 || ( ( time > sensorPollDelay ) && ( time - sensorPollDelay ) > lastPollTime ) )
	{
		readTelemetry();
		printTelemetry();
		lastPollTime = millis();
	}
	time = millis();
	if( lastPublishTime == 0 || ( ( time > publishDelay ) && ( time - publishDelay ) > lastPublishTime ) )
	{
		publishTelemetry();
		lastPublishTime = millis();
	}

	// Drive each servo one at a time using setPWM()
	Serial.printf( "Servo number %d\n", servoNumber );
	for( uint16_t pulseLength = SERVO_MIN; pulseLength < SERVO_MAX; pulseLength++ )
		pca9685.setPWM( servoNumber, 0, pulseLength );

	delay( 100 );
	for( uint16_t pulseLength = SERVO_MAX; pulseLength > SERVO_MIN; pulseLength-- )
		pca9685.setPWM( servoNumber, 0, pulseLength );

	// Drive each servo one at a time using microseconds(), it's not precise due to calculation rounding!
	// The microseconds() function is used to mimic the Arduino Servo library microseconds() behavior.
	for( uint16_t microseconds = US_MIN; microseconds < US_MAX; microseconds++ )
		pca9685.writeMicroseconds( servoNumber, microseconds );

	delay( 100 );
	for( uint16_t microseconds = US_MAX; microseconds > US_MIN; microseconds-- )
		pca9685.writeMicroseconds( servoNumber, microseconds );

	servoNumber++;
	if( servoNumber > 3 ) servoNumber = 0; // Testing the first 8 servo channels.
} // End of loop() function.
