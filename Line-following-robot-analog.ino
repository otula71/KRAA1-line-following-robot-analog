#include "Debug.h"
/*************************************************************************
* Název funkce: setup
**************************************************************************
* Nastavení potřebného a kalibrace analogových senzorů
* 
* Parametry:
*  none 
* 
* Vrací:
*  none
*************************************************************************/
void setup() {
#ifndef DISABLETRIMR
  pinMode(TRIMR1, INPUT);
  pinMode(TRIMR2, INPUT);
#endif
  pinMode(ONOFF, INPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(STBY, OUTPUT);
  pinMode(L_MOTOR1, OUTPUT);
  pinMode(L_MOTOR2, OUTPUT);
  pinMode(R_MOTOR1, OUTPUT);
  pinMode(R_MOTOR2, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  for(uint8_t i=0;i<NUM_SENSORS;i++){pinMode(SENSOR[i], INPUT);}
  stbyoff(false);
  #ifdef DEBUG
      Serial.begin(9600);
  #endif

  while (!kontrola_kalibrace()){kalibrace();}
  
}


/*************************************************************************
* Název funkce: loop
**************************************************************************
* Řídicí program.
* Kontroluje tlačítko a volá funkce pro překážku a jízdu
* 
* Parametry:
*  none 
* 
* Vrací:
*  none
*************************************************************************/
void loop() {
  if(digitalRead(ONOFF)) {
    onoff =! onoff; //při stisku tlačítka přepneme stav
    if(onoff) {
      DEBUG_PRINTLN("Jedeme!");
      stbyoff(true);
      pulse_led(1700, LED_BLUE); //chvilka strpení před startem
    }
    else {
      DEBUG_PRINTLN("Zastavujeme!");
      stbyoff(false);
      digitalWrite(LED_BLUE,HIGH);
      digitalWrite(LED_RED,LOW);
      delay(50);
    }
  }
  
  if (onoff) {
    #ifndef KACENA
      jedeme_s_PID(); //jedeme
    #else
      jedeme_stupid();
    #endif
  }
  else {
    zastav(500); //stojíme
  }
}

/*************************************************************************
* Název funkce: ovladani_motoru
**************************************************************************
* Tato (přetížená) funkce ovládá chod motorků
* 
* Parametry:
*  int l: od 0 do 255; rychlost levého motoru
*  int r: od 0 do 255; rychlost pravého motoru
*  char f: možno vynechat, potom jede dopředu ***
*          f(orward): dopředu, 
*          r(everse): dozadu, 
*          b(rake): brzdy, 
*          s(top): zastavit
* 
* Vrací:
*  none
*************************************************************************/
void ovladani_motoru(uint8_t l, uint8_t r) { 
    analogWrite(ENA, l);
    analogWrite(ENB, r);
    digitalWrite(L_MOTOR1, HIGH);
    digitalWrite(L_MOTOR2, LOW);
    digitalWrite(R_MOTOR1, HIGH);
    digitalWrite(R_MOTOR2, LOW);
}

void ovladani_motoru(uint8_t l, uint8_t r, char f) { 
  switch (f)  {
    case 'f':
    analogWrite(ENA, l);
    analogWrite(ENB, r);
    digitalWrite(L_MOTOR1, HIGH);
    digitalWrite(L_MOTOR2, LOW);
    digitalWrite(R_MOTOR1, HIGH);
    digitalWrite(R_MOTOR2, LOW);
    break;

    case 'r':
    analogWrite(ENA, l);
    analogWrite(ENB, r);
    digitalWrite(L_MOTOR1, LOW);
    digitalWrite(L_MOTOR2, HIGH);
    digitalWrite(R_MOTOR1, LOW);
    digitalWrite(R_MOTOR2, HIGH);
    break;

    case 'b':
    digitalWrite(ENA, LOW);
    digitalWrite(ENB, LOW);
    digitalWrite(L_MOTOR1, HIGH);
    digitalWrite(L_MOTOR2, HIGH);
    digitalWrite(R_MOTOR1, HIGH);
    digitalWrite(R_MOTOR2, HIGH);
    break;

    case 's':
    digitalWrite(ENA, HIGH);
    digitalWrite(ENB, HIGH);
    digitalWrite(L_MOTOR1, LOW);
    digitalWrite(L_MOTOR2, LOW);
    digitalWrite(R_MOTOR1, LOW);
    digitalWrite(R_MOTOR2, LOW);
    break;
  }
}
/*************************************************************************
* Název funkce: zastav
**************************************************************************
* zastavení motorů 
* 
* Parametry:
*  int t: čas v milisekundách pro pauzu
* 
* Vrací:
*  none
*************************************************************************/
void zastav(uint16_t t) {
    ovladani_motoru(0,0,'s');
    delay(t);
  }


/*************************************************************************
* Název funkce: detekuj_caru
**************************************************************************
* Tato funkce vrací přepočítanou hodnotu polohy čáry vůči senzorům
* 
* Parametry:
*  int z: hodnota, kterou funkce vrátí, pokud všechny senzory vidí bílou.
*         Používám ji k tomu, aby v takovém případě vrátila stejnou hodnotu
*         jako při předešlém čtení, ať už byla mezi senzory nebo se senzory
*         dostaly všechny mimo trasu
* 
* Vrací:
*  int polohy 0 až 4000, pokud vidí čáru
*  int -100, pokud je na krajních senzorech černá
*  int z, pokud je na všech senzorech bílá
*************************************************************************/
int32_t detekuj_caru(int32_t z) {
  cara_detekovana = false;
  uint32_t soucet_hodnot = 0;
  uint16_t pocet_sens = 0;
  noInterrupts();
  for(uint8_t i=0; i<NUM_SENSORS; i++){
  sensor[i] = analogRead(SENSOR[i]);
  sensor[i]=constrain(map(sensor[i], minS[i], maxS[i],1000,0),0,1000);
  DEBUG_PRINTS(sensor[i]);DEBUG_PRINTS("\t");
  if(sensor[i]>300)cara_detekovana = true; //čáru vyhodnocuji od hodnoty 500, potřeba poladit
    if(sensor[i]>60) { //připočítávám senzory, které zachytí jakous-takous hodnotu, také potřeba poladit
    soucet_hodnot += (uint32_t)sensor[i]*1000*i;
    pocet_sens += sensor[i]; //nedává to přesný výsledek, ale mohlo by to obstát
    }
  }
  interrupts();
 DEBUG_PRINTS("čára:");DEBUG_PRINTS(cara_detekovana);DEBUG_PRINTS("\t");
 DEBUG_PRINTSLN("");
  int32_t x = (sensor[0]>500 && sensor[4]>500) ? -100 : (cara_detekovana) ? soucet_hodnot/pocet_sens : z;
 DEBUG_PRINTS("Pozice:");DEBUG_PRINTS(x);DEBUG_PRINTS("\t");
  return x;
}

/*************************************************************************
* Název funkce: jedeme_s_PID
**************************************************************************
* Funkce pro ovládání robota pomocí algoritmu PID.
* Korekce se aplikuje pomocí úpravy rychlosti kol v intervalu [0, max rychlost]
* max rychlost <= 255. 
* 
* Parametry:
*  none
* 
* Vrací:
*  none
*************************************************************************/
#ifndef KACENA
void jedeme_s_PID() {
  pozice = detekuj_caru(pozice); //načti aktuální pozici
//  DEBUG_PRINT("pos2: ");DEBUG_PRINT(pozice);DEBUG_PRINT("\t");
  if (pozice == -100){/*zastav(1000);*/DEBUG_PRINTLN("Cílová čára");}//STOP
  else {
  int16_t error = STRED_SENZORU - pozice; //orientovaná odchylka od středu dráhy
  //DEBUG_PRINT("Odchylka: "); DEBUG_PRINTLN(error);
  #ifndef DISABLETRIMR
   Kd = (nacti_trimr(1))/5000.000;
   Kp = (nacti_trimr(2))/3500.000;
  #endif
  P = error;
  casAktualni = millis();
  casVzorkovaci = (float)(casAktualni - casMinuly)/75;
  I += error*casVzorkovaci;
  D = (error - lastError)/casVzorkovaci;
  int16_t korekce_rychlosti = P*Kp + I*Ki + D*Kd; //výpočet PID s měřením 
  DEBUG_PRINTT("Rychlost: ");DEBUG_PRINTT(MAX_SPEED_L); DEBUG_PRINTT("\t"); DEBUG_PRINTT("Kp: ");DEBUG_PRINTT(Kp); DEBUG_PRINTT("\t"); DEBUG_PRINTT("1000Ki: ");DEBUG_PRINTT(Ki*1000);DEBUG_PRINTT("\t");
  DEBUG_PRINTT("1000Kd: ");DEBUG_PRINTT(Kd*1000);DEBUG_PRINTT("\t");DEBUG_PRINTT("Vzork. čas (s): "); DEBUG_PRINTTLN(casVzorkovaci); 
  lastError = error;
  casMinuly = casAktualni;

/* MAXIMÁLNÍ RYCHLOST **********/
  MED_SPEED_L = MAX_SPEED_L - abs(korekce_rychlosti);
  MED_SPEED_R = MAX_SPEED_R - abs(korekce_rychlosti);
/*******************************/

  uint8_t rychlost_L = constrain(MED_SPEED_L + korekce_rychlosti,MIN_SPEED,MAX_SPEED_L);
  uint8_t rychlost_R = constrain(MED_SPEED_R - korekce_rychlosti,MIN_SPEED,MAX_SPEED_R);
   
  ovladani_motoru(rychlost_L, rychlost_R);
  delayMicroseconds(pauza); //hodnota původně pro LGT8F328, z nějakého důvodu se to bez pauzy chová, jako by to naopak nestíhalo... (i Nano)
  //DEBUG_PRINT("Pozice: ");DEBUG_PRINTLN(error);
  //DEBUG_PRINTT("Trimr/100: ");DEBUG_PRINTTLN(koef*100);
  }
}




#endif


/*************************************************************************
* Název funkce: jedeme_stupid
**************************************************************************
* Funkce pro ovládání robota pomocí primitivního algoritmu kačena.
* Záchranná funkce, kdyby to nešlo doladit :-)))))
* 
* Parametry:
*  none
* 
* Vrací:
*  none
*************************************************************************/
#ifdef KACENA
void jedeme_stupid() {
  pozice = detekuj_caru(pozice); //načti aktuální pozici
  #ifndef DISABLETRIMR
   MED_SPEED_L = constrain(nacti_trimr(2)/4,0,255);
   MED_SPEED_R = MED_SPEED_L;
  #endif
  if (pozice == -100){zastav(1000);DEBUG_PRINTLN("Cílová čára");}//STOP
  else if (pozice<2000){ovladani_motoru(MED_SPEED_L, MIN_SPEED);}
  else if (pozice>2000){ovladani_motoru(MIN_SPEED, MED_SPEED_R);}
  else {ovladani_motoru(MED_SPEED_L, MED_SPEED_R);}
}
#endif


/*************************************************************************
* Název funkce: nacti_trimr(1|2)
**************************************************************************
* Funkce pro ladění pohybu
* čte hodnotu z připojeného trimru
* Po nalezení hodnot je zadat do setupu a funkci v setupu vypnout
* 
* Parametry:
*  vyber trimr 1 nebo 2
* 
* Vrací:
*  uint16_t: 0-1023
*************************************************************************/
#ifndef DISABLETRIMR
uint16_t nacti_trimr(uint8_t x) { 
  switch (x) {
    case 1:
      return analogRead(TRIMR1);
      break;
    default:
      return analogRead(TRIMR2);
      break;
  }
}
#endif

/*************************************************************************
* Název funkce: kalibrace
**************************************************************************
* Funkce pro kalibraci senzorů - změří rozsah, který dávají jednotlivé  
* senzory a "normalizuje" ho na rozsah 0-1000
* 
* Parametry:
*  none
* 
* Vrací:
*  none
*  přiřazuje hodnoty polím minS[] a maxS[]
*************************************************************************/
void kalibrace() {
  #ifdef DEBUG
    delay(500);
  #endif
  DEBUG_PRINTLN(" ");
  DEBUG_PRINTLN("Kalibrace - sken senzorů...");
  stbyoff(true);
  uint32_t time0 = millis();

  for(uint8_t i=0; i<NUM_SENSORS; i++){
    sensor[i]=analogRead(SENSOR[i]);
    minS[i]=sensor[i];
    maxS[i]=sensor[i];
    }
  while((millis()-time0)<=2000){
    for(byte i=0; i<NUM_SENSORS; i++){
      sensor[i]=analogRead(SENSOR[i]);
      if(sensor[i]<minS[i]) minS[i]=sensor[i];
      if(sensor[i]>maxS[i]) maxS[i]=sensor[i];
    }
    ovladani_motoru(0, kalibracni_rychlost);
    }
    #ifdef DEBUG
      DEBUG_PRINTLN("Kalibrace dokončena: ");
      for (uint8_t i=0; i<NUM_SENSORS;i++){DEBUG_PRINT("senzor "); DEBUG_PRINT(i); DEBUG_PRINT(" min: "); DEBUG_PRINT(minS[i]); DEBUG_PRINT(" -- max: "); DEBUG_PRINTLN(maxS[i]);}
    #endif
    zastav(100);
}



/*************************************************************************
* Název funkce: stbyoff(BOOL)
**************************************************************************
* vypíná a zapíná standby motorů
* 
* Parametry:
*  TRUE|FALSE - při TRUE motory běží
* 
* Vrací:
*  none
*************************************************************************/

void stbyoff(boolean x){
  if (x) {digitalWrite(STBY,HIGH);}
  else {digitalWrite(STBY,LOW);}
}
  
/*************************************************************************
* Název funkce: kontrola_kalibrace
**************************************************************************
* kontroluje, zda kalibrace dala rozumné výsledky
* Pokud nedala, volá ji znovu
* 
* Parametry:
*  none
* 
* Vrací:
*  boolean TRUE|FALSE
*************************************************************************/

boolean kontrola_kalibrace(){
  boolean x = true;
  for (uint8_t i=0; i<NUM_SENSORS;i++){
    if (maxS[i] - minS[i] < 150) {x=false;}
  }
  if (x) {
    DEBUG_PRINTLN("Kalibrace OK");
    digitalWrite(LED_BLUE, HIGH);
  }
  else {
    DEBUG_PRINTLN("Chybná kalibrace");
    pulse_led(3000,LED_RED);
    digitalWrite(LED_RED,HIGH); delay(1000);digitalWrite(LED_RED,LOW);
    }
    return x;
}

/*************************************************************************
* Název funkce: pulse_led(čas, led)
**************************************************************************
* bliká pulzně s vybranou LED po zadanou dobu (ms)
* 
* Parametry:
*  int čas (ms)
*  led (LED_BLUE|LED_RED)
* 
* Vrací:
*  none
*************************************************************************/
void pulse_led(uint16_t t, uint8_t led){
    uint32_t time0 = millis();
    while((millis()-time0)<=t){digitalWrite(led, HIGH);delay(20);digitalWrite(led, LOW);delay(300);}
}

