/************************************************************************************
 * 	
 * 	Name    : AtMultiTouch Example                     
 * 	Author  : Mario Lanzilotta, AFFOA                        
 * 	Date    : September 14th, 2022                                   
 * 	Version : 1.1                                              
 * 	Notes   : Arduino Library for use with the AT42QT1070 Q-touch chip via I2C
************************************************************************************/

#include <Wire.h>
#include <AtMultiTouch.h>

AtMultiTouch touch;

uint8_t hitKeys;
int interruptPin = 27;
int ledPin = 17; 

void setup() {
    Serial.begin(115200);
    Serial.println("\nATMultiTouch Lib v1.0 by MJL");  
    touch.start(interruptPin);
    touch.setAutoCal(32); //Max on duration = 32 * 160ms = 5.12s
    touch.setGuard(7); //Disable Guard Key
    touch.setPwr(1); //Set Power to highest
    for (int i = 0; i < 7; i++) {
        touch.setGroup(i,0); //Set all pins to group 0
        touch.setAVE(i,8); //Set Averaging factor
        touch.setDI(i,4); //Adjust detection integrator
        touch.setThresh(i,50); //Adjust negative threshold
    }
    pinMode(ledPin,OUTPUT);
    digitalWrite(ledPin,HIGH);
}

void loop() {
  
  //check if the hit flag has been triggered by the interrupt 
  if(touch.hit() == true) { 
		  hitKeys = touch.readActiveKeys();
     
      //0 is no keys pressed
	    if(hitKeys == 0) { 
	        Serial.println("keyUP");
          digitalWrite(ledPin,LOW); 
	    }
      //Perform bitwise operations to isolate bits and print key number
	    else { 
          Serial.print("key");
          if (hitKeys & 1) {
              Serial.println("0");   
          }
          if ((hitKeys & 2)>>1) {
              Serial.println("1");   
          } 
          if ((hitKeys & 4)>>2) {
              Serial.println("2");   
          } 
          if ((hitKeys & 8)>>3) {
              Serial.println("3");   
          } 
          if ((hitKeys & 16)>>4) {
              Serial.println("4");   
          } 
          if ((hitKeys & 32)>>5) {
              Serial.println("5");   
          } 
          if ((hitKeys & 64)>>6) {
              Serial.println("6");   
          } 
          digitalWrite(ledPin,HIGH);
	    }
  } 
}
