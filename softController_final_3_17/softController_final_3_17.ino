/************************************************************************************
 *   
 *  @file     softController
 *  @author   Mario James Lanzilotta, AFFOA
 *  @version  2.2.2
 *  @brief    ETBL Soft Controller code for Adafruit Feather nRF52
 *  
************************************************************************************/

#include <Wire.h>
#include <AtMultiTouch.h>
#define VBATPIN A7

#define DEBUG
//#undef DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
#endif

// ms to wait before triggering watchdog
#define WATCHDOG_TIME 500

unsigned long wdt[7] = {0, 0, 0, 0, 0, 0, 0};
bool keyReady[7] = {true, true, true, true, true, true, true};

AtMultiTouch softController;

uint_least8_t hitKeys; //Readback from register, stores which channels are being touched
uint_least8_t lastState = 0; //Current state of touch keys, previous stored value of hitKeys

uint_least8_t interruptPin = 27; //CHANGE pin 

uint_least8_t numLEDs = 8; //5 + RGB LEDs
uint_least8_t ledPins[] = {2, 3, 4, 5, 16, 15, 7, 11}; //Pwm out pins to LED drivers (RGB is last 3)
uint8_t ledState = 0; //Current state of the LEDs (LSB is first LED
uint_least8_t ledValues[] = {0, 0, 0, 0, 0, 0, 0, 0}; //Current values of the LEDs

uint_least8_t rgbValues[][3] = {{255,255,255}, {255,0,0}, {100,5,0}, {255,128,0}, {0,255,0}, {0,255,255}, {0,0,255}, {80,0,80}}; //Array of RGB values
uint8_t rgbState = 0; //Variable to track RGB LED color

uint_least8_t boostEN = 30; //Enable Pin for Boost Converter

unsigned long debounceTimer;
bool timerOn = false; //tracker for debounce timer

float VBat; //Battery Voltage
unsigned long batTimer = 0;
bool idle = false; //Tracks power state of boost converter

/************************************************************************************
 *   
 *  @brief setup()
 *  
************************************************************************************/
void setup() {
  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println("\nStarting ATMultiTouch Lib");  
  #endif
  softController.start(interruptPin);
  softController.setAutoCal(30); //Max on duration = 19 * 160ms = ~3s
  softController.setGuard(7); //Disable Guard Key
  softController.setPwr(2);
  for (uint_least8_t i = 0; i < 7; i++) {//Loop through all channels
      softController.setGroup(i,0); //Set all pins to group 0
      softController.setAVE(i,8); //Set Averaging factor
      softController.setDI(i,16); //Set detection integrator
      if ( i < 3) {
        softController.setThresh(i,50); //Set negative threshold
      } else {
        softController.setThresh(i,50); //Set negative threshold
      }
  }
  uint_least8_t ledInitialValues[] = {255, 255, 255, 255, 255, 255, 255, 255}; 
  //Initialize LEDs
  for (uint_least8_t i = 0; i < numLEDs; i++) {
      pinMode(ledPins[i],OUTPUT);
      analogWrite(ledPins[i],ledInitialValues[i]);
  }


  delay(1000);
  
  //Calibrate softcontroller
  softController.calibrate(); 

  delay(100);

  //Enable Boost Converter
  pinMode(boostEN, OUTPUT);
  digitalWrite(boostEN, 1);
  readBattery();
}


/************************************************************************************
 *   
 *  @brief loop()
 *  
************************************************************************************/
void loop() {
  
    //Update boost converter state based on battery level/idle
    checkBoost(millis());
    
    //check if the hit flag has been triggered by the interrupt
    if(softController.hit() == true) {

      //Read the status registers
      hitKeys = (softController.readActiveKeys()); 
      #ifdef DEBUG
        Serial.println(hitKeys, BIN);
      #endif

      //if state has changed, start debounce timer
      if (hitKeys != lastState) {
        debounceTimer = millis();
        timerOn = true;
      }
      //If current state returns to stored state, stop timer
      else {
        timerOn = false; 
      }
    }

    
    selectKey();
}

