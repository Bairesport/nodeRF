

/* 
 *
 *     Firmware for basic node.
 *     
 *
 */

#include "protocoloApp.h"
#include "globals.h"
#include "configmanager.h"
#include "transceiver.h"

#include <RFM12B.h>
#include <Bounce.h>
//#include <avr/sleep.h>
#include <SPIFlash.h>
#include <SPI.h>
#include <WirelessHEX.h>
#include "hal.h"
#include <TimerOne.h>

// Configuration manager
configManager *Conf;
// Create Bounce objects for each button.  
Bounce         * button[] = { new Bounce(INPUT0, DEBOUNCE), new Bounce(INPUT1, DEBOUNCE) }; // Bounce(port, ms)
#define        BUTT_MAX  (sizeof(button) / sizeof(Bounce *))
// ADC instances
//Potentiometer  * adcIn[] = { new Potentiometer(ADC_0, 256), new Potentiometer(ADC_1, 256) }; //  Potentiometer(port, sectors)
//#define        ADC_MAX  (sizeof(adcIn) / sizeof(Potentiometer *))

bool refresh;

/* ********************************************************
*
*                       INITIALIZE
*
***********************************************************/

int led = 9;

unsigned char lastValue[ANA_CHANNELS];
  
void setup()
{
  Serial.begin(SERIAL_BAUD);
  DEBUGln(F("Kubernesys node starting up..."));
  
  refresh = false;
  Timer1.initialize(100000); // set a timer of length 100 microseconds
  Timer1.attachInterrupt( TIM1 ); // attach the service routine here   
  
  pinMode(LED_PIN, OUTPUT);  
  pinMode(INPUT0, INPUT_PULLUP);
  pinMode(INPUT1, INPUT_PULLUP);  

  // Read one time to stabilize the ADC
  for (int i = 0; i < ANA_CHANNELS; i++)
  { 
    lastValue[i] = Analog.get(i); 
  }
  for(int i = 0; i < BUTT_MAX; i++)
  {
    button[i]->update();
  }  
  
  DEBUGln(F("Initializing conf manager.."));
  Conf = new configManager();   
  DEBUGln(F("Initializing protocol.."));  
  protocolInit();
  
  char buff[120];
  sprintf(buff, "Id: %d, Network id: %d, Gateway id: %d, Transmitting at %d Mhz, encryp: %s", Conf->node(), 
                                                                                        Conf->network(), 
                                                                                        Conf->gateway(),  
                                                                                        Conf->frecuency() == RF12_433MHZ ? 433 :  Conf->frecuency()== RF12_868MHZ ? 868 : 915,
                                                                                        Conf->key()); 
  DEBUGln(buff);  
  char last;
  DEBUG(F("Free RAM: "));
  DEBUG( 0x08FF - (int)&last);
  DEBUGln(F(" bytes"));
  
  
  unsigned char * snd = getBuffer();
  appendEvent(snd, PRO_RESET);
  sendBuffer(snd);
  
  delay(DEBOUNCE);  
  for(int i = 0; i < BUTT_MAX; i++)
  {
    button[i]->update();
  }  
    
  DEBUGln();   
}

/* ****************************************************************
*
*                             LOOP
*
*******************************************************************/



void debugPWM();

void loop()
{
  unsigned char * buff = getBuffer();

  // Checkear si hay eventos de botones
  for(int i = 0; i < BUTT_MAX; i++)
  {
    if( button[i]->update() )
    {      
      appendEvent(buff, PRO_DIGITAL, i, button[i]->fallingEdge() ? 1 : 0 );
    }
  }    
        
    
        
  {
    static bool counter = false;
    if( !counter )
    {
      counter = true;
//      debugPWM(0, 0);       // actual = 0;
//      debugPWM(1, 255);     // 0 -> 1 : 255
//      debugPWM(0, 255);     // 1 -> 0 : 255     
//      debugPWM(4, 1);     // 0 -> 1 : 1
//        debugPWM(1, 255);       // 1 -> 0 : 1
//      debugPWM(255, 255);   // 0 -> 255 : 255
//      debugPWM(0, 255);     // 255 -> 0 : 255
//      debugPWM(255, 0);     // 0 -> 255 : 1
//      debugPWM(0, 1);       // 255 -> 0 : 1
    }  
  }
  
  if( refresh )
  {
    // Check the ADC inputs   
    for (int i = 0; i < ANA_CHANNELS; i++)
    { 
      unsigned char newValue = Analog.get(i);
      if( lastValue[i] != newValue )
      {
        DEBUG(F("ADC"));
        DEBUG(i); 
        DEBUG(F(" value: "));
        DEBUG(newValue);     
        lastValue[i] = newValue;      
        appendEvent(buff, PRO_ADC, i, lastValue[i] );         
        Serial.println();
      }
    }    
  
    PWM.actualize();
    refresh = false;
  }
  
  // The buffer is sent only if there is any data inside.
  sendBuffer(buff); 
  
  checkReceived();
  
  delay(5);
  
}


void debugPWM(unsigned char channel, unsigned char value)
{
      unsigned char snd[] = {1, 'C', 'P', channel, value, 50, '.', 'f'};
      msgReceived(2, snd, sizeof(snd));  
}

void TIM1() 
{
  refresh = true;
}



