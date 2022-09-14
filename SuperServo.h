
#ifndef CEILINGFAN_SUPERSERVO_H
#define CEILINGFAN_SUPERSERVO_H

/**
 * Created by Adam on 2022-09-11.
 * It's not really that super.
 * It's just a wrapper class that lets you set the minimum and maximum microseconds for each servo,
 * and then write to them using a percent instead of degrees.
 * I wrote this for a project which controls a radio controlled helicopter.
 * The swashplate has three servos, each with slight variations in their travel.
 * So a solution was needed for tuning them.
 * At first I was saving the min and max settings in global variables, but this got too cumbersome.
 */


#include "CeilingFan.h"


class SuperServo : public Servo
{
 private:
	int gpioPin;
	long minMicroseconds;
	long maxMicroseconds;
	long currentPosition;
	unsigned int reverse;
	Servo classServo;

 public:
	SuperServo( int gpio, long minMicros, long maxMicros, unsigned int reversed );
	void moveTo( int percent );
	void turnOff();
	int getPosition();
	void Reverse();
};


#endif //CEILINGFAN_SUPERSERVO_H
