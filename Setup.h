#ifndef SETUP_H
#define SETUP_H

#define DEBUG       // zakomentuj před finální kompilací, 

/*************************************************************************
* připojení k Arduinu
*************************************************************************/
const uint8_t NUM_SENSORS = 5;
const uint8_t SENSOR[NUM_SENSORS] = {14, 15, 16, 17, 18};   // IR snímače A0-A4
const uint16_t STRED_SENZORU = (NUM_SENSORS - 1) * 500;

const uint8_t ENA = 5;   //motory
const uint8_t ENB = 6;
const uint8_t L_MOTOR1 = 7;
const uint8_t L_MOTOR2 = 8;
const uint8_t R_MOTOR1 = 9;
const uint8_t R_MOTOR2 = 10;

const uint8_t ONOFF = 2;
#define TRIMR A6

const uint8_t HC_TRIG = 1;
const uint8_t HC_ECHO = 2;

const uint8_t SERVO = 3;

/*************************************************************************
* nastavení hodnot pro rychlost
*************************************************************************/
const uint8_t MAX_SPEED_L = 255;
const uint8_t MAX_SPEED_R = 255;
uint8_t MED_SPEED_L = 90;
uint8_t MED_SPEED_R = 90;
const uint8_t MIN_SPEED = 0;

/*************************************************************************
* ladění PID
*************************************************************************/
const float Kp = 0.100; //hodnota Proportional pro PID (např. 0.07)
                        // 2k*0.1275=255, 2k*0.07=140
const float Ki = 0; //hodnota Integral pro PID (např. 0.0008)
const float Kd = 0.0; //hodnota Derivative pro PID (např. 10-20 * Kp)

/*************************************************************************
* proměnné pro PID
*************************************************************************/
float P = 0;
float I = 0;
float D = 0;

int16_t lastError = 0;

/*************************************************************************
* Výchozí hodnoty
*************************************************************************/
uint16_t sensor[NUM_SENSORS] = {0, 0, 0, 0, 0};


/*************************************************************************
* Globální proměnné/konstanty
*************************************************************************/
boolean onoff = false;
boolean cara_detekovana = false;
int32_t pozice = STRED_SENZORU;
int16_t error = 0;
uint16_t minS[NUM_SENSORS];
uint16_t maxS[NUM_SENSORS];
uint8_t kalibracni_rychlost = 80;
const uint16_t PREKAZKA = 500;

/*************************************************************************
* Servo motorek pro čidlo vzdálenosti
*************************************************************************/
#include <Servo.h> 
Servo servo_oci;
const uint8_t KUK_VPRAVO = 140;
const uint8_t KUK_ROVNE = 90;
const uint8_t KUK_VLEVO = 40;

#endif

/*************************************************************************
* PINOUT Arduino nano
**************************************************************************
* D0               (pin TX)
* D1    HC trigger ***(pin RX)
* D2    HC echo ***
* D3 ~  servo pro natáčení "očiček" (oranžový kablík) (red+, brown GND) ***
* D4    tlačítko on/off ***
* D5 ~  L298 ENA ***
* D6 ~  L298 ENB ***
* D7    můstek 1 ***
* D8    můstek 2 ***
* D9 ~  můstek 3 ***
* D10 ~ můstek 4 ***
* D11 ~ 
* D12 
* D13
* A0    senzor LL ***
* A1    senzor L ***
* A2    senzor C ***
* A3    senzor R ***
* A4    senzor RR ***
* A5    
* A6    trimr  *** //nefunguje jako digitální pin
* A7           //nefunguje jako digitální pin
*******************************************************************/
