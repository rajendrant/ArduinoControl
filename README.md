# ArduinoControl

Provides a way to remote control arduino boards via WiFi. Following functionalities can be controlled.
 * Set pin mode as INPUT/OUTPUT/INPUT_PULLUP.
 * Digital read write.
 * Analog read write.
 * Servo PWM control.

## ArduinoControlSketch
This is the arduino sketch, that runs a server at port 6666 accepting commands from the ArduinoControlClient. WiFi username and password needs to be changed in this sketch.

## ArduinoControlClient
This python client provides API to send commands to the arduino.

### Credits
This code uses following libraries.
 * Google protobuf library from https://github.com/google/protobuf
 * Nanopb protobuf library from https://github.com/nanopb/nanopb

