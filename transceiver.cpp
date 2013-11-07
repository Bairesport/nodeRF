#include "transceiver.h"
#include "configmanager.h"
#include "globals.h"
#include "protocoloApp.h"
#include <SPIFlash.h>
#include <WirelessHEX.h>


extern SPIFlash EEPROMflash;

transceiver::transceiver()
{
}

transceiver::~transceiver()
{
}


bool transceiver::init(int node, int frec, int network, char * key)
{
  radio.Initialize(node, frec, network);  
  radio.Encrypt((uint8_t*)key);
  radio.Sleep();
}

int transceiver::send(int destID, unsigned char * payload, int sendSize, bool requestACK)
{
  //digitalWrite(LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  DEBUG("Sending[");
  DEBUG(sendSize);
  DEBUG("]:");
  for(byte i = 0; i < sendSize; i++)
  {
    if( payload[i] <= 9)
    {
      Serial.print((char)(payload[i] + '0'));
    }
    else
    {
      Serial.print((char)payload[i]);
    } 
    DEBUG(" ");
  }
  DEBUGln();
  
  radio.Wakeup();  
  radio.Send(destID, payload, sendSize, requestACK);
  if (requestACK)
  {
    DEBUG(" - waiting for ACK...");
    if (waitForAck()) DEBUG("ok!");
    else DEBUG("error: no ACK.");
  }
  radio.Sleep();
  DEBUGln();
  //digitalWrite(LED_PIN, LOW);   // turn the LED on (HIGH is the voltage level)  
  return 0;   
}

bool transceiver::check()
{
  bool ret = false;
  // Checkear si se recibio algo
  if (radio.ReceiveComplete())
  {
    if (radio.CRCPass())
    {
      ret = true;
      DEBUG("Node id ");
      Serial.print(radio.GetSender());
      Serial.print(" says: ");
      for (byte i = 0; i < *radio.DataLen; i++) //can also use radio.GetDataLen() if you don't like pointers
      {
        Serial.print(radio.Data[i], HEX);
        DEBUG(" ");
      }
   
      DEBUGln();
      
      CheckForWirelessHEX(radio, EEPROMflash, true);
      msgReceived(radio.GetSender(), (unsigned char *) radio.Data, radio.GetDataLen());

      if (radio.ACKRequested())
      {
        radio.SendACK();
        DEBUG(" - ACK sent");
      }
      DEBUGln();   
    }
    else
    {
      DEBUG("BAD-CRC");
    }
  }
  return ret;
}

// wait a few milliseconds for proper ACK, return true if received
bool transceiver::waitForAck() {
  long now = millis();
  while (millis() - now <= ACK_TIME)
    if (radio.ACKReceived(Conf->gateway()))
      return true;
  return false;
}

