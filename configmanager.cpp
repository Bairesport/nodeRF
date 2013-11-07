#include "configmanager.h"
#include "globals.h"

#include <SPI.h>
#include <RFM12B.h>
#include <string.h>

SPIFlash       EEPROMflash(8, 0xEF30); //EF30 for Windbond 4mbit flash
  
configManager::configManager() : flash(&EEPROMflash)
{
  if ( flash->initialize() )
  {
    flash->readBytes(ADDRESS, &config, sizeof(config));
  }
  else
 {  
    DEBUGln(F("EEPROM init FAIL!"));
 }

  if( !check() )
  {
    DEBUGln(F("EEPROM seems to be not initialized"));
    initialize();
  }
}

configManager::~configManager()
{
}

bool configManager::check()
{
  return ( config.magicID == SIGNATURE &&
           config.crc     == calculateCRC() );
}
      
int configManager::calculateCRC()
{
  int calculated = 0;
  for( unsigned char * index = (unsigned char *)&config.magicID; index < (unsigned char *) &config.crc; index++)
  {
    calculated += *index;
  }
  
  return calculated;  
}

bool configManager::initialize()
{ 
  DEBUGln(F("Initializing config"));
  config.magicID = SIGNATURE;
  config.version = 1;
  config.node    = NODE;
  config.network = NETWORK;
  config.gateway = GATEWAY;
  config.mode    = MODE_SLAVE;
  config.frecuency = RF12_433MHZ;
  strncpy(config.key, "enunlugardelamancha", MAX_KEY);
  config.key[MAX_KEY-1] = 0;
  update();
  return true;
}

void configManager::update()
{
  DEBUGln(F("Updating EEPROM contents"));
  config.crc     = calculateCRC();
  flash->blockErase4K(ADDRESS);
  while(flash->busy()); 
  flash->writeBytes(ADDRESS, &config, sizeof(config)); 
  while(flash->busy());  
}

int configManager::node()      { return config.node; }
int configManager::network()   { return config.network; }
int configManager::mode()      { return config.mode; }
int configManager::gateway()   { return config.gateway; }
int configManager::frecuency() { return config.frecuency; }
char * configManager::key()    { return config.key; }
  
void configManager::setNode(int node)        { config.node = node; update(); }
void configManager::setNetwork(int net)      { config.network = net; update(); }
void configManager::setMode(int mode)        { config.mode = mode; update(); }
void configManager::setGateway(int gateway)  { config.gateway = gateway; update(); }
void configManager::setFrecuency(int frec)   { config.frecuency = frec; update(); }
void configManager::setKey(char * key)       { strncpy(config.key, key, MAX_KEY); update(); }


