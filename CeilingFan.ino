/*
    Ceiling Fan
    A ceiling fan made from a R/C helicopter.
    Create 2020-07-28
    by Adam Howell
    https://github.com/AdamJHowell

    Hardware:
      HiLetgo ESP8266 NodeMCU ESP-12E
      Hobby King HK450 CCPM radio controlled helicopter
      Turnigy Typhoon 450H 2215H Heli Motor 3550kv
      Hobbyking SS Series 25-30A ESC
      4x Corona 919MG Digital Metal Gear Servo
      Antec NEO480 PSU
      Electronics-Salon 24/20-pin ATX DC Power Supply Breakout Board Module
      Radio Shack Experimentor 350 breadboard
      Hookup wire (various)
   CONNECTIONS:
      Cyclic/Collective Pitch Mixing (CCPM) Servos plugged into digital pins D2, D3, and D4.
      ESC plugged into digital pin D0.
      Twelve servo objects can be created on most boards using GPIO pins.
    If separate wires:
      Servo Black or Brown to Gnd.
      Servo Red or Orange (Center wire) to +5V
      Servo White or Yellow to Signal (Pin 9)
    API:
      To simplify the API, only one digit will be used for servo ranges.
      This means a range of 0 to 9.
      I may switch to a 0 to 99 range in a later release.
*/

#include <Servo.h>
#include <ESP8266WiFi.h>      // Network Client for the WiFi chipset.
#include <PubSubClient.h>     // PubSub is the MQTT API.

// WiFi and MQTT constants.
const char* ssid = "Red";
const char* password = "8012254722";
const char* mqttServer = "192.168.55.200";
const int mqttPort = 2112;
const char* mqttTopic = "mqttServo";

WiFiClient espClient;
PubSubClient mqttClient( espClient );
Servo throttleServo;    // Create servo object to control the ESC.
Servo rudderServo;      // Create servo object to control the rudder.
Servo collective1Servo; // Create servo object to control one of the three collective servos.
Servo collective2Servo; // Create servo object to control one of the three collective servos.
Servo collective3Servo; // Create servo object to control one of the three collective servos.


void moveServo( Servo thisServo, int position )
{
  if ( position < 0 )           // Check for an invalid value.
  {
    Serial.print( "moveServo() was given an invalid servo position: " );
    Serial.println( position );
    position = 5;               // Set the servo to a low, but valid, position.
  }
  else if ( position > 180 )    // Check for an invalid value.
  {
    Serial.print( "moveServo() was given an invalid servo position: " );
    Serial.println( position );
    position = 175;             // Set the servo to a low, but valid, position.
  }
  else
  {
    Serial.print( "\nMoving to position " );
    Serial.println( position );
    thisServo.write( position );               // Move the servo.
    delay( 300 );   // This delay is crucial!
  }
} // End of moveServo() function.


/**
   throttleChange() will handle the scaling and limits needed, and pass real-world values to moveServo().
*/
void throttleChange( int receivedValue )
{
  int finalValue = receivedValue * 20;    // Multiply by 20 to scale from 1-9 up to 1-180.
  moveServo( throttleServo, finalValue );
} // End of throttleChange() function.


/**
   collectiveChange() will handle the scaling and limits needed, and pass real-world values to moveServo().
*/
void collectiveChange( int receivedValue )
{
  int finalValue = receivedValue * 20;
  // ToDo: These next 3 lines may need significant tweaking of finalValue before calling moveServo().
  // Some servos will need finalValue inverted (180 - finalValue).
  moveServo( collective1Servo, finalValue );
  moveServo( collective2Servo, finalValue );
  moveServo( collective3Servo, finalValue );
} // End of collectiveChange() function.


/**
   rudderChange() will handle the scaling and limits needed, and pass real-world values to moveServo().
*/
void rudderChange( int receivedValue )
{
  int finalValue = receivedValue * 20;
  moveServo( rudderServo, finalValue );
} // End of rudderChange() function.


/**
   floodLightChange() will toggle the floodlights on or off.
   ~~~Presently, this will only toggle the on-board LED~~~
   ToDo: Change this to whichever GPIO pin will eventually drive the floodlight LEDs.
*/
void floodLightChange( int receivedValue )
{
  // Note that some boards consider 'HIGH' to be off.
  if ( receivedValue == 0 )
    digitalWrite( LED_BUILTIN, HIGH );    // Turn the LED off.
  if ( receivedValue == 1 )
    digitalWrite( LED_BUILTIN, LOW );     // Turn the LED on.
} // End of floodLightChange() function.


void tlofLightChange( int receivedValue )
{

} // End of tlofLightChange() function.


void fatoLightChange( int receivedValue )
{

} // End of fatoLightChange() function.


