#ifndef SETUP_H
#define SETUP_H

//#define DEBUG       // zakomentuj před finální kompilací
//#define DEBUG_SENZORY
//#define DEBUG_TRIMRY
//#define KACENA      // kačení styl jízdy
//#define DISABLETRIMR //nebude načítat hodnoty z trimrů

/*************************************************************************
* připojení k Arduinu
*************************************************************************/
const uint8_t ONOFF = 4;
const uint8_t ENA = 5;   //motory
const uint8_t ENB = 6;
const uint8_t L_MOTOR1 = 7;
const uint8_t L_MOTOR2 = 8;
const uint8_t R_MOTOR1 = 9;
const uint8_t R_MOTOR2 = 10;
const uint8_t LED_RED = 11;
const uint8_t LED_BLUE = 12;
const uint8_t STBY = 13;
const uint8_t NUM_SENSORS = 5;
const uint8_t SENSOR[NUM_SENSORS] = {14, 15, 16, 17, 18};   // IR snímače A0-A4
const uint16_t STRED_SENZORU = (NUM_SENSORS - 1) * 500;
#ifndef DISABLETRIMR
#define TRIMR1 A5
#define TRIMR2 A6
#endif

/*************************************************************************
* nastavení hodnot pro rychlost
*************************************************************************/
uint8_t MAX_SPEED_L = 240;
uint8_t MAX_SPEED_R = 240;
uint8_t MED_SPEED_L = 150;
uint8_t MED_SPEED_R = 150;
const uint8_t MIN_SPEED = 0;
uint16_t pauza = 50;

/*************************************************************************
* ladění PID
*************************************************************************/
float Kp = 0.27; //hodnota Proportional pro PID (např. 0.07)
                        // 2k*0.1275=255, 2k*0.07=140
float Ki = 0.001;//029; //hodnota Integral pro PID (např. 0.0008)
float Kd = 0.170; //hodnota Derivative pro PID (např. 10-20 * Kp)

/*************************************************************************
* proměnné pro PID
*************************************************************************/
float P = 0;
float I = 0;
float D = 0;
int16_t lastError = 0;
uint64_t casAktualni, casMinuly;
float casVzorkovaci;


/*************************************************************************
* Globální proměnné/konstanty
*************************************************************************/
boolean onoff = false;
boolean cara_detekovana = false;
int32_t pozice = STRED_SENZORU;
int16_t error = 0;
uint16_t sensor[NUM_SENSORS] = {0, 0, 0, 0, 0};
uint16_t minS[NUM_SENSORS];
uint16_t maxS[NUM_SENSORS];
uint8_t kalibracni_rychlost = 120;


#endif

/*************************************************************************
* PINOUT Arduino nano
**************************************************************************
* D0                    (pin RX)
* D1                    (pin TX)
* D2     
* D3 ~  
* D4    tlačítko on/off
* D5 ~  L298 ENA
* D6 ~  L298 ENB
* D7    můstek 1
* D8    můstek 2
* D9 ~  můstek 3
* D10 ~ můstek 4
* D11 ~ červená LED
* D12   modrá LED
* D13   můstek Standby
* A0    senzor LL
* A1    senzor L
* A2    senzor C
* A3    senzor R
* A4    senzor RR
* A5    trimr 1
* A6    trimr 2 //nefunguje jako digitální pin
* A7           //nefunguje jako digitální pin
*******************************************************************/
