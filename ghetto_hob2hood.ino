
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <IRsend.h>
#include <iostream>
#include "config.h"

IRrecv irrecv(RECV_PIN);
IRsend irsend(IR_LED);  // Set the GPIO to be used to sending the message.

// IR commands from AEG hob2hood device
const uint32_t IRCMD_VENT_1 = 0xE3C01BE2;     //Hob2hood On (level 1)
const uint32_t IRCMD_VENT_2 = 0xD051C301;     //Hob2hood level 2
const uint32_t IRCMD_VENT_3 = 0xC22FFFD7;     //Hob2hood level 3
const uint32_t IRCMD_VENT_4 = 0xB9121B29;     //Hob2hood level 4
const uint32_t IRCMD_VENT_OFF = 0x55303A3;    //Hob2hood off
const uint32_t IRCMD_LIGHT_ON = 0xE208293C;   //Light on (Hood on)
const uint32_t IRCMD_LIGHT_OFF = 0x24ACF947;  //Light off (Automatic after 2min)

// Faber hood codes
uint16_t faber_power[] = {868,606,816,1296,1534,636,818,614,816,1296,1534,2000,818,1296,818,612,1540};
uint16_t faber_light[] = {862, 614, 780, 676, 1502, 1322, 808, 622, 810, 646, 1504, 2684, 808, 622, 808, 648, 2228};
uint16_t faber_intense[] = {872, 604, 1538, 1312, 820, 612, 820, 610, 1538, 1312, 846, 1948, 1562, 1968, 1542};
uint16_t faber_up[] = {842, 634, 2258, 652, 814, 616, 816, 618, 2282, 626, 812, 2000, 2256, 1332, 1540};
uint16_t faber_down[] = {836, 1322, 1556, 614, 784, 646, 810, 1302, 1526, 642, 810, 2666, 1558, 1294, 1532};


//status vars
int light = 0;
int last_light = 0;
int current_vent = 0;
int target_vent = 0;

decode_results results;

void to_faber(uint16_t signal_data[]) {
  irsend.sendRaw(signal_data, sizeof(signal_data)/sizeof(signal_data[0]), 38);  // Send a raw data capture at 38kHz.
}

void setup(void){
  Serial.begin(115200);
  irrecv.enableIRIn();  // Start the receiver
  irsend.begin(); // Start IR sender
  while (!Serial)  // Wait for the serial connection to be establised.
    delay(50);
  Serial.println();
  Serial.print("IRrecvDemo is now running and waiting for IR message on Pin ");
  Serial.println(RECV_PIN);
  to_faber(faber_power);
}

void loop() {
  if (irrecv.decode(&results)) {
    switch (results.value) {
      case IRCMD_LIGHT_ON:
        light = 1;
        break;
        
      case IRCMD_LIGHT_OFF:
        light = 0;
        break;

      case IRCMD_VENT_1:
        target_vent = 1;
        break;

      case IRCMD_VENT_2:
        target_vent = 2;
        break;

      case IRCMD_VENT_3:
        target_vent = 3;
        break;

      case IRCMD_VENT_4:
        target_vent = 4;
        break;

      case IRCMD_VENT_OFF:
        target_vent = 0;
        break;

      default:
        break;
    }
    controlHood();
    irrecv.resume();
        
  }
}


void controlHood() {

  bool logLight = light!=last_light;
  bool logVent = target_vent!=current_vent;
  
  // control light
  switch (light) {
    // Light OFF
    case 0:
      if (logLight) {
        Serial.println("Light: OFF");
        irsend.sendRaw(faber_light, sizeof(faber_light)/sizeof(faber_light[0]), 38);
        delay(10);
      };
      
      break;
    // Light ON
    case 1:
      if (logLight) {
        Serial.println("Light: ON");
        irsend.sendRaw(faber_light, sizeof(faber_light)/sizeof(faber_light[0]), 38);
        delay(10);
      };
  };

  if (logVent) {
    while(current_vent != target_vent) {
      if (current_vent < target_vent) {
        if (current_vent == 0) {
          Serial.println("\nTurning ON\n");
          irsend.sendRaw(faber_power, sizeof(faber_power)/sizeof(faber_power[0]), 38);
          current_vent = 1;
        } else if (target_vent == 4) {
            Serial.println("\nTurning TURBO (lvl 4)\n");
            irsend.sendRaw(faber_intense, sizeof(faber_intense)/sizeof(faber_intense[0]), 38);
            current_vent = 4;
        } else {
            Serial.println("\nVent UP\n");
            irsend.sendRaw(faber_up, sizeof(faber_up)/sizeof(faber_up[0]), 38);
            current_vent++;
        };
      };
      
      if (current_vent > target_vent) {
        if (target_vent == 0) {
          Serial.println("\nTurning OFF\n");
          irsend.sendRaw(faber_power, sizeof(faber_power)/sizeof(faber_power[0]), 38);
          current_vent = 0;
        } else {
            Serial.println("\nVent DOWN\n");
            irsend.sendRaw(faber_down, sizeof(faber_down)/sizeof(faber_down[0]), 38);
            current_vent--;
        };
      };
      delay(300);
    };
  };

  last_light = light;
  
};
