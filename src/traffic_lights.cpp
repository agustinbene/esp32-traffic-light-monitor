#include "traffic_lights.h"

// --- Inicialización del array de semáforos ---
TrafficLightData trafficLights[NUM_TRAFFIC_LIGHTS] = {
    {TRAFFIC_LIGHT_1_PIN, false, false, DateTime(), DateTime(), false, false, 0, false},
    {TRAFFIC_LIGHT_2_PIN, false, false, DateTime(), DateTime(), false, false, 0, false}};

// --- Buffer de sesiones completadas ---
CompletedSession pendingSessions[MAX_PENDING_SESSIONS];
int pendingSessionsCount = 0;

void initTrafficLights()
{
    Serial.println("=== Inicializando sistema de semáforos ===");

    // Configurar pines de entrada con pull-up interno
    for (int i = 0; i < NUM_TRAFFIC_LIGHTS; i++)
    {
        pinMode(trafficLights[i].pin, INPUT_PULLUP);

        // Leer estado inicial
        trafficLights[i].currentState = !digitalRead(trafficLights[i].pin); // Invertido por pull-up
        trafficLights[i].previousState = trafficLights[i].currentState;

        Serial.print("Semáforo ");
        Serial.print(i + 1);
        Serial.print(" (Pin ");
        Serial.print(trafficLights[i].pin);
        Serial.print("): ");
        Serial.println(trafficLights[i].currentState ? "🔴 ROJO" : "🟢 NO ROJO");
    }

    // Limpiar buffer de sesiones
    pendingSessionsCount = 0;

    Serial.println("✅ Sistema de semáforos inicializado.");
}

void updateTrafficLights()
{
    unsigned long currentTime = millis();

    for (int i = 0; i < NUM_TRAFFIC_LIGHTS; i++)
    {
        // Leer estado actual (invertido por pull-up)
        bool rawState = !digitalRead(trafficLights[i].pin);

        // Procesar debounce
        if (rawState != trafficLights[i].currentState)
        {
            if (!trafficLights[i].isDebouncing)
            {
                trafficLights[i].debounceTime = currentTime;
                trafficLights[i].isDebouncing = true;
            }
            else if (currentTime - trafficLights[i].debounceTime >= DEBOUNCE_DELAY)
            {
                // El cambio es estable, procesarlo
                trafficLights[i].previousState = trafficLights[i].currentState;
                trafficLights[i].currentState = rawState;
                trafficLights[i].isDebouncing = false;

                processTrafficLightChange(i, rawState);
            }
        }
        else
        {
            // No hay cambio, resetear debounce
            trafficLights[i].isDebouncing = false;
        }
    }
}

void processTrafficLightChange(int lightIndex, bool newState)
{
    Serial.print("\n🚦 Cambio detectado en semáforo ");
    Serial.print(lightIndex + 1);
    Serial.print(": ");

    if (newState) // Luz roja se encendió
    {
        Serial.println("🔴 ROJO ENCENDIDO");

        if (isRTCRunning())
        {
            trafficLights[lightIndex].redOnTime = getCurrentTime();
            trafficLights[lightIndex].hasActiveSession = true;

            Serial.print("   Timestamp inicio: ");
            Serial.print(getFormattedDate());
            Serial.print(" ");
            Serial.println(getFormattedTime());
        }
        else
        {
            Serial.println("   ⚠️ RTC no disponible - no se puede registrar timestamp");
        }
    }
    else // Luz roja se apagó
    {
        Serial.println("🟢 ROJO APAGADO");

        if (trafficLights[lightIndex].hasActiveSession && isRTCRunning())
        {
            trafficLights[lightIndex].redOffTime = getCurrentTime();
            trafficLights[lightIndex].hasActiveSession = false;

            Serial.print("   Timestamp fin: ");
            Serial.print(getFormattedDate());
            Serial.print(" ");
            Serial.println(getFormattedTime());

            // Agregar sesión completada al buffer
            if (addCompletedSession(lightIndex,
                                    trafficLights[lightIndex].redOnTime,
                                    trafficLights[lightIndex].redOffTime))
            {
                Serial.println("   ✅ Sesión registrada para envío");
            }
            else
            {
                Serial.println("   ❌ Buffer lleno - sesión perdida");
            }
        }
        else if (!trafficLights[lightIndex].hasActiveSession)
        {
            Serial.println("   ⚠️ No había sesión activa");
        }
        else
        {
            Serial.println("   ⚠️ RTC no disponible");
        }
    }
}

