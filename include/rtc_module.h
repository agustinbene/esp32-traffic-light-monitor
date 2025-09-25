#ifndef RTC_MODULE_H
#define RTC_MODULE_H

#include <Wire.h>
#include "RTClib.h"

// --- Configuración del RTC DS1307 ---
extern RTC_DS1307 rtc;
extern char daysOfTheWeek[7][12];

// Pines I2C del DS1307
#define SDA_PIN 16 // Cambiado para evitar conflicto con W5100
#define SCL_PIN 0  // Pin RST del W5100 en el código original

// --- Funciones del módulo RTC ---
void initRTC();
void initRTCWithNTPSync(); // Nueva función que incluye sincronización NTP
void printRTCInfo();
DateTime getCurrentTime();
bool isRTCRunning();
void setRTCTime(DateTime dateTime);
void setRTCTimeFromCompilation();
String getFormattedDate();
String getFormattedTime();
String getFormattedDateTime();
uint32_t getUnixTimestamp();

#endif
