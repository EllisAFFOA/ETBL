/************************************************************************************
 *
 * 	Name    : AtMultiTouch
 * 	Author  : Mario Lanzilotta, AFFOA
 * 	Date    : September 1st, 2022
 * 	Version : 1.1.2
 * 	Notes   : Arduino Library for use with the AT42QT1070 Q-touch chip via i2c
 *
 ***********************************************************************************/


#include "AtMultiTouch.h"

AtMultiTouch* AtMultiTouch::pAtMultiTouch = 0;

/***********************************************************
 *
 * Constructor
 *
 ***********************************************************/
AtMultiTouch::AtMultiTouch() {
	pAtMultiTouch = this;	//the ptr points to this object
}

/***********************************************************
 *
 * Opens the I2C connection and performs and initial calibration
 * @param interruptPin the pin number connected to the CHANGE line 
 *
 ***********************************************************/
void AtMultiTouch::start(uint8_t interruptPin){
	
	pinMode(changePin, INPUT);
	keyHit = false;
	//Begin the i2c communication
	Wire.begin();
	
	//setup the key change interrupt, call bttnpress on interrupt
	attachInterrupt(digitalPinToInterrupt(interruptPin),bttnPressISR,FALLING); 
	
	reset(); //Reset the chip on startup
	
	//Perform initial calibration cycle
	calibrate();
}

/***********************************************************
 *
 * Performs a soft reset on the device
 *
 ***********************************************************/
void AtMultiTouch::reset() {
	//reset device by writing non-zero value to register 0x39
	Wire.beginTransmission(0x1B); // transmit to device
	Wire.write(0x39);             // sets register pointer to the reset register (0x39)
	Wire.write(0x44);             // send non-zero value to initiate a reset
	Wire.endTransmission(true);      // stop transmitting
	
	delay(125); // wait 125 ms for device to reset
	Wire.begin(); // re-open the i2c after device has restarted
    delay(100); //wait 100 ms for i2c to reopen
}

/***********************************************************
 *
 * Performs a calibration of the keys
 *
 ***********************************************************/
void AtMultiTouch::calibrate() {
	#ifdef SERIAL_DEBUG
        Serial.println("Calibrating...");
    #endif
	uint8_t cal = 1;
	Wire.beginTransmission(0x1B); // transmit to device
	Wire.write(0x38);             // sets register pointer to the cal register (0x38)
	Wire.write(0x44);             // send non-zero value to initiate a cal
	Wire.endTransmission(true);      // stop transmitting
	
	while (cal == 1) {
		Wire.beginTransmission(0x1B); // transmit to device
		Wire.write(0x02); // want to read detection status // set pointer
		Wire.endTransmission();      // stop transmitting
		Wire.requestFrom(0x1B, 1);    // request 1 byte from slave device
		cal = Wire.read()>>7;
	}
}

/***********************************************************
 *
 * Sets the AKS group for the specified key
 * @param key the key to be modified
 * @param group the group number to be set for key
 *
 ***********************************************************/
void AtMultiTouch::setGroup(uint8_t key, uint8_t group) {
	Wire.beginTransmission(0x1B); // transmit to device
	Wire.write(0x27 + key); // want to read detection status // set pointer
	Wire.endTransmission();      // stop transmitting
	Wire.requestFrom(0x1B, 1);    // request 1 byte from slave device
	uint8_t AVE = Wire.read()>>2;
	
    //Check for valid arguments to avoid writing to incorrect memory locations
    if (key >= 0 && key <= 6) {
        if (group >= 0 && group <= 3) {
            Wire.beginTransmission(0x1B);
            Wire.write(0x27 + key);   //Set memory address to 39 (key 0) + key 
            Wire.write((AVE<<2) + group); //Write group number to first 2 bits, Averaging factor to remaining 6
            Wire.endTransmission(true);
        }
        //If invalid, print error
        else {
            #ifdef SERIAL_DEBUG
                Serial.println("Invalid Group #");
            #endif
        }
    }
    //If invalid, print error
    else {
        #ifdef SERIAL_DEBUG
            Serial.println("Invalid Key #");
        #endif
    }
}

