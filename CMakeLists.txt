cmake_minimum_required( VERSION 3.23 )
project( CeilingFan )

set( CMAKE_CXX_STANDARD 14 )

include_directories( .
		C:/Users/Adam/AppData/Local/Arduino15/packages/arduino/hardware/avr/1.8.5/cores/arduino/
		C:/Code/Arduino/libraries/
		C:/Code/Arduino/libraries/Servo/src/
		C:/Users/Adam/AppData/Local/Arduino15/packages/esp8266/hardware/esp8266/3.0.2/libraries/Wire/
		C:/Users/Adam/AppData/Local/Arduino15/packages/esp8266/hardware/esp8266/3.0.2/libraries/ESP8266WiFi/src/
		C:/Code/Arduino/libraries/ArduinoJson/
		C:/Code/Arduino/libraries/PubSubClient/src/
		C:/Code/Arduino/libraries/Adafruit_PWM_Servo_Driver_Library/
		"C:/Program Files (x86)/Arduino/hardware/arduino/avr/cores/arduino/"
		"C:/Program Files (x86)/Arduino/hardware/tools/avr/lib/gcc/avr/7.3.0/include/"
		C:/Users/Adam/AppData/Local/Arduino15/packages/arduino/tools/avr-gcc/7.3.0-atmel3.6.1-arduino7/avr/include/
		C:/Users/Adam/AppData/Local/Arduino15/packages/esp8266/hardware/esp8266/3.0.2/libraries/ESP8266mDNS/src/
		C:/Users/Adam/AppData/Local/Arduino15/packages/esp8266/hardware/esp8266/3.0.2/libraries/ArduinoOTA/
		)

add_executable( CeilingFan
		CeilingFan.ino
		NetworkFunctions.ino
		CeilingFan.h
		privateInfo.h
		ServoFunctions.ino
		SuperServo.ino SuperServo.h )

set_target_properties( CeilingFan PROPERTIES LINKER_LANGUAGE CXX )
