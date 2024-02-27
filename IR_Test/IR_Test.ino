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
#include <sys/time.h>
#include "esp_timer.h"
#include <EEPROM.h>

AsyncUDP udp;


int RECV_PIN = 2;

IRrecv irrecv(RECV_PIN);

decode_results results;


static bool eth_connected = false;
static bool udp_connected = false;

const int vol_down = 551534655;
const int vol_up = 551502015;
const int repeat = 4294967295;
const int mute = 551522415;

#define EEPROM_SIZE 12
int volume;
bool muted;

int now = esp_timer_get_time()/1000;
int reset_time = now + 1000;


int current_command = 0;


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
      if (udp.connect(IPAddress(192,168,2,75), 48630)) {
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

void Set_Volume(int vol){
  int address = 0;
  EEPROM.write(address, vol);
  address += sizeof(vol);
  EEPROM.write(address, muted);
  EEPROM.commit();
  int adjusted_volume = (vol/100.0) * 65535;
  Serial.print("Volume at: ");
  Serial.print(vol);
  Serial.print("%    Raw: ");
  Serial.println(adjusted_volume);
  Serial.println("Sending Network Commands");
  for (int i=314; i<=384; i=i+10) {
    udp.printf("CS %d %d\r", i, adjusted_volume);
  }
}




void setup()
{
  Serial.begin(9600);
  // In case the interrupt driver crashes on setup, give a clue
  // to the user what's going on.
  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("Enabled IRin");

  Serial.println("Starting Ethernet");
  WiFi.onEvent(Net_Event);
  ETH.begin();
  Serial.println("Started Ethernet");
  EEPROM.begin(EEPROM_SIZE);
  int address = 0;
  volume = EEPROM.read(address);
  address += sizeof(volume);
  muted = EEPROM.read(address);
}

void loop() {
  now = esp_timer_get_time()/1000;

  if ( now > reset_time && current_command != 0 ){
    current_command = 0;
  }

  if (irrecv.decode(&results)) {
    Serial.println(results.value);
    reset_time = now + 1000;
    if (eth_connected && udp_connected)  {
    //if (true) {
      if (results.value == repeat && (current_command == vol_up || current_command == vol_down)) {
        results.value = current_command;
      }

      if (results.value == vol_up and volume < 100 ) {
        volume = volume + 1;
        current_command = vol_up;
        muted = 0;
        Set_Volume(volume);
      }
      else if (results.value == vol_down && volume > 0) {
        volume = volume - 1;
        current_command = vol_down;
        muted = 0;
        Set_Volume(volume);
      }
      else if (results.value == mute) {
        if ( muted == 1 ){
          muted = 0;
          Set_Volume(volume);
        }
        else {
          muted = 1;
          Set_Volume(0);
        }

      }

      
      
    }
    irrecv.resume(); // Receive the next value
    
  }
  delay(100);  
}