/***********************************************************
 *
 * Sets the averaging factor AVE for the specified key
 * @param key the key to be modified
 * @param AVE the averaging factor to be set. Internally limited to 1, 2, 4, 8, 16, or 32
 *
 ***********************************************************/
void AtMultiTouch::setAVE(uint8_t key, uint8_t AVE) {
	Wire.beginTransmission(0x1B); // transmit to device
	Wire.write(0x27 + key); // want to read detection status // set pointer
	Wire.endTransmission();      // stop transmitting
	Wire.requestFrom(0x1B, 1);    // request 1 byte from slave device
	uint8_t group = (Wire.read() & 0x03);
	
    //Check for valid arguments to avoid writing to incorrect memory locations
    if (key >= 0 && key <= 6) {
        if (AVE >= 1 && AVE <= 32) {
            Wire.beginTransmission(0x1B);
            Wire.write(0x27 + key);   //Set memory address to 39 (key 0) + key 
            Wire.write((AVE<<2) + group); //Write group number to first 2 bits, Averaging factor to remaining 6
            Wire.endTransmission(true);
        }
        //If invalid, print error
        else {
            #ifdef SERIAL_DEBUG
                Serial.println("Invalid Averaging Factor");
            #endif
        }
    }
    //If invalid, print error
    else {
        #ifdef SERIAL_DEBUG
            Serial.println("Invalid Key #");
        #endif
    }
}


/***********************************************************
 *
 * Sets the specified key as the guard channel
 * @param key the key to be set as the guard channel. > 6 disables the guard channel
 *
 ***********************************************************/
 void AtMultiTouch::setGuard(uint8_t key) {
	if (key >= 0 && key <= 15) {
        Wire.beginTransmission(0x1B);
        Wire.write(0x35);   //Set memory address to 53, config guard channel
        Wire.write(key); //Set key as guard channel
        Wire.endTransmission(true);
    }
    //If invalid, print error
    else {
        #ifdef SERIAL_DEBUG
            Serial.println("Invalid Key #");
        #endif
    }
 }

/***********************************************************
 *
 * Sets the max on duration before the keys are recalibrated
 * @param calInt the maximum time a key can be on before forced recalibration * 160ms
 *
 ***********************************************************/
void AtMultiTouch::setAutoCal(uint8_t calInt) {
    Wire.beginTransmission(0x1B);   
	Wire.write(0x37); //set to register 55, config Max On duration
	Wire.write(calInt);
	Wire.endTransmission(true);
}

/***********************************************************
 *
 * Sets the negative threshold that must be reached for a key to go into detectgraph
 * @param key the key to be modified
 * @param threshold the negative threshold to be set
 *
 ***********************************************************/
void AtMultiTouch::setThresh(uint8_t key, uint8_t threshold) {
	//Check for valid arguments
    if (key >= 0 && key <= 6) {
        Wire.beginTransmission(0x1B);
        Wire.write(0x20 + key);   //Set memory address to 32 (key 0) + key 
        Wire.write(threshold); //Write threshold to memory location
        Wire.endTransmission(true); 
    }
    //If invalid, print error
    else {
        #ifdef SERIAL_DEBUG
            Serial.println("Invalid Key #");
        #endif
    }
}

/***********************************************************
 *
 * Sets the number of consecutive measurements needed for a key to go into detectgraph
 * @param key the key to be modified
 * @param num the number of measurements needed
 *
 ***********************************************************/
