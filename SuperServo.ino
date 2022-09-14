/**
 * Created by Adam on 2022-09-11.
 */

#include "SuperServo.h"


SuperServo::SuperServo( int gpio, long minMicros, long maxMicros, unsigned int reversed )
{
	gpioPin = gpio;
	minMicroseconds = minMicros;
	maxMicroseconds = maxMicros;
	reverse = reversed;

	pinMode( gpioPin, OUTPUT );
	classServo.attach( gpioPin );
	currentPosition = map( 50, 0, 100, minMicroseconds, maxMicroseconds );
	classServo.writeMicroseconds( currentPosition );
	moveTo( 50 );
}


void SuperServo::moveTo( int percent )
{
	currentPosition = map( percent, 0, 100, minMicroseconds, maxMicroseconds );
	classServo.writeMicroseconds( currentPosition );
}


void SuperServo::turnOff()
{
	moveTo( 50 );
}


int SuperServo::getPosition()
{
	return map( currentPosition, minMicroseconds, maxMicroseconds, 0, 100 );
}

void SuperServo::Reverse()
{
	if( reverse == 1 )
		reverse = 0;
	else
		reverse = 1;
}
