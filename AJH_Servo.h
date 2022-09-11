// AJH_Servo.h
#ifndef AJH_SERVO_h
#define AJH_SERVO_h

#include <CeilingFan.h>

class AJH_Servo
{
 private:
	int ledPin;
	unsigned char ledState;
	unsigned int position;

 public:
	AJH_Servo( int gpio, int minMicros, int maxMicros, int reverse );
	void moveTo( int percent );
	void turnOFF();
	int getState();
};

#endif // AJH_SERVO_h
