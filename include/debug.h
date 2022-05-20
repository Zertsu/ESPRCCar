#pragma once

#define DEBUG

#ifdef DEBUG
   #define DPRINT(...)    Serial.print(__VA_ARGS__)
   #define DPRINTLN(...)  Serial.println(__VA_ARGS__)
   #define DPRINTF(...)   Serial.printf(__VA_ARGS__)
#else
   #define DPRINT(...)
   #define DPRINTLN(...)
   #define DPRINTF(...)
#endif
