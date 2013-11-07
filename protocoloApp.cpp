#include "globals.h"
#include "protocoloApp.h"
#include "transceiver.h"
#include <WirelessHEX.h>
#include "hal.h"

// loop over all the data received to get the messages

transceiver  trans;

bool  protocolInit()
{
  return trans.init(Conf->node(), Conf->frecuency(), Conf->network(), Conf->key()); 
}

bool checkReceived()
{
   return trans.check(); 
}

int msgReceived(int sender, unsigned char * data, int len)
{
  if(len < PRO_MIN_PACKET)
  { 
    DEBUG(F("Packet too small: "));
    DEBUGln(len);
    return -1;
  }
      
  if( data[PRO_VERSION] != PRO_VERSION1 )
  {
    DEBUG(F("Unknown protocol version "));  
    Serial.println(data[PRO_VERSION], HEX);  
    return -1;
  }
  
  unsigned char * buff = getBuffer(); // for a possible response.
  
  unsigned char *msg = &data[PRO_DATA];
  
  while( msg[PRO_TYPE] != PRO_EOF && 
         msg - data < len )
  {
    switch( msg[PRO_TYPE] )
    {
    case PRO_COMMAND:
      DEBUG(F(" received CHANGE "));
      msg += processChange(msg);
      break;

    case PRO_GET:
      DEBUG(F("received GET "));  
      msg += processGet(msg, buff );       
      break;

    case PRO_EVENT:
      DEBUG(F("received EVENT "));  
      msg += processEvent(msg);  
      break;
      
    case PRO_RESPONSE:
      DEBUG(F("received RESPONSE "));  
      msg += processEvent(msg);  
      break;

    default:
      DEBUG(F("unknown msg received ["));
      DEBUG(len);
      DEBUG("]: ");
      for (int i=0; i < (len - (msg - data)) ; i++)
      {
        if( msg[i] <= 9)
        {
          DEBUG((char)(msg[i] + '0'));
        }
        else
        {
          DEBUG((char)msg[i]);
        } 
        DEBUG(" ");        
      }
      DEBUGln();
      return -1;
      break;
    }
  }
  
  sendBuffer(buff); // will send only if there is any data.  
  return 0;
}

/***************************************************************
 *
 *                     CHANGE
 *
 ***************************************************************/
