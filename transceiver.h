#ifndef TRANSCEIVER_H
#define TRANSCEIVER_H

#include <RFM12B.h>

class transceiver
{
public:
  transceiver();
  ~transceiver();
  bool init(int node, int frec, int network, char * key);
  int  send(int destID, unsigned char * payload, int sendSize, bool requestACK);
  bool  check();

private:
 bool waitForAck();
 
 RFM12B         radio;
};



#endif // RADIO_H
