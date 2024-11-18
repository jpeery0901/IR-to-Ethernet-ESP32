/*
 * IRremote: IRrecvDemo - demonstrates receiving IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 */

#include <IRremote.h>
#include <ETH.h>
#include <WiFi.h>
#include "AsyncUDP.h"
#include <sys/time.h>
#include "esp_timer.h"
//#include <EEPROM.h>

AsyncUDP udp;

const IPAddress ip_address(192,168,2,150);
const int port = 9324;

int RECV_PIN = 2;

IRrecv irrecv(RECV_PIN);

decode_results results;


static bool eth_connected = false;
static bool udp_connected = false;

const int vol_down = -66847996;
const int vol_up = -50136316;
const int repeat = 0;
const int mute = -167118076;

//#define EEPROM_SIZE 12
//int volume;
//bool muted;

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
      if (udp.connect(ip_address, port)) {
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

void Send_Command(String cmd){
  Serial.println("Sending Network Commands");
  if (udp_connected) {
    udp.printf("%s", cmd.c_str());
  } else {
    Serial.println("UDP Failed");
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
//   EEPROM.begin(EEPROM_SIZE);
//   int address = 0;
//   volume = EEPROM.read(address);
//   if (volume == 255 || volume < 0 || volume > 100) {
//     // EEPROM is uninitialized, so set a default volume
//     volume = 50; // or whatever default you prefer
//   }
//   address += sizeof(volume);
//   muted = EEPROM.read(address);
}

void loop() {
  now = esp_timer_get_time()/1000;

  if ( now > reset_time && current_command != 0 ){
    current_command = 0;
  }

  if (irrecv.decode()) {
    int ir_command = irrecv.decodedIRData.decodedRawData;
    Serial.println(ir_command);
    reset_time = now + 1000;
    if (eth_connected && udp_connected)  {
    //if (true) {
      if (ir_command == repeat && (current_command == vol_up || current_command == vol_down)) {
        ir_command = current_command;
      }

      if (ir_command == vol_up ) {
        current_command = vol_up;
        Send_Command("vol_up");
      }
      else if (ir_command == vol_down) {
        current_command = vol_down;
        Send_Command("vol_down");
      }
      else if (ir_command == mute) {
        Send_Command("mute_toggle");
      }
      
    }
    irrecv.resume(); // Receive the next value
    
  }
  delay(100);  
}