int processChange(const unsigned char * msg)
{
  int ret;
  int len;
  unsigned char subType = msg[PRO_SUBTYPE];  

  switch(subType)
  {
  case  PRO_DIGITAL:
    DEBUG(F("digital "));  
    if( msg[PRO_CD_EOD] == PRO_EOD )
    {
      int output = msg[PRO_CD_NUM];
      int value = msg[PRO_CD_VALUE];
      DEBUG(F("# "));  
      DEBUG(output);
      DEBUG(F(" to "));
      DEBUGln(value);
      Digital.set(output, value);
    }
    else
    {
      DEBUG(F("without EOD."));    
    }
    ret = PRO_CD_SIZE;
    break;

  case  PRO_TRIAC:
     DEBUG("triac ");   
    if( msg[PRO_CT_EOD] == PRO_EOD )
    {
      int output = msg[PRO_CT_NUM];
      int value = msg[PRO_CT_VALUE];
      int time = msg[PRO_CT_TIME];
      DEBUG(F("#"));  
      DEBUG(output);
      DEBUG(F(" to "));
      DEBUG(value);
      DEBUG(F(" in "));
      DEBUG(time/10.0);
      DEBUGln(F(" sec."));
      
      // Triac.Set(output, value, time);      
    }  
    else
    {
      DEBUG(F("without EOD."));  
    }    
    ret = PRO_CT_SIZE;
    break;      

  case  PRO_PWM:
    DEBUG(F("pwm "));    
    if( msg[PRO_CP_EOD] == PRO_EOD )
    {
      int output = msg[PRO_CP_NUM];
      int value = msg[PRO_CP_VALUE];
      int time = msg[PRO_CP_TIME];
      DEBUG(F("#"));  
      DEBUG(output);
      DEBUG(F(" to "));
      DEBUG(value);
      DEBUG(F(" in "));
      DEBUG(time/10.0);
      DEBUGln(F(" sec."));    
      PWM.set(output, value, time);        
    }
    else
    {
      DEBUG(F("without EOD."));   
    }    
    ret = PRO_CP_SIZE;
    break;    

  case  PRO_USART:
    DEBUG(F("serial "));   
    len = msg[PRO_CS_LEN];
    if( msg[PRO_CS_EOD + len] == PRO_EOD )
    {
      DEBUG(F("of "));  
      DEBUG(len);
      DEBUGln(F(" bytes "));     
      Serial.write( (const uint8_t*) msg[PRO_CS_BUFF], len);
    }
    else
    {
      DEBUG(F("without EOD.")); 
    }    
    
    ret = PRO_CS_SIZE + len;
    break;        

  case  PRO_EEPROM:
    DEBUG(F("eeprom "));   
    len = msg[PRO_CE_LEN];  
    if( msg[PRO_CE_EOD + len] == PRO_EOD )
    {
      int address = ((short *)msg)[PRO_CE_ADR];
      int len     = msg[PRO_CE_LEN];
      DEBUG(F("address "));  
      DEBUG(address);
      DEBUG(F(" of "));  
      DEBUG(len);      
      DEBUG(F(" bytes "));  
    }
    else
    {
      DEBUG(F("without EOD."));    
    }    
    ret = PRO_CE_SIZE + len;
    break;        

  case  PRO_FRECUENCY:
    DEBUG(F("frecuency "));    
    if( msg[PRO_CF_EOD] == PRO_EOD )
    {
      switch(msg[PRO_CF_VALUE])
      {
        case PRO_FRECUENCY_433:
            Conf->setFrecuency(RF12_433MHZ);
            trans.init(Conf->node(), Conf->frecuency(), Conf->network(), Conf->key()); 
            DEBUG(F("to 433Mhz "));  
            break;

        case PRO_FRECUENCY_868:
            Conf->setFrecuency(RF12_868MHZ);
            trans.init(Conf->node(), Conf->frecuency(), Conf->network(), Conf->key()); 
            DEBUG(F("to 868Mhz "));  
            break;        

        case PRO_FRECUENCY_915:
            Conf->setFrecuency(RF12_915MHZ);
            trans.init(Conf->node(), Conf->frecuency(), Conf->network(), Conf->key());       
            DEBUG(F("to 915Mhz "));  
            break;

        default:
            DEBUG(F("to unknown frecuency: "));
            DEBUGln(msg[PRO_CF_VALUE]);
      }
    }
    else
    {
      DEBUG(F("without EOD."));   
    }    
    ret = PRO_CF_SIZE;
    break;        

  case  PRO_RESET:
    DEBUG(F("reset "));    
    if( msg[PRO_CR_EOD] == PRO_EOD )
    {
      DEBUG(F("now."));  
      resetUsingWatchdog(true);
    }
    else
    {
      DEBUG(F("without EOD."));    
    }    
    ret = PRO_CR_SIZE;
    break;
    
  default:  
    DEBUG(F("unknown: "));
    DEBUG(subType);   
    ret = 2;
  }

  return ret;
}


/****************************************************************
 *
 *                             GET
 *
 ***************************************************************/
int processGet( const unsigned char * msg, unsigned char * buff )
{
  int ret;
  unsigned char subType = msg[PRO_SUBTYPE];   

  switch(subType)
  {
  case  PRO_DIGITAL:
    DEBUG(F("digital "));  
    if( msg[PRO_GD_EOD] == PRO_EOD )
    {
      int channel = msg[PRO_GD_NUM];
      DEBUG(F("# "));  
      DEBUG(channel);
      int value = Digital.get(channel);
      appendResponse(buff, PRO_DIGITAL, channel, value);
    }
    else
    {
      DEBUG(F("without EOD."));    
    }
    ret = PRO_GD_SIZE;  
    break;
  case  PRO_TRIAC:
    break;      
  case  PRO_PWM:
    break;                      
  case  PRO_EEPROM:
    break;        
  case  PRO_FRECUENCY:
    DEBUG(F("Frecuency "));  
    if( msg[PRO_GF_EOD] == PRO_EOD )
    {
      appendResponse(buff, PRO_FRECUENCY, Conf->frecuency());
    }
    else
    {
      DEBUG(F("without EOD."));    
    }
    ret = PRO_GF_SIZE;   
    break; 
    
  case  PRO_ADC:
    DEBUG(F("ADC "));  
    if( msg[PRO_GA_EOD] == PRO_EOD )
    {
      int channel = msg[PRO_GD_NUM];
      DEBUG(F("# "));  
      DEBUG(channel);
      int value = Analog.get(channel);
      appendResponse(buff, PRO_ADC, channel, value);
    }
    else
    {
      DEBUG(F("without EOD."));    
    }
    ret = PRO_GA_SIZE; 
    break;  
        
  default:  
    DEBUG(F("Unknown GET msg: "));
    DEBUGln(subType);             
  }    
}


