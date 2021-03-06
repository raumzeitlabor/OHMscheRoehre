#include <Arduino.h>
#include <avr/wdt.h>
#include <EtherCard.h>
#include "controller.h"
#include "debugprint.h"

byte Ethernet::buffer[600];

#if STATIC
static byte myip[] = { 192,168,1,200 };
static byte gwip[] = { 192,168,1,1 };
#endif

#define MANUAL_OVERRIDE 32
#define MODE_FAST 64
#define COLORMASK 3
#define MODEMASK 28
#define MODE_STATIC 0
#define MODE_BUBBLE 1
#define MODE_FADE 2
#define MODE_BAR_V 3
#define MODE_BAR_H 4

#define LEDSPERTURN 7

uint8_t animationState;
uint8_t animationState2;

extern void Transmit(uint8_t* data, uint16_t size, uint16_t sourceport, const uint8_t* destip, uint16_t destport);

CController::CController() : m_artnet(*this, Ethernet::buffer + UDP_DATA_P, ether.myip, ether.mymac)
{
}

#define DATAPIN 4
#define SELECTPIN 6
#define ETHERRESETPIN 9

void CController::Initialize()
{
  //make the pixel/strip pin an input, and enable the internal pullup
  pinMode(SELECTPIN, INPUT);
  digitalWrite(SELECTPIN, HIGH);
  //let the pin rise if the jumper is open
  delay(10);

    LEDS.addLeds<WS2812B, DATAPIN, GRB>(m_leds, NUM_LEDS); //led strip
    memset(m_leds, 0x10, sizeof(m_leds)); //led strip can't handle full white

  //make all leds white
  LEDS.show();

  //init the led timestamp
  m_ledshowtime = millis();

  //make the reset pin low for 100 ms, to reset the ENC28J60
  pinMode(ETHERRESETPIN, OUTPUT);
  digitalWrite(ETHERRESETPIN, LOW);
  delay(100);
  digitalWrite(ETHERRESETPIN, HIGH);
  delay(100);

  wdt_reset();
  uint8_t mac[] = { 0x70,0x69,0x69,0x2D,0x30,0x31 };
  if (ether.begin(sizeof(Ethernet::buffer), mac))
  {
    DBGPRINT("Ethernet controller set up\n");
  }
  else
  {
    DBGPRINT("Failed to set up Ethernet controller\n");
    for(;;); //wait for the watchdog timer reset
  }

  //delay for the ethernet switch to bring up stuff
  wdt_reset();
  delay(5000);
  wdt_reset();

  //enable broadcast for dhcp and art-net
  ether.enableBroadcast();

#if STATIC
  ether.staticSetup(myip, gwip);
#else
  DBGPRINT("Requesting ip address using DHCP\n");

  //try dhcp 5 times before doing a watchdog timer reset
  for (uint8_t i = 0; i < 5; i++)
  {
    if (ether.dhcpSetup())
    {
      DBGPRINT("DHCP succeeded\n");
      break;
    }
    else
    {
      DBGPRINT("DHCP failed\n");
      if (i == 4)
        for(;;); //wait for the watchdog timer reset
    }
  }
#endif

  DBGPRINT("IP: %i.%i.%i.%i\n", ether.myip[0], ether.myip[1], ether.myip[2], ether.myip[3]);
  DBGPRINT("GW: %i.%i.%i.%i\n", ether.gwip[0], ether.gwip[1], ether.gwip[2], ether.gwip[3]);
  DBGPRINT("DNS: %i.%i.%i.%i\n", ether.dnsip[0], ether.dnsip[1], ether.dnsip[2], ether.dnsip[3]);
  DBGPRINT("MASK: %i.%i.%i.%i\n", ether.mymask[0], ether.mymask[1], ether.mymask[2], ether.mymask[3]);

  SetPortAddressFromIp();
  EtherCard::dhcp_renewed = false;
  m_artnet.Initialize();
  wdt_reset();

  //init last valid data timestamp
  m_validdatatime = millis();
}

void CController::SetPortAddressFromIp()
{
  //store the ip address and subnetmask into uint32_t
  uint32_t address = 0;
  uint32_t mask = 0;
  for (uint8_t i = 0; i < 4; i++)
  {
    address |= (uint32_t)ether.myip[i] << (3 - i) * 8;
    mask |= (uint32_t)ether.mymask[i] << (3 - i) * 8;
  }

  //do a bitwise and with the subnetmask to get the host address
  uint32_t hostaddress = address & ~mask;
  //subtract one from the host address, take the 15 least significant bits, and use it as the art-net portaddress
  uint16_t portaddress = (hostaddress - 1) & 0x3FFF;

  m_artnet.SetPortAddress(portaddress);
  //set the ArtPollReply delay so that if a lot of these controllers are on the network,
  //and their ip addresses are numbered sequentually,
  //a controller will send ArtPollReply every 2 milliseconds
  //this prevents the ENC28J60 buffer filling up with a shitload of ArtPollReply broadcasts
  uint16_t delay = (hostaddress % 1000) * 2;
  m_artnet.SetPollReplyDelay(delay);

  DBGPRINT("address:%lu\n", address);
  DBGPRINT("mask:%lu\n", mask);
  DBGPRINT("hostaddress:%lu\n", hostaddress);
  DBGPRINT("portaddress:%u\n", portaddress);
  DBGPRINT("net:%i subnet:%i universe:%i\n", (portaddress >> 8) & 0xFF, (portaddress >> 4) & 0xF, portaddress & 0xF);
  DBGPRINT("artpollreply delay: %i\n", delay);
}

