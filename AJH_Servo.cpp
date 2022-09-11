// AJH_Servo.cpp
#include "AJH_Servo.h"

AJH_Servo::AJH_Servo( int gpio, int minMicros, int maxMicros, int reverse )
{
	ledPin = gpio;
	ledState = LOW;
	pinMode( ledPin, OUTPUT );
}

void AJH_Servo::moveTo( int percent )
{
	ledState = HIGH;
	digitalWrite( ledPin, ledState );
}

void AJH_Servo::turnOFF()
{
	ledState = LOW;
	digitalWrite( ledPin, ledState );
}

int AJH_Servo::getState()
{
	return ledState;
}