/********************************************************************
 *
 *                      EVENT
 *
 ********************************************************************/
int processEvent( const unsigned char * msg )
{
  int ret;
  unsigned char subType = msg[PRO_SUBTYPE];  

  switch(subType)
  {
  case  PRO_DIGITAL:
    DEBUG(F("digital "));    
    if( msg[PRO_ED_EOD] == PRO_EOD )
    {
      int output = msg[PRO_ED_NUM];
      int value  = msg[PRO_ED_VALUE];
      DEBUG(F("#"));
      DEBUG(output);
      DEBUG(F(" to "));
      DEBUGln(value);
    }
    else
    {
      DEBUG(F("without EOD."));    
    }     
    ret = PRO_ED_SIZE;  
    break;
    
  case  PRO_TRIAC:
    DEBUG(F("triac "));  
    if( msg[PRO_ET_EOD] == PRO_EOD )
    {
      int output = msg[PRO_ET_NUM];
      int value  = msg[PRO_ET_VALUE];
      DEBUG(F("#"));
      DEBUG(output);
      DEBUG(F(" to "));
      DEBUGln(value);
    }
    else
    {
      DEBUG("without EOD.");    
    }     
    ret = PRO_ET_SIZE;  
    break;      

  case  PRO_ADC:
    DEBUG("adc ");  
    if( msg[PRO_EA_EOD] == PRO_EOD )
    {
      int output = msg[PRO_EA_NUM];
      int value  = msg[PRO_EA_VALUE];
      DEBUG("#");
      DEBUG(output);
      DEBUG(" to ");
      DEBUGln(value);
    }
    else
    {
      DEBUG(F("without EOD."));   
    }     
    ret = PRO_EA_SIZE;   
    break; 
    
  case  PRO_EEPROM:
    DEBUG(F("eeprom ???"));    
    break;        
  case  PRO_FRECUENCY:
    DEBUG(F("frecuency ???"));      
    break;
    
  case  PRO_USART:
    DEBUG(F("serial ???")); 
    break;
    
  case  PRO_RESET:
    DEBUG(F("reset."));    
    if( msg[PRO_ER_EOD] == PRO_EOD )
    {
      ;
    }
    else
    {
      DEBUG(F("without EOD."));   
    }     
    ret = PRO_ED_EOD + 1;  
    break;
        
  default:  
    DEBUG(F("unknown: "));
    DEBUG(subType);             
  } 
  
  return ret;
}



/* *********************************************************************
 *
 *                   BUFFER MANAGEMENT
 *
 ***********************************************************************/
unsigned char _RFM12Bbuff_[PRO_MAX_PACKET];  
unsigned char * getBuffer()
{
  _RFM12Bbuff_[PRO_VERSION]        = PRO_VERSION1;
  _RFM12Bbuff_[PRO_HEADER_SIZE]    = PRO_EOF;   
  _RFM12Bbuff_[PRO_HEADER_SIZE +1] = 0; 
  return _RFM12Bbuff_;  
}


unsigned char * findEOF(unsigned char * msg)
{
  // find the last EOF mark to append the new msg.
  int i;
  for(i = 0; i < PRO_MAX_PACKET; i++)
  {
    if( msg[i] == PRO_EOF && msg[i+1] != PRO_EOF)
    {
      return msg + i;
    }
  }
    
  DEBUGln(F("EOF not found"));
  return NULL;
}

