#ifndef NETWORK_H
#define NETWORK_H

#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>
#include "rtc_module.h"     // Para incluir datos del RTC en los envíos
#include "traffic_lights.h" // Para incluir datos de semáforos en los envíos

// --- Configuración de Red ---
extern byte mac[];
extern IPAddress dnsServer;

// --- Servidor destino ---
extern const char *host;
extern const int port;
extern const char *endpoint;

// --- Control de tiempo ---
extern const unsigned long interval;
extern unsigned long previousMillis;
extern int requestCounter;

// --- Cliente HTTP ---
extern EthernetClient client;

// --- Pines SPI para W5100 ---
#define MISO_PIN 12
#define MOSI_PIN 13
#define SCK_PIN 14
#define CS_PIN 15

// --- Funciones del módulo de red ---
void initNetwork();
void sendNetworkData();
void sendNetworkDataWithRTC(); // Nueva función que incluye datos del RTC
void sendTrafficLightData();   // Nueva función para enviar datos de semáforos
bool postJSON(const char *host, int port, const char *path, const String &payload);
void checkNetworkConnection();

#endif