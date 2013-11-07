#ifndef HAL_H
#define HAL_H

#define DIG_CHANNELS  8
#define ANA_CHANNELS  2
#define PWM_CHANNELS  4

class HalDigital
{
public:  
  HalDigital();
  ~HalDigital();
  bool set(int channel, int value);
  bool get(int channel);

private: 
};

//class Triac
//{
//public:    
//  Triac();
//  ~Triac();
//  bool set(int channel, int value, int time);  
//};


class HalPWM
{
public:    
  HalPWM();
  ~HalPWM();
  bool set(int channel, int value, int time); 
  void actualize();
  
  friend class HalDigital; // need to stop the PWM when a digital output is activated.
  
private:
  struct
  {
    unsigned char actual;
    unsigned char final;
    char inc;
  } fade[PWM_CHANNELS];
  
  bool stop(int port);
  
};


class HalADC
{
public:    
  HalADC();
  ~HalADC();
  unsigned char get(int channel);
private:
  char lastValue[ANA_CHANNELS];
};

extern HalDigital Digital;
extern HalADC Analog;
extern HalPWM PWM;

#endif
