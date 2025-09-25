#ifndef NTP_SYNC_H
#define NTP_SYNC_H

#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "RTClib.h"

// --- Configuración NTP ---
extern const char *ntpServer;
extern const int ntpPort;
extern const int timeZoneOffset; // Offset en horas respecto a UTC
extern EthernetUDP udp;

// --- Funciones del módulo NTP ---
bool initNTP();
bool syncRTCWithNTP();
unsigned long getNTPTime();
DateTime convertNTPToDateTime(unsigned long ntpTime);
void sendNTPRequest();
bool receiveNTPResponse(unsigned long &ntpTime);
void printNTPSyncStatus(bool success, DateTime syncedTime);

#endif