bool addCompletedSession(int trafficLightId, DateTime startTime, DateTime endTime)
{
    if (pendingSessionsCount >= MAX_PENDING_SESSIONS)
    {
        return false; // Buffer lleno
    }

    CompletedSession *session = &pendingSessions[pendingSessionsCount];
    session->trafficLightId = trafficLightId;
    session->startTime = startTime;
    session->endTime = endTime;
    // session->startTimestamp = startTime.unixtime();
    // session->endTimestamp = endTime.unixtime();
    // session->durationSeconds = session->endTime.unixtime() - session->startTime.unixtime();

    pendingSessionsCount++;
    return true;
}

void printTrafficLightStatus()
{
    Serial.println("\n--- Estado actual de semáforos ---");
    for (int i = 0; i < NUM_TRAFFIC_LIGHTS; i++)
    {
        Serial.print("Semáforo ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(trafficLights[i].currentState ? "🔴" : "🟢");

        if (trafficLights[i].hasActiveSession)
        {
            Serial.print(" (Sesión activa desde: ");
            Serial.print(trafficLights[i].redOnTime.hour());
            Serial.print(":");
            if (trafficLights[i].redOnTime.minute() < 10)
                Serial.print("0");
            Serial.print(trafficLights[i].redOnTime.minute());
            Serial.print(")");
        }
        Serial.println();
    }
    Serial.println("----------------------------------");
}

void printPendingSessions()
{
    if (pendingSessionsCount == 0)
    {
        Serial.println("No hay sesiones pendientes.");
        return;
    }

    Serial.println("\n--- Sesiones pendientes para envío ---");
    for (int i = 0; i < pendingSessionsCount; i++)
    {
        CompletedSession *session = &pendingSessions[i];
        Serial.print("Sesión ");
        Serial.print(i + 1);
        Serial.print(" - Semáforo ");
        Serial.print(session->trafficLightId + 1);
        Serial.print(": ");
        Serial.print(session->endTime.unixtime() - session->startTime.unixtime());
        Serial.print("s (");
        Serial.print(session->startTime.hour());
        Serial.print(":");
        if (session->startTime.minute() < 10)
            Serial.print("0");
        Serial.print(session->startTime.minute());
        Serial.print(" - ");
        Serial.print(session->endTime.hour());
        Serial.print(":");
        if (session->endTime.minute() < 10)
            Serial.print("0");
        Serial.print(session->endTime.minute());
        Serial.println(")");
    }
    Serial.println("---------------------------------------");
}

bool hasPendingTrafficLightData()
{
    return pendingSessionsCount > 0;
}

String getTrafficLightDataJSON()
{
    String json = "{";
    json += "\"traffic_light_sessions\":[";

    for (int i = 0; i < pendingSessionsCount; i++)
    {
        if (i > 0)
            json += ",";

        CompletedSession *session = &pendingSessions[i];
        json += "{";
        json += "\"traffic_light_id\":" + String(session->trafficLightId + 1) + ",";
        json += "\"start_timestamp\":" + String(session->startTime.unixtime()) + ",";
        json += "\"end_timestamp\":" + String(session->endTime.unixtime()) + ",";
        json += "\"duration_seconds\":" + String(session->endTime.unixtime() - session->startTime.unixtime()) + ",";
        json += "\"start_date\":\"" + String(session->startTime.year()) + "-" +
                String(session->startTime.month()) + "-" +
                String(session->startTime.day()) + "\",";
        json += "\"start_time\":\"" + String(session->startTime.hour()) + ":" +
                (session->startTime.minute() < 10 ? "0" : "") + String(session->startTime.minute()) + ":" +
                (session->startTime.second() < 10 ? "0" : "") + String(session->startTime.second()) + "\",";
        json += "\"end_date\":\"" + String(session->endTime.year()) + "-" +
                String(session->endTime.month()) + "-" +
                String(session->endTime.day()) + "\",";
        json += "\"end_time\":\"" + String(session->endTime.hour()) + ":" +
                (session->endTime.minute() < 10 ? "0" : "") + String(session->endTime.minute()) + ":" +
                (session->endTime.second() < 10 ? "0" : "") + String(session->endTime.second()) + "\"";
        json += "}";
    }

    json += "],";
    json += "\"total_sessions\":" + String(pendingSessionsCount);
    json += "}";

    return json;
}

void clearPendingSessions()
{
    pendingSessionsCount = 0;
    Serial.println("🗑️ Buffer de sesiones limpiado.");
}

int getPendingSessionsCount()
{
    return pendingSessionsCount;
}