int appendCommnad(unsigned char * msg, char subType, unsigned char number, unsigned char value, unsigned char time)
{
  int size = 0;
  msg = findEOF(msg);
  if( !msg )
  {
    return -1;
  }
  
  msg[PRO_TYPE]    = PRO_COMMAND;
  msg[PRO_SUBTYPE] = subType;
  
  switch(subType)
  {
    case PRO_DIGITAL:
      msg[PRO_CD_NUM]     = number;
      msg[PRO_CD_VALUE]   = value; 
      msg[PRO_CD_EOD]     = PRO_EOD; 
      msg[PRO_CD_EOD+1]   = PRO_EOF;       
      size = PRO_HEADER_SIZE + PRO_CD_SIZE + PRO_EOF_SIZE;        
      break;
    
    case PRO_TRIAC:
      msg[PRO_CT_NUM]     = number;
      msg[PRO_CT_VALUE]   = value; 
      msg[PRO_CT_TIME]    = time; 
      msg[PRO_CT_EOD]     = PRO_EOD; 
      msg[PRO_CT_EOD+1]   = PRO_EOF;         
      size = PRO_HEADER_SIZE + PRO_CT_SIZE + PRO_EOF_SIZE;        
      break;

    case PRO_PWM:
      msg[PRO_CP_NUM]     = number;
      msg[PRO_CP_VALUE]   = value; 
      msg[PRO_CP_TIME]    = time; 
      msg[PRO_CP_EOD]     = PRO_EOD; 
      msg[PRO_CP_EOD+1]   = PRO_EOF;         
      size = PRO_HEADER_SIZE + PRO_CP_SIZE + PRO_EOF_SIZE;  
      break;

    case PRO_RESET: 
      msg[PRO_ER_EOD]     = PRO_EOD; 
      msg[PRO_ER_EOD+1]   = PRO_EOF;       
      size = PRO_HEADER_SIZE + PRO_CR_SIZE + PRO_EOF_SIZE;
      break;
      
    default:
      DEBUG(F("Unknow command "));
      DEBUG(subType);
      DEBUGln(F(" to send."));
  }
  
}


//  version type subType num value EOD EOF
int appendEvent(unsigned char * msg, char subType, unsigned char number, unsigned char value)
{
  int size = 0;
  msg = findEOF(msg);
  if( !msg )
  {
    return -1;
  }
 
  msg[PRO_TYPE]    = PRO_EVENT;
  msg[PRO_SUBTYPE] = subType;
  
  switch(subType)
  {
    case PRO_DIGITAL:
      msg[PRO_ED_NUM]     = number;
      msg[PRO_ED_VALUE]   = value; 
      msg[PRO_ED_EOD]     = PRO_EOD; 
      msg[PRO_ED_EOD+1]   = PRO_EOF;       
      size = PRO_HEADER_SIZE + PRO_ED_SIZE + PRO_EOF_SIZE;        
      break;
    
    case PRO_TRIAC:
      msg[PRO_ET_NUM]     = number;
      msg[PRO_ET_VALUE]   = value; 
      msg[PRO_ET_EOD]     = PRO_EOD; 
      msg[PRO_ET_EOD+1]   = PRO_EOF;         
      size = PRO_HEADER_SIZE + PRO_ET_SIZE + PRO_EOF_SIZE;        
      break;

    case PRO_ADC:
      msg[PRO_EA_NUM]     = number;
      msg[PRO_EA_VALUE]   = value; 
      msg[PRO_EA_EOD]     = PRO_EOD; 
      msg[PRO_EA_EOD+1]   = PRO_EOF;       
      size = PRO_HEADER_SIZE + PRO_EA_SIZE + PRO_EOF_SIZE;
      break;

    case PRO_RESET: 
      msg[PRO_ER_EOD]     = PRO_EOD; 
      msg[PRO_ER_EOD+1]   = PRO_EOF;       
      size = PRO_HEADER_SIZE + PRO_ER_SIZE + PRO_EOF_SIZE;
      break;
      
    default:
      DEBUG(F("Unknow event "));
      DEBUG(subType);
      DEBUGln(F(" to send."));
  }
}

