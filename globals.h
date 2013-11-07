#ifndef GLOBALS_H
#define GLOBALS_H

#include "configmanager.h"

//#define NETWORKID    99  //the network ID we are on
//
//#define NODEID        1  //network ID used for this unit
//#define GATEWAYID     2  //the node ID we're sending to

#define ACK_TIME     50  // # of ms to wait for an ack
#define SERIAL_BAUD  115200
#define LED_PIN      9
#define FREQUENCY    RF12_433MHZ //Match this with the version of your Moteino! (others: RF12_433MHZ, RF12_915MHZ)
#define INPUT0       16  // PC2 on Atmel328, D16 on Arduino
#define INPUT1       17  // PC3 on Atmel328, D17 on Arduino
#define ADC_0        6   // ADC6 on Atmel328, ADC6 on Arduino
#define ADC_1        7   // ADC7 on Atmel328, ADC7 on Arduino
#define LED          9   // PB1 on Atmel328, D9 on Arduino


// Hardware
#define DIGITAL      0
#define TRIAC        1
#define ANALOG       2

#define DEBOUNCE     100

//extern configManager *Conf;

#define DEBUG(X)     Serial.print(X)
#define DEBUGln(X)   Serial.println(X)

#endif
