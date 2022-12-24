/*
 * IRremote: IRrecvDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>
#include <ETH.h>
#include "AsyncUDP.h"


AsyncUDP udp;


int RECV_PIN = 2;

IRrecv irrecv(RECV_PIN);

decode_results results;


static bool eth_connected = false;
static bool udp_connected = false;

void Net_Event(WiFiEvent_t event)
{
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("Cool IR Shtuff");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      if (udp.connect(IPAddress(192,168,1,101), 48630)) {
        udp_connected = true;
      }else {
        udp_connected = false;
      }
      
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      udp_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      udp_connected = false;
      break;
    default:
      break;
  }
}




void setup()
{
  Serial.begin(115200);
  // In case the interrupt driver crashes on setup, give a clue
  // to the user what's going on.
  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");

  Serial.println("Starting Ethernet");
  WiFi.onEvent(Net_Event);
  ETH.begin();
  Serial.println("Started Ethernet");
  
}

void loop() {
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    if (eth_connected && udp_connected)  {
      int volume = 65535;
      for (int i=314; i<=384; i=i+10) {
        Serial.println("Sending Network Command");
        udp.printf("CS %d %d\r", i, volume);
      }
    }
    irrecv.resume(); // Receive the next value
    
  }
  delay(100);
}
