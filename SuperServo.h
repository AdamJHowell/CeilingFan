//
// Created by Adam on 2022-09-11.
//

#ifndef CEILINGFAN_SUPERSERVO_H
#define CEILINGFAN_SUPERSERVO_H


class SuperServo : public Servo
{
 private:
	unsigned int gpioPin;
	unsigned int minMicroseconds;
	unsigned int maxMicroseconds;
	unsigned int currentPosition;
	unsigned int reverse;

 public:
	SuperServo( int gpio, int minMicros, int maxMicros, int reverse );
	void moveTo( int percent );
	void turnOFF();
	int getState();
};


#endif //CEILINGFAN_SUPERSERVO_H