/************************************************************************************
 *   
 * @brief update the LED state based on the keys pressed
 *  
************************************************************************************/
void selectKey() {
  //if debounce timer runs out, update state and LEDs
  if (((millis() - debounceTimer) >= 100) && timerOn == true) { //50ms debounce delay
    timerOn = false;

    switch (hitKeys) {
      case 0: //0 is no keys pressed
        DEBUG_PRINT("UP");
        //Serial.println("UP");
        break;
      case 1: //Left Wrist
        DEBUG_PRINT("KEY1");
        //Serial.println("KEY1");
        ledState = (ledState ^ 2);
        break;
      case 2: //Right Wrist
        DEBUG_PRINT("KEY2");
        //Serial.println("KEY2");
        ledState = (ledState ^ 8);
        break;
      case 4: //Back
        DEBUG_PRINT("KEY3");
        ///Serial.println("KEY3");
        ledState = (ledState ^ 1);
        break;
      case 5: //left arm
        DEBUG_PRINT("KEY4");
        //Serial.println("KEY4");
        ledState = (ledState ^ 4);
        break;
      case 6: //right arm
        DEBUG_PRINT("KEY5");
        //Serial.println("KEY5");
        ledState = (ledState ^ 16);
        break;
      case 7:
        wdtCheck(0);
        if (keyReady[0]) {
          DEBUG_PRINT("KEY6");
          //Serial.println("KEY6");
          ledState = (ledState ^ 224);
          //Update RGB Values
          ledValues[5] = rgbValues[rgbState/2][0]; //Red
          ledValues[6] = rgbValues[rgbState/2][1]; //Green
          ledValues[7] = rgbValues[rgbState/2][2]; //Blue
          rgbState++;
          if (rgbState > 15) {
            rgbState = 0;
          }
          wdtReset(0);
        }
        break;
      default:
         break;
    }
    //Update previous state
    lastState = hitKeys;

    //Update LEDs
    updateLEDs();
  }
}

// Check if the WDT has been triggered
void wdtCheck(int idx) {
  // If over WATCHDOG_TIME ms has passed set keyReady to true
  keyReady[idx] = millis() - wdt[idx] >= WATCHDOG_TIME;
}

// Reset the WDT to the current time
void wdtReset(int idx) {
  wdt[idx] = millis();
}


/************************************************************************************
 *   
 * @brief Writes to the LEDs according to ledValues and ledState
 *  
************************************************************************************/
void updateLEDs() {
    for (uint_least8_t i = 0; i < numLEDs; i++) { //Update all LEDs
      uint8_t state = (ledState & (1 << i)) >> i;
      analogWrite(ledPins[i], (ledValues[i] * state));
    }
}
/************************************************************************************
 *   
 * @brief reads the battery voltage and updates VBat
 * @returns the updated battery voltage 
 *  
************************************************************************************/
float readBattery() {
    pinMode(VBATPIN,INPUT);
    VBat = analogRead(VBATPIN);
    VBat *= 1.53; 
    VBat *= 3.3;  // Multiply by 3.3V, our reference voltage
    VBat /= 1024; 
    return VBat;
}

/************************************************************************************
 *   
 *  @brief updates boost converter state 
 *  @param currTime the current time from millis()
 *  
************************************************************************************/
void checkBoost(unsigned long currTime) {
    //Check if idle
    if (((currTime - debounceTimer) >= 10000) && ledState == 0) { //Idle for 10s before sleeping
      idle = true;
      if (digitalRead(boostEN) == 1) {
        digitalWrite(boostEN, 0);
        DEBUG_PRINT("ZzZzZzZzZ");
        //Serial.println("ZzZzZzZzZ");
      }
    }
    //If not idle, turn boost on if battery is charged enough
    else {
      idle = false;
      if (digitalRead(boostEN) == 0 && VBat >= 3.3) {
        digitalWrite(boostEN, 1);
        DEBUG_PRINT("I'm awake");
        //Serial.println("I'm awake");
      }
    }
    
    //Check battery voltage
    if (((currTime - batTimer) >= 60000)) { //Wait 60s before checking voltage
      batTimer = currTime;
      //Disable boost if battery voltage below 3.3V
      if (readBattery() < 3.3) {
        digitalWrite(boostEN, 0);
        digitalWrite(17, 1);
      }
      //Otherwise, check if idle
      else {
        if (!idle) {
          digitalWrite(boostEN, 1);
          digitalWrite(17, 0);
        }
      }
      DEBUG_PRINT(VBat);
      DEBUG_PRINT("V");
      //Serial.print(VBat); Serial.println("V");
    }
}