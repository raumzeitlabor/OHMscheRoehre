#ifndef CONTROLLER_H
#define CONTROLLER_H

#define STATIC 0

#include <FastSPI_LED2.h>
#include "artnet.h"

#define NUM_LEDS 150

class CController
{
  public:
    CController();

    void    Initialize();
    void    Process();
    void    HandlePacket(byte ip[4], uint16_t port, uint8_t* data, uint16_t len);
    void    Transmit(uint8_t* data, uint16_t size, uint16_t sourceport, const uint8_t* destip, uint16_t destport);
    void    OnDmxData(uint8_t* data, uint16_t channels);
    void    OnValidData();
    uint8_t manualOverride();
    void    doManualOverride();

    uint8_t  m_brightness;
    uint16_t m_preset;
    
  private:
    void    SetPortAddressFromIp();

    CArtNet  m_artnet;
    CRGB     m_leds[NUM_LEDS];
    uint32_t m_ledshowtime; 
    uint32_t m_validdatatime;
};

#endif //CONTROLLER_H
