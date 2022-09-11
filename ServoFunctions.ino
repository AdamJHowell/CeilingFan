//
// Created by Adam on 2022-08-30.
//
#include "CeilingFan.h"

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
		digitalWrite( FLOOD_LED_PIN, LED_OFF ); // Turn the LED off.
	else
	{
		digitalWrite( FLOOD_LED_PIN, LED_ON ); // Turn the LED on.
		digitalWrite( MCU_LED, LED_ON );			// Turn the LED on.
		digitalWrite( ESP12LED, LED_ON );		// Turn the LED on.
	}
} // End of floodLightChange() function.


/**
 * tlofLightChange() will toggle the Touchdown Liftoff lights on or off.
 */
void tlofLightChange( int receivedValue )
{
	// Note that some boards consider 'HIGH' to be off.
	if( receivedValue == 0 )
		digitalWrite( TLOF_LED_PIN, LED_OFF ); // Turn the LED off.
	else
		digitalWrite( TLOF_LED_PIN, LED_ON ); // Turn the LED on.
} // End of tlofLightChange() function.


/**
 * fatoLightChange() will toggle the Final Approach Liftoff lights on or off.
 */
void fatoLightChange( int receivedValue )
{
	// Note that some boards consider 'HIGH' to be off.
	if( receivedValue == 0 )
		digitalWrite( FATO_LED_PIN, LED_OFF ); // Turn the LED off.
	else
		digitalWrite( FATO_LED_PIN, LED_ON ); // Turn the LED on.
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
	digitalWrite( FLOOD_LED_PIN, LED_OFF );
	// Turn the LED on.
	digitalWrite( TLOF_LED_PIN, LED_OFF );
	// Turn the LED on.
	digitalWrite( FATO_LED_PIN, LED_OFF );
	// Center the rudder servo and collective servos.
	rudderServo.write( rudderPos );
	collective1Servo.write( collective1Pos );
	collective2Servo.write( collective2Pos );
	collective3Servo.write( collective3Pos );
	delay( 1000 );
} // End of killSwitch() function.
