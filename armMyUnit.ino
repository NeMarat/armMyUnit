/*
 * TX-RX; RX-TX GND-GND - Air Unit
 * PIN 2; GND - PWM input for ARMing; val > 1650
 * VCC - 5v input from ESC
*/

#define VEHICLE_TYPE 2  //0==ArduPlane, 1==ArduCopter, 2==INAVPlane, 3==INAVCopter. Used for flight modes

#include "MSP.h"
#include "flt_modes.h"

MSP msp;

byte PWM_PIN = 2;
uint16_t pwm_value;
uint16_t prev_pwm_value;
int8_t pwm_reads_count;

uint32_t previousMillis_MSP = 0;
const uint32_t next_interval_MSP = 100;

uint32_t flightModeFlags = 0;
uint32_t general_counter = 0;

uint8_t base_mode = MAV_MODE_PREFLIGHT;
uint8_t system_status = MAV_STATE_UNINIT;
uint32_t custom_mode = 0; //flight mode

msp_status_BF_t status_BF = {0};

void setup() {
    Serial.begin(115200);
    msp.begin(Serial);
    pinMode(PWM_PIN, INPUT);
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
    pwm_value = 1000;
    prev_pwm_value = pwm_value;
    pwm_reads_count = 0;
}

uint16_t read_pwm() {
  pwm_value = pulseIn(PWM_PIN, HIGH);
  if (pwm_value+32 < prev_pwm_value) {
    pwm_reads_count++;
    if (pwm_reads_count > 10) {
      prev_pwm_value=pwm_value;
      pwm_reads_count=0;
    }
  } else {
      if (pwm_value > prev_pwm_value+32) {
        prev_pwm_value=pwm_value;
        pwm_reads_count=0;  
      }
    }
 return(prev_pwm_value);
}

void set_flight_mode_flags(){
  if(read_pwm() > 1650){
    flightModeFlags |= ARM_ACRO_BF;
    digitalWrite(13, HIGH);
  } else {
    flightModeFlags &= ~ARM_ACRO_BF;
    digitalWrite(13, LOW);
  }
}

void send_msp_to_airunit() {
    //MSP_STATUS
    status_BF.flightModeFlags = flightModeFlags;
    msp.send(MSP_STATUS, &status_BF, sizeof(status_BF));
}

void loop() {
  uint32_t currentMillis_MSP = millis();
  if ((uint32_t)(currentMillis_MSP - previousMillis_MSP) >= next_interval_MSP) {
    previousMillis_MSP = currentMillis_MSP;
    set_flight_mode_flags();
    send_msp_to_airunit();
    general_counter += next_interval_MSP;
  }
}
