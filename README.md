# OneRC
Arduino project to use one remote control to operate three devices (TV, Set-top box and speakerset).

I have three devices, with three separate remote controls
* RC1: Arcadyan HMB2260 Set-top box
    RC1A: Unknown protocol, Manchester encoding on 56kHz
    RC1B: RC5-protocol 38kHz for TV
* RC2: Philips TV (RC6-protocol)
* RC3: Speakerset (NEC-protocol)
   
I want to use just one remote to control all three devices.
We need to connect an IR-receiver and an IR-LED to an Arduino Pro Mini (ATmega328, 16MHz, 5V) and a few other components.
See the electronic diagram.
The Arduino is powered by the USB-port of the set-top box.

NOTE: This project uses the Arduino IRremote library. (https://github.com/z3t0/Arduino-IRremote).
This library seems to conflict with RobotIRremote-library that ships with the Arduino-editor nowadays. Easiest way to fix this problem is to remove the RobotIRRemote-folder from the Arduino installation folder.
