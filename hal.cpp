#include "globals.h"
#include "hal.h"

HalDigital Digital;
HalADC     Analog;
HalPWM     PWM;

char DIG_PORTS[DIG_CHANNELS] = {14, 4, 7, 9, 5, 6, 3, 15};
char ANA_PORTS[ANA_CHANNELS] = {6, 7};//, 2, 3, 0};
char PWM_PORTS[PWM_CHANNELS] = {5, 6, 9, 3};

#define TICS_PER_SECOND 100
/* ************************************************
 *                     ADC
 **************************************************/
 
HalDigital::HalDigital()
{
}
  
HalDigital::~HalDigital()
{
}

bool HalDigital::set(int channel, int value)
{
    bool ret = false;  
    DEBUG(F("Digital channel: "));
    Serial.print(channel);  
    
    if( channel >= DIG_CHANNELS)
    {
      DEBUGln(F(" out of range"));
      return false;
    }
      
    DEBUG(F(" set to "));  
    DEBUGln(value);  
    if( channel >= DIG_CHANNELS)
    {
      return false;
    }
      

    int output = DIG_PORTS[channel];
   
    switch(value)
    {
      case 0:
      case 1:
        pinMode(output, OUTPUT);       
        digitalWrite(output, value);
        break;

     default:
       ret = false;
    }
    
    return ret;
}


bool HalDigital::get(int channel)
{
  int port = DIG_PORTS[channel];
 // pinMode(channel, INPUT);     
  return digitalRead(port);
}


/* ************************************************
 *                     ADC
 **************************************************/
HalADC::HalADC()
{
  for(int i = 0; i < ANA_CHANNELS; i++)
  {
    lastValue[i] = analogRead(ANA_PORTS[i]) / 4;  // transform 1023-0 to 255-0    
  } 
}

HalADC::~HalADC()
{
}

unsigned char HalADC::get(int channel)
{
    if( channel >= ANA_CHANNELS)
    {
      return 0;
    }  
    
  char port  = ANA_PORTS[channel];  
  //pinMode(port, INPUT);  
  char value = analogRead(port) / 4; 
  
  if( abs(value - lastValue[channel]) > 5 )
  {
    lastValue[channel] = value;
  }
  else
  {
    lastValue[channel] = (lastValue[channel] + value) / 2;
  }
  
  return lastValue[channel];
}


/************************************************************
 *                            PWM
 ***********************************************************/

HalPWM::HalPWM()
{
  for(int i = 0; i < PWM_CHANNELS; i++)
  {
    fade[i].inc    = 0;
    fade[i].actual = 0;
    fade[i].final  = 0;    
  }  
}
HalPWM::~HalPWM()
{
}

bool HalPWM::set(int channel, int value, int time)
{
    bool ret = false;
    DEBUG(F("PWM channel: "));
    Serial.print(channel);  
    
    if( channel >= PWM_CHANNELS)
    {
      DEBUGln(F(" out of range"));
      return false;
    }
      
    DEBUG(F(", actual: "));
    Serial.print(fade[channel].actual);      
    DEBUG(F(", final: "));
    Serial.print(value);
    
   // noInterrupts();
  
    // setting the same value as actual, stop the fading.  
    if( fade[channel].actual == value)
    {
      DEBUGln(F(", setting the same value as actual. Stop fadding"));      
      fade[channel].final = value;     
      fade[channel].inc = 0;      
      int output = PWM_PORTS[channel];
      analogWrite(output, value); 
      ret = true;
    }
    else if( !time ) // time = 0, set the final value and stop fading.
    {
      DEBUGln(F(", setting value in time = 0. Stop fadding"));          
      fade[channel].final = value;   
      fade[channel].actual = value;   
      fade[channel].inc = 0;      
      int output = PWM_PORTS[channel];
      analogWrite(output, value); 
      ret = true;
    }
    else // fade
    {
      fade[channel].final = value;    
      
      int cant_tics = (time * TICS_PER_SECOND) / 10;
      
      DEBUG(F(", cant tics: "));
      Serial.print(cant_tics);
  
      int inc = round(((fade[channel].final - fade[channel].actual) * 10.0 ) / cant_tics);
      DEBUG(F(", inc: "));
      Serial.print(inc);   
      
      if( inc > 127 )
      {
        fade[channel].inc = 127;
      }
      else if( inc < -127 )
      {
        fade[channel].inc = -127;
      }
      else if( inc == 0 )
      {
          fade[channel].inc = (fade[channel].final > fade[channel].actual) ? 1 : -1;
      }
      else
      {
       fade[channel].inc = inc; 
      }
      
      DEBUG(", corrected inc: ");
      Serial.println(fade[channel].inc);  
      ret = true;    
    }
    
   // interrupts(); 
    return ret;
}

void HalPWM::actualize()
{
  for(int i = 0; i < PWM_CHANNELS; i++)
  {
    if( fade[i].final != fade[i].actual &&
        fade[i].inc   != 0 )
    {   
      DEBUGln();
      DEBUG(F("PWM FADE channel: "));
      DEBUG(i);
      DEBUG(F(" , final: "));
      DEBUG(fade[i].final);
      DEBUG(F(" , actual: "));
      DEBUG(fade[i].actual);
      DEBUG(F(" , inc: "));
      DEBUG((int)fade[i].inc);  
      
      if( fade[i].inc > 0 )
      {
        fade[i].actual += (fade[i].actual + fade[i].inc > fade[i].final) ? (fade[i].final - fade[i].actual) :  fade[i].inc;
      }
      else
      {
        fade[i].actual += (fade[i].actual + fade[i].inc < fade[i].final) ? (fade[i].final - fade[i].actual) :  fade[i].inc;        
      }
      
      DEBUG(F(" , new actual: "));
      DEBUG(fade[i].actual);  
      
      // we reach the final value, stop the fading
      if( fade[i].final == fade[i].actual )
      {
        fade[i].inc = 0;
      }
      
      // set new value
      int output = PWM_PORTS[i];
      analogWrite(output, fade[i].actual);          
    }
  } 
}
  

// stops the PWM fadding in that port.
bool HalPWM::stop(int port)
{
  bool ret = false;
  for(int i = 0; i < PWM_CHANNELS; i++)
  {
    if(PWM_PORTS[i] == port)
   {
     fade[i].final = fade[i].actual;
     fade[i].inc = 0;
     ret = true;
     break;
   } 
  }
  return ret;
}

