#ifndef TRAFFIC_LIGHTS_H
#define TRAFFIC_LIGHTS_H

#include <Arduino.h>
#include "rtc_module.h"

// --- Configuración de pines para los 4 semáforos ---
#define TRAFFIC_LIGHT_1_PIN 4
#define TRAFFIC_LIGHT_2_PIN 2
// #define TRAFFIC_LIGHT_3_PIN 5
// #define TRAFFIC_LIGHT_4_PIN 18

// --- Número de semáforos ---
#define NUM_TRAFFIC_LIGHTS 2

// --- Estructura para almacenar datos de semáforo ---
struct TrafficLightData
{
    int pin;                    // Pin de entrada
    bool currentState;          // Estado actual (true = luz roja encendida)
    bool previousState;         // Estado anterior
    DateTime redOnTime;         // Timestamp cuando se encendió la luz roja
    DateTime redOffTime;        // Timestamp cuando se apagó la luz roja
    bool hasActiveSession;      // Si hay una sesión activa (luz roja encendida)
    bool hasPendingData;        // Si hay datos pendientes para enviar
    unsigned long debounceTime; // Para anti-rebote
    bool isDebouncing;          // Estado de debounce
};

// --- Configuración de debounce ---
#define DEBOUNCE_DELAY 200 // ms - Aumentado para mejor filtrado

// --- Array de datos de semáforos ---
extern TrafficLightData trafficLights[NUM_TRAFFIC_LIGHTS];

// --- Buffer para almacenar sesiones completadas ---
#define MAX_PENDING_SESSIONS 20 // Reducido para evitar JSONs muy grandes

struct CompletedSession
{
    int trafficLightId; // ID del semáforo (0-3)
    DateTime startTime; // Cuando se encendió la luz roja
    DateTime endTime;   // Cuando se apagó la luz roja
};

extern CompletedSession pendingSessions[MAX_PENDING_SESSIONS];
extern int pendingSessionsCount;

// --- Funciones del módulo de semáforos ---
void initTrafficLights();
void updateTrafficLights();
void processTrafficLightChange(int lightIndex, bool newState);
bool addCompletedSession(int trafficLightId, DateTime startTime, DateTime endTime);
void printTrafficLightStatus();
void printPendingSessions();
bool hasPendingTrafficLightData();
String getTrafficLightDataJSON();
void clearPendingSessions();
int getPendingSessionsCount();

#endif
