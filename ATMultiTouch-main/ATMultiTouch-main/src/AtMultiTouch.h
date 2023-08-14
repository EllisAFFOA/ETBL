/************************************************************************************
 *
 * 	Name    : AtMultiTouch
 * 	Author  : Mario Lanzilotta, AFFOA
 * 	Date    : September 1st, 2022
 * 	Version : 1.1.1
 * 	Notes   : Arduino Library for use with the AT42QT1070 Q-touch chip via i2c
 *
 ***********************************************************************************/


#ifndef AtMultiTouch_H
#define AtMultiTouch_H

//#define SERIAL_DEBUG  //do you want status messages printed to serial?


#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif


#include <Wire.h>


extern "C" void bttnPressISR(void) __attribute__ ((signal));


class AtMultiTouch {

public:
	friend void bttnPressISR(void); //make friend so bttnPressISR can access private var keyhit

	AtMultiTouch();

/***********************************************************
 *
 * start
 *
 * init the AtMultiTouch class
 *
 ***********************************************************/
	void start(uint8_t interruptPin);

/***********************************************************
 *
 * reset
 *
 * soft resets the chip by writing to the reset register
 *
 ***********************************************************/
	void reset();
	
/***********************************************************
 *
 * calibrate
 *
 * triggers a calibration cycle by writing to the calibration register
 *
 ***********************************************************/
	void calibrate();

/***********************************************************
 *
 * setGroup
 *
 * sets the group of a key to group
 *
 ***********************************************************/
    void setGroup(uint8_t key, uint8_t group);
	
/***********************************************************
 *
 * setAVE
 *
 * sets the averaging factor of key to AVE
 *
 ***********************************************************/
	void setAVE(uint8_t key, uint8_t AVE);

/***********************************************************
 *
 * setGuard
 *
 * sets key as the Guard Channel
 *
 ***********************************************************/
    void setGuard(uint8_t key);

/***********************************************************
 *
 * setAutoCal
 *
 * sets the auto recalibration interval to calInt * 160ms
 *
 ***********************************************************/
    void setAutoCal(uint8_t calInt);

/***********************************************************
 *
 * setThresh
 *
 * sets the negative threshold value for key to register a detection
 *
 ***********************************************************/
    void setThresh(uint8_t key, uint8_t threshold);
	
/***********************************************************
 *
 * setDI
 *
 * sets the number of consecutive measurements that need to 
 * pass the threshold before a key will register a detection 
 *
 ***********************************************************/
    void setDI(uint8_t key, uint8_t num);
	
/***********************************************************
 *
 * setPwr
 *
 * sets the interval between key measurements to LP * 8ms
 *
 ***********************************************************/
    void setPwr(uint8_t LP);

/***********************************************************
 *
 * readActiveKeys
 *
 * returns the byte read from the key status register
 *
 ***********************************************************/
	uint8_t readActiveKeys();

/***********************************************************
 *
 * hit
 *
 * return whether hit is true or false
 *
 ***********************************************************/
	bool hit();

/***********************************************************
 *
 * getKeys
 *
 * get the stored key touch states
 *
 ***********************************************************/
	uint8_t getKeys();

/***********************************************************
 *
 * print signal
 *
 * update the current key touch statuses
 *
 ***********************************************************/	
	void printSignal(uint8_t key);


private:

	uint8_t readKeyReg();

	uint8_t changePin;
	volatile bool keyHit; //Hit detection
	static AtMultiTouch* pAtMultiTouch; //ptr to AtMultiTouch class for the ISR
	uint8_t activeKeys; //store the key touch states

};

#endif
