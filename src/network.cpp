#include <Arduino.h>
#include "network.h"

// --- Configuración de Red ---
byte mac[] = {0xDA, 0xAD, 0xBE, 0xEF, 0xAE, 0xED};

// DNS alternativo (opcional pero recomendado)
IPAddress dnsServer(8, 8, 8, 8);

// --- Servidor destino ---
const char *host = "bot.abenegas.com.ar";
const int port = 80;
const char *endpoint = "/w5100";

// --- Control de tiempo ---
const unsigned long interval = 5000; // ms
unsigned long previousMillis = 0;
int requestCounter = 0;

// --- Cliente HTTP ---
EthernetClient client;

void initNetwork()
{
    Serial.println("=== Inicializando módulo de red W5100 ===");

    // Inicializar SPI
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);

    // Inicializa SPI CS
    Ethernet.init(CS_PIN);

    // DHCP con DNS forzado
    if (Ethernet.begin(mac) == 0)
    {
        Serial.println("❌ Error: DHCP falló. Reiniciando...");
        delay(5000);
        ESP.restart();
    }
    Ethernet.setDnsServerIP(dnsServer);

    // Verificación del hardware
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
        Serial.println("❌ No se detectó el módulo W5100. Verifica conexiones.");
        while (true)
            delay(1);
    }
    if (Ethernet.linkStatus() == LinkOFF)
    {
        Serial.println("⚠️  Cable de red no conectado.");
    }

    // Mostrar IP y configuración de red
    Serial.println("\n--- Configuración de Red ---");
    Serial.print("IP:      ");
    Serial.println(Ethernet.localIP());
    Serial.print("Gateway: ");
    Serial.println(Ethernet.gatewayIP());
    Serial.print("DNS:     ");
    Serial.println(Ethernet.dnsServerIP());
    Serial.println("------------------------------");

    Serial.println("✅ Módulo de red inicializado correctamente.");
}

void sendNetworkData()
{
    requestCounter++;

    // Armar JSON
    StaticJsonDocument<256> doc;
    doc["device_id"] = "ESP32CAM_W5100";
    doc["request_number"] = requestCounter;
    doc["uptime_seconds"] = millis() / 1000;

    String payload;
    serializeJson(doc, payload);

    Serial.print("\n[#");
    Serial.print(requestCounter);
    Serial.println("] Enviando JSON:");
    Serial.println(payload);

    if (postJSON(host, port, endpoint, payload))
    {
        Serial.println("✅ Petición exitosa.");
    }
    else
    {
        Serial.println("❌ Error al enviar la petición.");
    }
}

void sendNetworkDataWithRTC()
{
    requestCounter++;

    // Armar JSON con datos del RTC
    StaticJsonDocument<512> doc;
    doc["device_id"] = "ESP32CAM_W5100_RTC";
    doc["request_number"] = requestCounter;
    doc["uptime_seconds"] = millis() / 1000;

    // Agregar información del RTC si está funcionando
    if (isRTCRunning())
    {
        doc["rtc_status"] = "running";
        doc["unix_timestamp"] = getUnixTimestamp();
    }
    else
    {
        doc["rtc_status"] = "not_running";
        doc["unix_timestamp"] = 0;
    }

    String payload;
    serializeJson(doc, payload);

    Serial.print("\n[#");
    Serial.print(requestCounter);
    Serial.println("] Enviando JSON con datos RTC:");
    Serial.println(payload);

    if (postJSON(host, port, endpoint, payload))
    {
        Serial.println("✅ Petición exitosa.");
    }
    else
    {
        Serial.println("❌ Error al enviar la petición.");
    }
}

void sendTrafficLightData()
{
    if (!hasPendingTrafficLightData())
    {
        Serial.println("📡 No hay datos de semáforos para enviar.");
        return;
    }

    requestCounter++;

    // Armar JSON con datos de semáforos y RTC
    StaticJsonDocument<2048> doc;
    doc["device_id"] = "ESP32CAM_TRAFFIC_MONITOR";
    doc["request_number"] = requestCounter;
    doc["uptime_seconds"] = millis() / 1000;

    // Agregar información del RTC
    if (isRTCRunning())
    {
        doc["rtc_status"] = "running";
        doc["unix_timestamp"] = getUnixTimestamp();
    }
    else
    {
        doc["rtc_status"] = "not_running";
    }

    // Agregar datos de sesiones de semáforos
    JsonArray sessions = doc.createNestedArray("traffic_light_sessions");

    for (int i = 0; i < getPendingSessionsCount(); i++)
    {
        CompletedSession *session = &pendingSessions[i];
        JsonObject sessionObj = sessions.createNestedObject();

        sessionObj["traffic_light_id"] = session->trafficLightId + 1;
        sessionObj["start_timestamp"] = session->startTime.unixtime();
        sessionObj["end_timestamp"] = session->endTime.unixtime();
    }

    doc["total_sessions"] = getPendingSessionsCount();

    String payload;
    serializeJson(doc, payload);

    Serial.print("\n[#");
    Serial.print(requestCounter);
    Serial.print("] Enviando datos de ");
    Serial.print(getPendingSessionsCount());
    Serial.println(" sesiones de semáforos:");
    Serial.println(payload);

    if (postJSON(host, port, "/traffic_lights", payload))
    {
        Serial.println("✅ Datos de semáforos enviados exitosamente.");
        clearPendingSessions(); // Limpiar buffer después del envío exitoso
    }
    else
    {
        Serial.println("❌ Error al enviar datos de semáforos. Datos conservados para reintento.");
    }
}

bool postJSON(const char *host, int port, const char *path, const String &payload)
{
    Serial.print("Conectando a ");
    Serial.print(host);
    Serial.print(":");
    Serial.print(port);
    Serial.print("... ");

    if (!client.connect(host, port))
    {
        Serial.println("falló.");
        return false;
    }
    Serial.println("OK");

    // Construir la petición HTTP
    client.println("POST " + String(path) + " HTTP/1.1");
    client.println("Host: " + String(host));
    client.println("User-Agent: ESP32CAM-W5100/1.0");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(payload.length());
    client.println(); // línea en blanco
    client.print(payload);

    // Leer respuesta del servidor
    unsigned long timeout = millis() + 2000;
    while (!client.available())
    {
        if (millis() > timeout)
        {
            Serial.println("⏱️ Timeout de respuesta.");
            client.stop();
            return false;
        }
        delay(10);
    }

    String statusLine = client.readStringUntil('\r');
    Serial.println("Respuesta: " + statusLine);
    client.stop();

    return statusLine.startsWith("HTTP/1.1 200");
}

void checkNetworkConnection()
{
    // Verificar estado de la conexión
    if (Ethernet.linkStatus() == LinkOFF)
    {
        Serial.println("⚠️ Cable de red desconectado.");
    }

    // Mantener la conexión DHCP
    switch (Ethernet.maintain())
    {
    case 1:
        // Renovación falló
        Serial.println("Error renovando DHCP lease");
        break;
    case 2:
        // Renovación exitosa
        Serial.println("DHCP lease renovado");
        Serial.print("Nueva IP: ");
        Serial.println(Ethernet.localIP());
        break;
    case 3:
        // Rebind falló
        Serial.println("Error en rebind DHCP");
        break;
    case 4:
        // Rebind exitoso
        Serial.println("DHCP rebind exitoso");
        Serial.print("Nueva IP: ");
        Serial.println(Ethernet.localIP());
        break;
    default:
        // Sin cambios
        break;
    }
}