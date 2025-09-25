#include "ntp_sync.h"

// --- Configuración NTP ---
const char *ntpServer = "pool.ntp.org";
const int ntpPort = 123;
const int timeZoneOffset = -3; // GMT-3 (Argentina) - Ajustar según tu zona horaria
EthernetUDP udp;

// Buffer para paquetes NTP
byte packetBuffer[48];

bool initNTP()
{
    Serial.println("=== Inicializando cliente NTP ===");

    if (udp.begin(8888))
    { // Puerto local para UDP
        Serial.println("✅ Cliente NTP inicializado en puerto 8888");
        return true;
    }
    else
    {
        Serial.println("❌ Error inicializando cliente NTP");
        return false;
    }
}

bool syncRTCWithNTP()
{
    Serial.println("\n--- Sincronizando RTC con servidor NTP ---");
    Serial.print("Servidor NTP: ");
    Serial.println(ntpServer);
    Serial.print("Zona horaria: GMT");
    if (timeZoneOffset >= 0)
        Serial.print("+");
    Serial.println(timeZoneOffset);

    unsigned long ntpTime = getNTPTime();

    if (ntpTime == 0)
    {
        Serial.println("❌ Error obteniendo hora de NTP");
        return false;
    }

    // Convertir tiempo NTP a DateTime
    DateTime syncedTime = convertNTPToDateTime(ntpTime);

    // Ajustar RTC con la nueva hora
    extern RTC_DS1307 rtc;
    rtc.adjust(syncedTime);

    printNTPSyncStatus(true, syncedTime);
    return true;
}

unsigned long getNTPTime()
{
    // Limpiar buffer
    memset(packetBuffer, 0, 48);

    // Enviar solicitud NTP
    sendNTPRequest();

    // Esperar respuesta (timeout de 5 segundos)
    unsigned long startTime = millis();
    while (millis() - startTime < 5000)
    {
        unsigned long ntpTime;
        if (receiveNTPResponse(ntpTime))
        {
            return ntpTime;
        }
        delay(10);
    }

    Serial.println("⏱️ Timeout esperando respuesta NTP");
    return 0;
}

DateTime convertNTPToDateTime(unsigned long ntpTime)
{
    // NTP timestamp es desde 1900, Unix timestamp desde 1970
    // Diferencia: 2208988800 segundos (70 años)
    const unsigned long seventyYears = 2208988800UL;
    unsigned long unixTime = ntpTime - seventyYears;

    // Aplicar offset de zona horaria
    unixTime += (timeZoneOffset * 3600);

    return DateTime(unixTime);
}

void sendNTPRequest()
{
    // Construir paquete NTP
    packetBuffer[0] = 0b11100011; // LI, Version, Mode
    packetBuffer[1] = 0;          // Stratum
    packetBuffer[2] = 6;          // Polling Interval
    packetBuffer[3] = 0xEC;       // Peer Clock Precision

    // 8 bytes de ceros para Root Delay & Root Dispersion
    for (int i = 4; i < 12; i++)
    {
        packetBuffer[i] = 0;
    }

    // Reference ID
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;

    Serial.print("Enviando solicitud NTP a ");
    Serial.print(ntpServer);
    Serial.print("...");

    if (udp.beginPacket(ntpServer, ntpPort))
    {
        udp.write(packetBuffer, 48);
        if (udp.endPacket())
        {
            Serial.println(" OK");
        }
        else
        {
            Serial.println(" Error en endPacket");
        }
    }
    else
    {
        Serial.println(" Error en beginPacket");
    }
}

bool receiveNTPResponse(unsigned long &ntpTime)
{
    int packetSize = udp.parsePacket();

    if (packetSize >= 48)
    {
        Serial.println("✅ Respuesta NTP recibida");

        // Leer paquete
        udp.read(packetBuffer, 48);

        // Extraer timestamp del campo "Transmit Timestamp" (bytes 40-43)
        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

        ntpTime = (highWord << 16) | lowWord;

        Serial.print("Timestamp NTP: ");
        Serial.println(ntpTime);
        return true;
    }

    return false;
}

void printNTPSyncStatus(bool success, DateTime syncedTime)
{
    if (success)
    {
        Serial.println("✅ RTC sincronizado exitosamente con NTP");
        Serial.print("Nueva fecha/hora: ");
        Serial.print(syncedTime.day());
        Serial.print("/");
        Serial.print(syncedTime.month());
        Serial.print("/");
        Serial.print(syncedTime.year());
        Serial.print(" ");

        if (syncedTime.hour() < 10)
            Serial.print("0");
        Serial.print(syncedTime.hour());
        Serial.print(":");
        if (syncedTime.minute() < 10)
            Serial.print("0");
        Serial.print(syncedTime.minute());
        Serial.print(":");
        if (syncedTime.second() < 10)
            Serial.print("0");
        Serial.println(syncedTime.second());

        extern char daysOfTheWeek[7][12];
        Serial.print("Día de la semana: ");
        Serial.println(daysOfTheWeek[syncedTime.dayOfTheWeek()]);
    }
    else
    {
        Serial.println("❌ Error sincronizando RTC con NTP");
    }
    Serial.println("----------------------------------");
}
