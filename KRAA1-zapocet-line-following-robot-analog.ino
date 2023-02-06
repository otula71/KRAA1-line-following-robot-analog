#include "Debug.h"
/*************************************************************************
* Název funkce: setup
**************************************************************************
* Nastavení vstupů/výstupů
* případná další nastavení
* 
* Parametry:
*  none 
* 
* Vrací:
*  none
*************************************************************************/
void setup() {
  pinMode(HC_TRIG, OUTPUT);
  pinMode(HC_ECHO, INPUT);
  pinMode(TRIMR, INPUT);
  pinMode(ONOFF, INPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(L_MOTOR1, OUTPUT);
  pinMode(L_MOTOR2, OUTPUT);
  pinMode(R_MOTOR1, OUTPUT);
  pinMode(R_MOTOR2, OUTPUT);
  for(uint8_t i=0;i<NUM_SENSORS;i++){pinMode(SENSOR[i], INPUT);}
  
  servo_oci.attach(SERVO);
  
  #ifdef DEBUG
      Serial.begin(9600);
  #endif
}


/*************************************************************************
* Název funkce: loop
**************************************************************************
* Řídicí program.
* Kontroluje spuštění/vypnutí a volá další funkce
* 
* Parametry:
*  none 
* 
* Vrací:
*  none
*************************************************************************/
void loop() {
  if(digitalRead(ONOFF) == HIGH) {
    onoff =! onoff; //při stisku tlačítka přepneme stav
    if(onoff == true) {
      DEBUG_PRINTLN("Jedeme!");
      delay(1700); //chvilka strpení před startem
    }
    else {
      DEBUG_PRINTLN("Zastavujeme!");
      delay(50);
    }
  }
  
  if (onoff == true) {
    jedeme_s_PID(); //jedeme
  }
  else {
    zastav(500); //stojíme
  }
}

/*************************************************************************
* Název funkce: ovladani_motoru
**************************************************************************
* Tato funkce ovládá chod motorků
* 
* Parametry:
*  int l: od 0 do 255; rychlost levého motoru
*  int r: od 0 do 255; rychlost pravého motoru
*  boolean lf, rf; true je směr dopředu, false dozadu
* 
* Vrací:
*  none
*************************************************************************/
void ovladani_motoru(uint8_t l, uint8_t r, boolean lf, boolean rf) { 
    analogWrite(ENA, l);
    analogWrite(ENB, r);
  if(lf && rf)  {
    digitalWrite(L_MOTOR1, HIGH);
    digitalWrite(L_MOTOR2, LOW);
    digitalWrite(R_MOTOR1, HIGH);
    digitalWrite(R_MOTOR2, LOW);
  }

  else if(!lf && rf) {
    digitalWrite(L_MOTOR1, LOW);
    digitalWrite(L_MOTOR2, HIGH);
    digitalWrite(R_MOTOR1, HIGH);
    digitalWrite(R_MOTOR2, LOW);
  }

  else if(lf && !rf) { 
    digitalWrite(L_MOTOR1, HIGH);
    digitalWrite(L_MOTOR2, LOW);
    digitalWrite(R_MOTOR1, LOW);
    digitalWrite(R_MOTOR2, HIGH);
  }

  else if(!lf && !rf) {
    digitalWrite(L_MOTOR1, LOW);
    digitalWrite(L_MOTOR2, HIGH);
    digitalWrite(R_MOTOR1, LOW);
    digitalWrite(R_MOTOR2, HIGH);
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
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
    digitalWrite(L_MOTOR1, LOW);
    digitalWrite(L_MOTOR2, LOW);
    digitalWrite(R_MOTOR1, LOW);
    digitalWrite(R_MOTOR2, LOW);
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
int detekuj_caru(int16_t z) {
  cara_detekovana = false;
  uint32_t soucet_hodnot = 0;
  uint16_t pocet_sens = 0;
  for(uint8_t i=0; i<NUM_SENSORS; i++){
  sensor[i] = analogRead(SENSOR[i]);
  sensor[i]=constrain(map(sensor[i], minS[i], maxS[i],1000,0),0,1000);
  cara_detekovana = (sensor[i]>200) ? true : false; //čáru vyhodnocuji od hodnoty 200, potřeba poladit
  if(sensor[i]>50) { //připočítávám senzory, které zachytí jakous-takous hodnotu, také potřeba poladit
    soucet_hodnot += (uint32_t)sensor[i]*1000*i;
    pocet_sens += sensor[i]; //nedává to přesný výsledek, ale mohlo by to obstát
    }
  }
  int16_t x = (sensor[0]>200 && sensor[4]>200) ? -100 : (cara_detekovana) ? soucet_hodnot/pocet_sens : (pozice < STRED_SENZORU) ? 0 : STRED_SENZORU*2;
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
void jedeme_s_PID() {
  int16_t pozice = detekuj_caru(pozice); //načti aktuální pozici
  if (pozice == -100){zastav(1000);DEBUG_PRINTLN("Cílová čára");}//STOP
  else {
  int16_t error = STRED_SENZORU - pozice; //orientovaná odchylka od středu dráhy
   float koef = nacti_trimr()/10230.0000;
  P = error;
  I = I + error;
  D = error - lastError;
  lastError = error;
  int16_t korekce_rychlosti = P*(Kp+koef) + I*Ki + D*Kd; //výpočet PID s měřením 
//  int16_t korekce_rychlosti = P*Kp + I*Ki + D*Kd; //výpočet PID korekce rychlosti 
  uint8_t rychlost_L = constrain(MED_SPEED_L + korekce_rychlosti,0,MAX_SPEED_L);
  uint8_t rychlost_R = constrain(MED_SPEED_R - korekce_rychlosti,0,MAX_SPEED_R);

  ovladani_motoru(rychlost_L, rychlost_R, true, true);

  DEBUG_PRINT("Pozice: ");DEBUG_PRINTLN(error);
  DEBUG_PRINT("Trimr/100: ");DEBUG_PRINTLN(koef*100);
  }
}

/*************************************************************************
* Název funkce: zatoc
**************************************************************************
* Toto je funkce do foroty pro odbočování, zadá se směr, rozdíl rychlosti
* a doba odbočování
* 
* Parametry:
*  char L|R
*  int rychlost (změna)
*  int cas (doba v ms, po kterou bude zatáčet)
* 
* Vrací:
*  none
*************************************************************************/
void zatoc(char smer, uint8_t spd, uint16_t cas) { //Turning setup
  switch(smer) {
    case 'L':
      ovladani_motoru(constrain(MED_SPEED_L-spd,MIN_SPEED, MAX_SPEED_L), constrain(MED_SPEED_R+spd,MIN_SPEED, MAX_SPEED_R), true, true);
      delay(cas);
      break;
    case 'R':
      ovladani_motoru(constrain(MED_SPEED_L+spd,MIN_SPEED, MAX_SPEED_L), constrain(MED_SPEED_R-spd,MIN_SPEED, MAX_SPEED_R), true, true);
      delay(cas);
      break;
  }
}


/*************************************************************************
* Název funkce: prekazka
**************************************************************************
* Funkce pro detekci překážky na trase
* pomocí ultrazvukového senzoru HC-SR04
* 
* Parametry:
*  none
* 
* Vrací:
*  uint: vzdálenost předmětu (reálně čas odezvy)
*************************************************************************/
uint16_t prekazka() { 
  digitalWrite(HC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(HC_TRIG, HIGH);
  delayMicroseconds(5);
  digitalWrite(HC_TRIG, LOW);
  uint16_t odezva = pulseIn(HC_ECHO, HIGH);
  DEBUG_PRINT("Odezva: ");DEBUG_PRINTLN(odezva);
//  boolean x = (odezva > PREKAZKA) ? false : true;
  return odezva;
}

/*************************************************************************
* Název funkce: nacti_trimr
**************************************************************************
* Funkce pro ladění pohybu
* čte hodnotu z připojeného trimru
* načítá se pouze při ladění
* 
* Parametry:
*  none
* 
* Vrací:
*  uint16_t: 0-1023
*************************************************************************/
uint16_t nacti_trimr() { 
  return analogRead(TRIMR);
}

/*************************************************************************
* Název funkce: rozhledni_se
**************************************************************************
* Funkce pro zjištění vzdálenosti možné překážky  
* 
* Parametry:
*  char L|R|S - směr pohledu vlevo|vpravo|rovně
* 
* Vrací:
*  uint16_t: vzdálenost objektu v zadaném směru
*************************************************************************/
uint16_t rozhledni_se(char S) {
  switch (S) {
    case 'L':
      servo_oci.write(KUK_VLEVO);
      return prekazka();
      servo_oci.write(KUK_ROVNE);
      break;
    case 'R':
      servo_oci.write(KUK_VPRAVO);
      return prekazka();
      servo_oci.write(KUK_ROVNE);
      break;
    default:
      return prekazka();
      break;
    }
  }