uint8_t CController::manualOverride () {
   if ((m_preset & MANUAL_OVERRIDE) == MANUAL_OVERRIDE) {
     return true;
   } else {
     return false;
   }
}

void CController::doManualOverride () {
  uint32_t mode;
  uint8_t j,t;
  CRGB color;
  
  switch (m_preset & COLORMASK) {
     case 0: color = CRGB::Blue; break;
     case 1: color = CRGB::Red; break;
     case 2: color = CRGB::Yellow; break;
     case 3: color = CRGB::Green; break;
  }
  
  Serial.print(m_brightness);
  Serial.print(" ");
  color.r = (float)((float)m_brightness/128) * (float)color.r;
  color.g = (float)((float)m_brightness/128) * (float)color.g;
  color.b = (float)((float)m_brightness/128) * (float)color.b;
  
  Serial.println(color.r);
  mode = (m_preset & MODEMASK) >> 2;
  
//  Serial.println(mode);
  
  switch (mode) {
     case MODE_STATIC:
      for (j=0;j<NUM_LEDS;j++) {
        m_leds[j] = color;
      }
      break;
     case MODE_BUBBLE:
//       // Red Bubbles are low, yellow bubbles are medium, green bubbles are high. Some kind of progress bar
       switch (m_preset & COLORMASK) {
         case 1:
           t = 100;
           break;
         case 2:
           t = 70;
           break;
         case 3:
           t = 40;
           break;
         default:
           t = 0;
       }

       for (j=NUM_LEDS-1;j>t;j--) {
         m_leds[j] = color;
       }
       
       if (t>0) {
       for (j=0;j<5;j++) {
         m_leds[random(t)] = color; 
       }
       }
       break;
     case MODE_FADE:
       animationState++;
       if (animationState > NUM_LEDS) {
        animationState = 0; 
        animationState2 = !animationState2;
       }

      if (animationState2) {
       for (j=0;j<NUM_LEDS-animationState;j++) {
         m_leds[j] = color;
       } 
      } else {
        for (j=(NUM_LEDS-animationState);j<NUM_LEDS;j++) {
         m_leds[j] = color;
       }
       
      }
     break;
     case MODE_BAR_V:
       animationState++;
       
       if (animationState > (NUM_LEDS/LEDSPERTURN)) {
          animationState = 0; 
       }
       
         for (j=0;j<LEDSPERTURN;j++) {
            m_leds[j+((21-animationState)*LEDSPERTURN)] = color;                  
         }
     break;
     case MODE_BAR_H:
       animationState++;
       
       if (animationState > LEDSPERTURN-1) {
          animationState = 0; 
       }
       
       for (j=animationState;j<NUM_LEDS;j+=LEDSPERTURN) {
        m_leds[j] = color; 
       }
     break;
  }
 
  LEDS.show();
  wdt_reset();
  
  if ((m_preset & MODE_FAST) == MODE_FAST) {
    delay(1);
  } else {
    delay(50); 
  }
}

void CController::Process()
{
  if (manualOverride()) {
      memset(m_leds, 0x00, sizeof(m_leds));
     doManualOverride();
    return;
  }
  
  if (EtherCard::dhcp_renewed)
  {
    //possibly new ip address, reset port address
    SetPortAddressFromIp();
    m_artnet.Initialize();
    EtherCard::dhcp_renewed = false;
  }
  
  uint32_t now = millis();

  //if valid art-net data has been received in the last minute,
  //reset the watchdog timer
//  if (now - m_validdatatime < 60000)
    wdt_reset();

  m_artnet.Process(now);

  //transmit data to the leds at least once per second, to make sure they stay on
  if (now - m_ledshowtime >= 1000)
  {
    LEDS.show();
    m_ledshowtime = now;
  }

  
}

void CController::HandlePacket(byte ip[4], uint16_t port, uint8_t* data, uint16_t len)
{
  m_artnet.HandlePacket(ip, port, data, len);
}

void CController::Transmit(uint8_t* data, uint16_t size, uint16_t sourceport, const uint8_t* destip, uint16_t destport)
{
  ::Transmit(data, size, sourceport, destip, destport);
}

void CController::OnDmxData(uint8_t* data, uint16_t channels)
{
  memcpy(m_leds, data, min(channels, sizeof(m_leds)));
  LEDS.show();
  m_ledshowtime = millis();
}

void CController::OnValidData()
{
  m_validdatatime = millis();
  wdt_reset();
}