//  version type subType num value EOD EOF
int appendGet(unsigned char * msg, char subType, unsigned char number)
{
  int size = 0;
  msg = findEOF(msg);
  if( !msg )
  {
    return -1;
  }
 
  msg[PRO_TYPE]    = PRO_GET;
  msg[PRO_SUBTYPE] = subType;
  
  switch(subType)
  {
    case PRO_DIGITAL:
      msg[PRO_GD_NUM]     = number;
      msg[PRO_GD_EOD]     = PRO_EOD; 
      msg[PRO_GD_EOD+1]   = PRO_EOF;       
      size = PRO_HEADER_SIZE + PRO_GD_SIZE + PRO_EOF_SIZE;        
      break;
    
    case PRO_TRIAC:
      msg[PRO_GT_NUM]     = number;
      msg[PRO_GT_EOD]     = PRO_EOD; 
      msg[PRO_GT_EOD+1]   = PRO_EOF;         
      size = PRO_HEADER_SIZE + PRO_GT_SIZE + PRO_EOF_SIZE;        
      break;

    case PRO_ADC:
      msg[PRO_GA_NUM]     = number;
      msg[PRO_GA_EOD]     = PRO_EOD; 
      msg[PRO_GA_EOD+1]   = PRO_EOF;       
      size = PRO_HEADER_SIZE + PRO_GA_SIZE + PRO_EOF_SIZE;
      break;
      
    default:
      DEBUG(F("Unknow GET "));
      DEBUG(subType);
      DEBUGln(F(" to send."));
  }
}


//  version type subType num value EOD EOF
int appendResponse(unsigned char * msg, char subType, unsigned char number, unsigned char value)
{
  int size = 0;
  msg = findEOF(msg);
  if( !msg )
  {
    return -1;
  }
 
  msg[PRO_TYPE]    = PRO_RESPONSE;
  msg[PRO_SUBTYPE] = subType;
  
  switch(subType)
  {
    case PRO_DIGITAL:
      msg[PRO_ED_NUM]     = number;
      msg[PRO_ED_VALUE]   = value; 
      msg[PRO_ED_EOD]     = PRO_EOD; 
      msg[PRO_ED_EOD+1]   = PRO_EOF;       
      size = PRO_HEADER_SIZE + PRO_ED_SIZE + PRO_EOF_SIZE;        
      break;
    
    case PRO_TRIAC:
      msg[PRO_ET_NUM]     = number;
      msg[PRO_ET_VALUE]   = value; 
      msg[PRO_ET_EOD]     = PRO_EOD; 
      msg[PRO_ET_EOD+1]   = PRO_EOF;         
      size = PRO_HEADER_SIZE + PRO_ET_SIZE + PRO_EOF_SIZE;        
      break;

    case PRO_ADC:
      msg[PRO_EA_NUM]     = number;
      msg[PRO_EA_VALUE]   = value; 
      msg[PRO_EA_EOD]     = PRO_EOD; 
      msg[PRO_EA_EOD+1]   = PRO_EOF;       
      size = PRO_HEADER_SIZE + PRO_EA_SIZE + PRO_EOF_SIZE;
      break;
      
    case PRO_FRECUENCY:
      msg[PRO_EF_NUM]     = number;
      msg[PRO_EF_EOD]     = PRO_EOD; 
      msg[PRO_EF_EOD+1]   = PRO_EOF;       
      size = PRO_HEADER_SIZE + PRO_EF_SIZE + PRO_EOF_SIZE;
      break;      
      
    default:
      DEBUG(F("Unknow response "));
      DEBUG(subType);
      DEBUGln(F(" to send."));
  }
}


int sendBuffer(unsigned char * buff)
{
  int len;
  // seek for the EOF to know the packet size
  for(len = 0; len < PRO_MAX_PACKET; len++)
    if( buff[len] == PRO_EOF && buff[len+1] != PRO_EOF)
      break;

  if( len != PRO_MAX_PACKET &&
      len >= PRO_MIN_PACKET-1 )
  {   
    trans.send(Conf->gateway(), buff, len+1 , false);   
  } 
  return len != PRO_MAX_PACKET;
}