/**
   callback() handles MQTT subscriptions.
   When a message comes in on a topic we have subscribed to, this function is executed.
*/
void callback( char* topic, byte* payload, unsigned int length )
{
  Serial.println();
  Serial.print( "Message arrived [" );
  Serial.print( topic );
  Serial.print( "] " );
  for ( int i = 0; i < length; i++ )
  {
    char receivedKey = ( char )payload[i];
    Serial.print( receivedKey );

    if ( receivedKey == 't' )        // Process throttle changes.
    {
      if ( length > 1 )
      {
        char receivedValue = ( char )payload[i + 1];    // Store the 0-9 value in receivedValue.
        i++;                                            // Increment the index since it was consumed.
        throttleChange( receivedValue - '0' );          // Send the 0-9 value.
      }
    }
    else if ( receivedKey == 'c' )        // Process collective changes.
    {
      if ( length > 1 )
      {
        char receivedValue = ( char )payload[i + 1];    // Store the 1-9 value in receivedValue.
        i++;                                            // Increment the index since it was consumed.
        collectiveChange( receivedValue - '0' );        // Send the 1-9 value.
      }
    }
    else if ( receivedKey == 'r' )        // Process rudder changes.
    {
      if ( length > 1 )
      {
        char receivedValue = ( char )payload[i + 1];    // Store the 1-9 value in receivedValue.
        i++;                                            // Increment the index since it was consumed.
        rudderChange( receivedValue - '0' );            // Send the 1-9 value.
      }
    }
    else if ( receivedKey == 'f' )        // Process floodlight changes.
    {
      if ( length > 1 )
      {
        char receivedValue = ( char )payload[i + 1];    // Store the 0-1 value in receivedValue.
        i++;                                            // Increment the index since it was consumed.
        floodLightChange( receivedValue - '0' );        // Send the 0-1 value.
      }
    }
    else if ( receivedKey == 'l' )        // Process green TLOF circle LED changes.
    {
      if ( length > 1 )
      {
        char receivedValue = ( char )payload[i + 1];    // Store the 0-1 value in receivedValue.
        i++;                                            // Increment the index since it was consumed.
        tlofLightChange( receivedValue - '0' );         // Send the 0-1 value.
      }
    }
    else if ( receivedKey == 'a' )        // Process white FATO square LED changes.
    {
      if ( length > 1 )
      {
        char receivedValue = ( char )payload[i + 1];    // Store the 0-1 value in receivedValue.
        i++;                                            // Increment the index since it was consumed.
        fatoLightChange( receivedValue - '0' );         // Send the 0-1 value.
      }
    }
  }
} // End of callback() function.


/**
   reconnect() will attempt to reconnect the MQTT and WiFi clients.
*/
void reconnect()
{
  // Loop until we're reconnected.
  while ( !mqttClient.connected() )
  {
    Serial.print( "Attempting MQTT connection..." );
    if ( mqttClient.connect( "ESP8266 Client" ) )         // Attempt to connect using the designated clientID.
    {
      Serial.println( "connected" );
      mqttClient.subscribe( mqttTopic );                // Subscribe to the designated MQTT topic.
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
   The setup() function runs once when the device is booted, and then loop() takes over.
*/
void setup()
{
  throttleServo.attach( 16 );     // Attaches the servo on GPIO16 (D0) to the throttle servo object.
  rudderServo.attach( 5 );        // Attaches the servo on GPIO5 (D1) to the rudder servo object.
  collective1Servo.attach( 4 );   // Attaches the servo on GPIO4 (D2) to the collective1 servo object.
  collective2Servo.attach( 0 );   // Attaches the servo on GPIO0 (D3) to the collective2 servo object.
  collective3Servo.attach( 2 );   // Attaches the servo on GPIO2 (D4) to the collective3 servo object.
  throttleServo.write( 90 );       // Set the throttle to zero.
  rudderServo.write( 90 );        // Move the servo to its center position.
  collective1Servo.write( 90 );   // Move the collective to neutral.
  collective2Servo.write( 90 );   // Move the collective to neutral.
  collective3Servo.write( 90 );   // Move the collective to neutral.
  Serial.begin( 115200 );         // Start the Serial communication to send messages to the computer.
  delay( 10 );
  Serial.println( '\n' );

  mqttClient.setServer( mqttServer, mqttPort );   // Set the MQTT client parameters.
  mqttClient.setCallback( callback );             // Assign the callback() function to handle MQTT callbacks.

  pinMode( LED_BUILTIN, OUTPUT );             // Initialize digital pin LED_BUILTIN as an output.

  WiFi.begin( ssid, password );               // Connect to the WiFi network.
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
  while ( WiFi.status() != WL_CONNECTED )     // Wait for the WiFi to connect.
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
  Serial.println( WiFi.localIP() );           // Send the IP address of the ESP8266 to the computer.
} // End of setup() function.


/**
   The loop() function begins after setup(), and repeats as long as the unit is powered.
*/
void loop()
{
  int pos;    // A position variable for the servo.
  // My servos are fully counter-clockwise when 0 is written to them.

  if ( !mqttClient.connected() )
  {
    reconnect();
  }
  mqttClient.loop();
} // End of loop() function.
