#ifndef PROTOCOLO_APP
#define PROTOCOLO_APP

#if ARDUINO >= 100
  #include <Arduino.h> // Arduino 1.0
#else
  #include <WProgram.h> // Arduino 0022
#endif

#include <RFM12B.h>

/* ###################################################################################################  */

#define PRO_VERSION1          1

#define PRO_VERSION           0
#define PRO_DATA              1
#define PRO_TYPE              0
#define PRO_SUBTYPE           1
#define PRO_EOF              'f'     // End Of Frame
#define PRO_MIN_PACKET        2
#define PRO_EOD              '.'     // End Of Data 
#define PRO_EOF_SIZE          1
#define PRO_HEADER_SIZE       1
#define PRO_MAX_PACKET      255
#define PRO_MIN_PACKET        5      // ver XX XX EOD EOF


// Message types
#define PRO_COMMAND          'C'    // Change
#define PRO_GET              'G'    // Get
#define PRO_EVENT            'E'    // Event
#define PRO_RESPONSE         'R'    // Response

// Message subTypes
#define PRO_DIGITAL          'D'    // Digital
#define PRO_TRIAC            'T'    // Triac
#define PRO_PWM              'P'    // PWM
#define PRO_ADC              'A'    // ADC
#define PRO_USART            'S'    // Serial
#define PRO_EEPROM           'E'    // EERPOM
#define PRO_FRECUENCY        'F'    // Frecuency
#define PRO_RESET            'R'    // Reset

// sizes of messages incude the EOD terminator

// Change Digital
#define PRO_CD_NUM        2 
#define PRO_CD_VALUE      3 
#define PRO_CD_EOD        4 
#define PRO_CD_SIZE       5

// Change Triac
#define PRO_CT_NUM          2
#define PRO_CT_VALUE        3
#define PRO_CT_TIME         4
#define PRO_CT_EOD          5
#define PRO_CT_SIZE         6

// Change PWM
#define PRO_CP_NUM            2
#define PRO_CP_VALUE          3
#define PRO_CP_TIME           4
#define PRO_CP_EOD            5
#define PRO_CP_SIZE           6

// Write to Serial
#define PRO_CS_LEN          2  // offset of USART len
#define PRO_CS_BUFF         3  // offset of buffer
#define PRO_CS_EOD          3 
#define PRO_CS_SIZE         4  // must add the len of message

// Write to EEPROM
#define PRO_CE_ADR         2  // offset of EEPROM address
#define PRO_CE_LEN         4  // offset of EEPROM message len#define PRO_CE_EOD        4  // must add the len of message
#define PRO_CE_BUFF        5  // offset of EEPROM address
#define PRO_CE_EOD         6  
#define PRO_CE_SIZE         7  // must add the len of message

// Change Frecuency
#define PRO_CF_VALUE    2
#define PRO_CF_EOD      3
#define PRO_CF_SIZE     4

#define PRO_FRECUENCY_433      0
#define PRO_FRECUENCY_868      1
#define PRO_FRECUENCY_915      2

// Reset the microcontroller
#define PRO_CR_EOD          2
#define PRO_CR_SIZE         3

// Event Digital
#define PRO_ED_NUM        2 
#define PRO_ED_VALUE      3 
#define PRO_ED_EOD        4 
#define PRO_ED_SIZE       5

// Event Triac
#define PRO_ET_NUM        2
#define PRO_ET_VALUE      3 
#define PRO_ET_EOD        4 
#define PRO_ET_SIZE       5

// Event ANALOG
#define PRO_EA_NUM        2 
#define PRO_EA_VALUE      3 
#define PRO_EA_EOD        4
#define PRO_EA_SIZE       5

// Event RESET
#define PRO_ER_EOD        2
#define PRO_ER_SIZE       3

// Event FRECUENCY
#define PRO_EF_NUM        2 
#define PRO_EF_EOD        3
#define PRO_EF_SIZE       4

// Get Digital
#define PRO_GD_NUM        2 
#define PRO_GD_EOD        3 
#define PRO_GD_SIZE       4

// Get Analog
#define PRO_GA_NUM        2 
#define PRO_GA_EOD        3 
#define PRO_GA_SIZE       4

// Get Triac
#define PRO_GT_NUM        2 
#define PRO_GT_EOD        3 
#define PRO_GT_SIZE       4


// Get FRECUENCY
#define PRO_GF_EOD        2 
#define PRO_GF_SIZE       3

bool  protocolInit();

int processChange(const unsigned char * msg);
int processGet(const unsigned char * msg, unsigned char * buff );
int processEvent(const unsigned char * msg);
unsigned char * getBuffer();
int appendEvent(unsigned char * msg, char subType, unsigned char number = 0, unsigned char value = 0);
int appendCommnad(unsigned char * msg, char subType, unsigned char number = 0, unsigned char value = 0, unsigned char time = 0);
int appendGet(unsigned char * msg, char subType, unsigned char number);
int appendResponse(unsigned char * msg, char subType, unsigned char number, unsigned char value = 0);


int sendBuffer(unsigned char * buff);
bool checkReceived();
int msgReceived(int sender, unsigned char * data, int len);


#endif  // PROTOCOLO_APP
