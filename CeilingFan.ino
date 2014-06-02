/* Ceiling Fan, by Adam Howell
 - Moves 3 servos through a range of positions
 - Will eventually control 3 collective servos and 1 speed controller
 - SEE the comments after "//" on each line below
 - CONNECTIONS:
 - Servo connector plugged on pins 3, 6, and 9.
 - If separate wires:
  - Servo Black or Brown to Gnd.
  - Servo Red or Orange (Center wire) to +5V
  - Servo White or Yellow to Signal (Pin 9)
 - V1.02 02/11/13
 Questions: terry@yourduino.com */


/*-----( Import needed libraries )-----*/
#include <Servo.h>  // Comes with Arduino IDE


/*-----( Declare Constants and Pin Numbers )-----*/
// I no longer use the next definition, since I've modified this to use 3 servos.  I'm keeping it to remind me which pins are usable.
//#define ServoPIN  9  // Can be changed 3,5,6,9,10,11
// The minimum and maximum values may need to be adjusted for each servo.  The theoretical range is from 0 to 180.
#define ServoMIN  0     // This will be the lower value for all servos, 0 is the absolute floor.
#define ServoMAX  180   // This will be the higher value for all servos, 180 is the absolute ceiling.
#define Servo3MIN 70    // This will be the minimum value servo 3 will move to.  This is how high the arm moves up.  Port servo.
#define Servo3MAX 160   // This will be the maximum value servo 3 will move to.  This is how low the arm moves down.  Port servo.
#define Servo6MIN 20    // This will be the minimum value servo 6 will move to.  This is how low the arm moves down.  A smaller number results in a lower stopping point.  Starboard servo.
#define Servo6MAX 110   // This will be the maximum value servo 6 will move to.  This is how high the arm moves up.  Starboard servo.
#define Servo9MIN 70    // This will be the minimum value servo 9 will move to.  This is how high the arm moves up.  A larger number results in less upward vertical travel.  Rear servo.
#define Servo9MAX 150   // This will be the maximum value servo 9 will move to.  This is how low the arm moves down.  Rear servo.


/*-----( Declare objects )-----*/
// create servo objects to control servos
// a maximum of eight servo objects can be created
Servo MyServo3;    // Servo object for pin 3.
Servo MyServo6;    // Servo object for pin 6.
Servo MyServo9;    // Servo object for pin 9.
/*-----( Declare Variables )-----*/
int pos = 0;       // variable to store the servo position
int invpos = 0;    // This is an inverted position to accomodate servo reversing, e.g. servo's on the opposite side of the craft.


void setup()   /****** SETUP: RUNS ONCE ******/
{
//  myservo.attach(ServoPIN);  // attaches the servo on pin 9 to the servo object.
  MyServo3.attach(3);  // Attach MyServo3 to pin 3.
  MyServo6.attach(6);  // Attach MyServo6 to pin 6.
  MyServo9.attach(9);  // Attach MyServo9 to pin 9.

}//--(end setup )---


void loop()   /****** LOOP: RUNS CONSTANTLY ******/
{
  for( pos = ServoMIN; pos < ServoMAX; pos += 1 )    // Count from our minimum value to our maximum value in steps of 1 degree.
  {
    if( pos > Servo3MIN && pos < Servo3MAX )         // This is an upper cap for servo 3.
    {
        MyServo3.write(pos);                         // Move the servo to the position in variable 'pos'.
    }
    invpos = ServoMAX - pos;                         // Calculate the inveted position by taking the maximum value and subtracting the current postitional value.
    if( invpos > Servo6MIN && invpos < Servo6MAX )  // If the left servo goes lower than about 35, it hits the main drive gear, so this prevents that interaction.
    {
        MyServo6.write(invpos);                      // Move the servo to the position in variable 'invpos'.
    }
    if( pos > Servo9MIN && pos < Servo9MAX)          // This is an upper cap for servo 9.
    {
        MyServo9.write(pos);                         // Move the servo to the position in variable 'pos'.
    }
    delay(10);                                       // Wait 15ms for the servo to reach the position.
  }
  delay(1000);
  for( pos = ServoMAX; pos > ServoMIN; pos-=1 )      // Count from our maximum value to our minimum value in steps of 1 degree.
  {                                
    if( pos > Servo3MIN && pos < Servo3MAX )         // This is an upper cap for servo 3.
    {
        MyServo3.write(pos);                         // Move the servo to the position in variable 'pos'.
    }
    invpos = ServoMAX - pos;                         // Calculate the inveted position by taking the maximum value and subtracting the current postitional value.
    if( invpos > Servo6MIN && invpos < Servo6MAX )  // If the left servo goes lower than about 35, it hits the main drive gear, so this prevents that interaction.
    {
        MyServo6.write(invpos);                      // Move the servo to the position in variable 'invpos'.
    }
    if( pos > Servo9MIN && pos < Servo9MAX)          // This is an upper cap for servo 9.
    {
        MyServo9.write(pos);                         // Move the servo to the position in variable 'pos'.
    }
    delay(10);                                       // Wait 15ms for the servo to reach the position.
  }
  delay(1000);

}//--(end main loop )---

