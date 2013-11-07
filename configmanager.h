#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#if ARDUINO >= 100
  #include <Arduino.h> // Arduino 1.0
#else
  #include <WProgram.h> // Arduino 0022
#endif

#include <SPIFlash.h>

class configManager
{
public:
  configManager();
  ~configManager();
  int node();
  int network();
  int mode();
  int gateway();
  int frecuency();
  char * key();
  
  void setNode(int node);
  void setNetwork(int net);
  void setMode(int mode);
  void setGateway(int gateway);
  void setFrecuency(int frec); 
  void setKey(char * key);
private:
  enum 
  { 
    SIGNATURE  = 0x6076616A,
    NODE       = 1,
    GATEWAY    = 2,
    NETWORK    = 99,    
    ADDRESS    = 32 * 1024,
    MODE_SLAVE = 0,
    MAX_KEY    = 16,
  };
  
  struct nodeConfig_V1
  {
    long    magicID;
    int     version;
    int     node;
    int     network;
    int     mode;
    int     gateway;
    int     frecuency;
    char    key[MAX_KEY];
    int     crc;       
   } config;  
 
  bool check();
  int  calculateCRC();
  bool initialize();
  void update();
  
  SPIFlash  * flash;
};

extern configManager *Conf;

#endif // CONFIGMANAGER_H
