```
Name    : AtMultiTouch Readme
Author  : Mario James Lanzilotta, AFFOA
Date    : September 1st 2022
Version : 1.1.1
Notes   : Arduino Library for the Atmel AT42QT1070 QTouch 7-channel Sensor IC
```

Supports Multitouch and Comms mode on the AT42QT1070 chip via I2C with Wire library. Based on the AtTouch Library by Noah Shibley

### Functions

```cpp
start(uint8_t interruptPin); //initializes the I2C bus and sets up the interrupt
reset(); //soft resets the AT42QT1070 
calibrate(); //triggers a calibration cycle
setGroup(uint8_t key, uint8_t group); //sets the group number of key to group
setAVE(uint8_t key, uint8_t AVE); //sets the averaging factor of key to AVE
setGuard(uint8_t key); //sets key as the guard pin, or disables the guard pin if > 6
setAutoCal(uint8_t calInt); //sets the maximum on duration to calInt * 160ms
setThresh(uint8_t key, uint8_t threshold); //sets the negative threshold needed to register a detection for key to threshold
setDI(uint8_t key, uint8_t num); //sets the detection integrator level for key to num
setPwr(uint8_t LP); //sets the interval between key measurements to LP * 8ms
hit(); //returns whether a key has been hit or not, True/False
getKeys(); //returns the stored state of the keys from the last register read
printSignal(uint8_t key); //prints the signal value of key for debugging
readActiveKeys(); //reads the key status register and returns the current state of the keys 
```
