#ifndef DEBUG_H
#define DEBUG_H

#include "Setup.h"

#ifdef DEBUG
   #define DEBUG_PRINT(x)     Serial.print(x)
   #define DEBUG_PRINTLN(x)   Serial.println(x)
#else
   #define DEBUG_PRINT(x)
   #define DEBUG_PRINTLN(x)
#endif

#ifdef DEBUG_SENZORY
   #define DEBUG_PRINTS(x)     Serial.print(x)
   #define DEBUG_PRINTSLN(x)   Serial.println(x)
#else
   #define DEBUG_PRINTS(x)
   #define DEBUG_PRINTSLN(x)
#endif

#ifdef DEBUG_TRIMRY
   #define DEBUG_PRINTT(x)     Serial.print(x)
   #define DEBUG_PRINTTLN(x)   Serial.println(x)
#else
   #define DEBUG_PRINTT(x)
   #define DEBUG_PRINTTLN(x)
#endif


#endif
