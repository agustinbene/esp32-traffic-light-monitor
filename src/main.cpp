#include <Arduino.h>
#include "rtc_module.h"
#include "network.h"
#include "traffic_lights.h"

void setup()
{
  delay(1000);
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("=== ESP32CAM + RTC DS1307 + W5100 + NTP + SEMÁFOROS ===");

  // --- Inicializar módulo de red primero (necesario para NTP) ---
  initNetwork();

  // --- Inicializar RTC con sincronización NTP ---
  initRTCWithNTPSync();

  // --- Inicializar sistema de semáforos ---
  initTrafficLights();

  Serial.println("✅ Sistema completo inicializado. Iniciando operación...");
}

void loop()
{
  // Actualizar estado de semáforos (debe ejecutarse en cada loop para detección rápida)
  updateTrafficLights();

  // Verificar y enviar datos de red cada intervalo definido
  if (millis() - previousMillis >= interval)
  {
    previousMillis = millis();

    // Mostrar información actual del RTC
    printRTCInfo();

    // Mostrar estado de semáforos
    printTrafficLightStatus();

    // Enviar datos de semáforos si hay sesiones pendientes
    if (hasPendingTrafficLightData())
    {
      sendTrafficLightData();
    }
    else
    {
      // Solo enviar heartbeat si no hay datos de semáforos
      sendNetworkDataWithRTC();
    }

    // Mostrar sesiones pendientes (para debug)
    if (getPendingSessionsCount() > 0)
    {
      printPendingSessions();
    }
  }

  // Mantener conexión de red (verificar cada loop)
  checkNetworkConnection();

  delay(10); // Pausa pequeña para no sobrecargar el loop pero mantener detección rápida
}
