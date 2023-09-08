/************************************************************************************
 *   
 *  @file     softController
 *  @author   Mario James Lanzilotta (AFFOA), Ellis Hobby (AFFOA)
 *  @version  2.2.3
 *  @brief    ETBL Soft Controller code for Adafruit Feather nRF52
 *  
************************************************************************************/

#include "etbl.h"


ETBL etbl;


/************************************************************************************
 *   
 *  @brief setup()
 *  
************************************************************************************/
void setup() {
  
  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println("\nStarting ETBL Soft Controller");  
  #endif

  DEBUG_PRINTLN((String)"key" + 1);
  etbl.init();

}


/************************************************************************************
 *   
 *  @brief loop()
 *  
************************************************************************************/
void loop() {
  
    etbl.checkBoost(millis());
    etbl.checkKeys();
    etbl.parseKeys();
    etbl.updateLEDs();
}