void AtMultiTouch::setDI(uint8_t key, uint8_t num) {
	//Check for valid arguments
    if (key >= 0 && key <= 6) {
		//minimum of 2 consecutive measurements needed
        Wire.beginTransmission(0x1B);
        Wire.write(0x2E + key);   //Set memory address to 46 (key 0) + key 
        Wire.write(num); //Write DI level to memory location
        Wire.endTransmission(true);
    }
    //If invalid, print error
    else {
        #ifdef SERIAL_DEBUG
            Serial.println("Invalid Key #");
        #endif
    }
}

/***********************************************************
 *
 * Sets the interval between measurements
 * @param LP the time between measurements * 8ms
 *
 ***********************************************************/
void AtMultiTouch::setPwr(uint8_t LP) {
	Wire.beginTransmission(0x1B);   
	Wire.write(0x36); //set to register 54, config low power mode
	Wire.write(LP);
	Wire.endTransmission(true);
}

/***********************************************************
 *
 * @returns keyHit the hit detection flag
 *
 ***********************************************************/
bool AtMultiTouch::hit() {
	return keyHit;
}

/***********************************************************
 *
 * @returns the previous read value of the key status register
 *
 ***********************************************************/
uint8_t AtMultiTouch::getKeys() {
	return  activeKeys;
}

/***********************************************************
 *
 * Updates the stored key statuses to the current value
 * @returns the new value of the key status register
 *
 ***********************************************************/
uint8_t AtMultiTouch::readActiveKeys() {
	
	uint8_t regOut = readKeyReg(); //read data from key status register
	#ifdef SERIAL_DEBUG
        Serial.println(regOut, BIN);
    #endif
	//if no data recieved, don't update key statuses
    if (regOut != -1) { 
        activeKeys = regOut;
    }
    return activeKeys;
}

/***********************************************************
 *
 * Prints the raw signal value of key from the signal register
 * @param key the key to have its signal read and printed
 *
 ***********************************************************/
void AtMultiTouch::printSignal(uint8_t key) {
	uint8_t sig1, sig2;
	
	Wire.beginTransmission(0x1B); // transmit to device
	Wire.write(0x04 + (2*key)); // want to read detection status // set pointer
	Wire.endTransmission();      // stop transmitting
	Wire.requestFrom(0x1B, 1);    // request 1 byte from slave device
	sig1 = Wire.read();
	
	Wire.beginTransmission(0x1B); // transmit to device
	Wire.write(0x05 + (2*key)); // want to read detection status // set pointer
	Wire.endTransmission();      // stop transmitting
	Wire.requestFrom(0x1B, 1);    // request 1 byte from slave device
	sig2 = Wire.read();
	
	#ifdef SERIAL_DEBUG
        Serial.print("Key");
		Serial.print(key);
		Serial.print(" Signal: ");
		Serial.println((sig1<<8) + sig2);
    #endif
}

/***********************************************************
 *
 * Reads the key status register
 * @returns the read value of the key status register
 *
 ***********************************************************/
uint8_t AtMultiTouch::readKeyReg(){

	if(keyHit == false) { //safety in case readActiveKeys called without interrupt 
		return -1;
	}
	else {

		uint8_t bttnReg = 0;

		// to clear change we must read both status bytes 02 and 03
		Wire.beginTransmission(0x1B); // transmit to device
		Wire.write(0x02); // want to read detection status // set pointer
		Wire.endTransmission();      // stop transmitting
		Wire.requestFrom(0x1B, 1);    // request 1 byte from slave device
		Wire.read();

		Wire.beginTransmission(0x1B); // transmit to device
		Wire.write(0x03); // want to read key status // set pointer
		Wire.endTransmission();      // stop transmitting
		Wire.requestFrom(0x1B, 1);    // request 1 byte from slave device
		bttnReg = Wire.read();
		
		keyHit = false; //Reset keyHit status
		return bttnReg;
	}
}

/***********************************************************
 *
 * ISR function called when the interrrupt triggers
 *
 ***********************************************************/
void bttnPressISR() {
	AtMultiTouch::pAtMultiTouch->keyHit = true; //Set keyHit to true when interrupt triggers
}
