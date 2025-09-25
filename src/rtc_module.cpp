#include <Arduino.h>
#include "rtc_module.h"
#include "ntp_sync.h"

// --- Variables globales del RTC ---
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void initRTC()
{
    Serial.println("=== Inicializando RTC DS1307 ===");

    // Inicializar I2C
    Wire.begin(SDA_PIN, SCL_PIN);

    if (!rtc.begin())
    {
        Serial.println("❌ No se detecta RTC DS1307.");
        Serial.println("Verifica las conexiones:");
        Serial.println("  VCC -> 3.3V o 5V");
        Serial.println("  GND -> GND");
        Serial.print("  SDA -> GPIO ");
        Serial.println(SDA_PIN);
        Serial.print("  SCL -> GPIO ");
        Serial.println(SCL_PIN);
        while (1)
            delay(10);
    }

    if (!rtc.isrunning())
    {
        Serial.println("⚠️ RTC no está funcionando. Ajustando hora...");
        setRTCTimeFromCompilation();
    }

    DateTime now = rtc.now();
    Serial.println("✅ RTC DS1307 funcionando:");
    Serial.print("Fecha: ");
    Serial.println(getFormattedDate());
    Serial.print("Hora: ");
    Serial.println(getFormattedTime());
    Serial.println("------------------------------");
}

void printRTCInfo()
{
    if (!isRTCRunning())
    {
        Serial.println("❌ RTC no está funcionando!");
        return;
    }

    DateTime now = getCurrentTime();

    Serial.println("\n--- Información del RTC ---");
    Serial.print("Fecha completa: ");
    Serial.println(getFormattedDateTime());
    Serial.print("Día de la semana: ");
    Serial.println(daysOfTheWeek[now.dayOfTheWeek()]);

    Serial.print("Unix timestamp: ");
    Serial.print(getUnixTimestamp());
    Serial.print("s = ");
    Serial.print(getUnixTimestamp() / 86400L);
    Serial.println("d desde 1/1/1970");

    // Calcular fecha futura (ejemplo)
    DateTime future(now + TimeSpan(7, 12, 30, 6));
    Serial.print("En 7 días, 12h, 30m, 6s será: ");
    Serial.print(future.day());
    Serial.print("/");
    Serial.print(future.month());
    Serial.print("/");
    Serial.print(future.year());
    Serial.print(" ");
    if (future.hour() < 10)
        Serial.print("0");
    Serial.print(future.hour());
    Serial.print(":");
    if (future.minute() < 10)
        Serial.print("0");
    Serial.print(future.minute());
    Serial.print(":");
    if (future.second() < 10)
        Serial.print("0");
    Serial.println(future.second());
    Serial.println("---------------------------");
}

DateTime getCurrentTime()
{
    return rtc.now();
}

bool isRTCRunning()
{
    return rtc.isrunning();
}

void setRTCTime(DateTime dateTime)
{
    rtc.adjust(dateTime);
    Serial.println("✅ Hora del RTC ajustada.");
}

void setRTCTimeFromCompilation()
{
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println("✅ RTC ajustado con hora de compilación.");
}

String getFormattedDate()
{
    DateTime now = getCurrentTime();
    String date = "";

    if (now.day() < 10)
        date += "0";
    date += String(now.day()) + "/";

    if (now.month() < 10)
        date += "0";
    date += String(now.month()) + "/";

    date += String(now.year());

    return date;
}

String getFormattedTime()
{
    DateTime now = getCurrentTime();
    String time = "";

    if (now.hour() < 10)
        time += "0";
    time += String(now.hour()) + ":";

    if (now.minute() < 10)
        time += "0";
    time += String(now.minute()) + ":";

    if (now.second() < 10)
        time += "0";
    time += String(now.second());

    return time;
}

String getFormattedDateTime()
{
    return getFormattedDate() + " " + getFormattedTime();
}

uint32_t getUnixTimestamp()
{
    DateTime now = getCurrentTime();
    return now.unixtime();
}

void initRTCWithNTPSync()
{
    Serial.println("=== Inicializando RTC DS1307 con sincronización NTP ===");

    // Inicializar I2C
    Wire.begin(SDA_PIN, SCL_PIN);

    if (!rtc.begin())
    {
        Serial.println("❌ No se detecta RTC DS1307.");
        Serial.println("Verifica las conexiones:");
        Serial.println("  VCC -> 3.3V o 5V");
        Serial.println("  GND -> GND");
        Serial.print("  SDA -> GPIO ");
        Serial.println(SDA_PIN);
        Serial.print("  SCL -> GPIO ");
        Serial.println(SCL_PIN);
        while (1)
            delay(10);
    }

    // Mostrar hora actual antes de la sincronización
    if (rtc.isrunning())
    {
        DateTime currentTime = rtc.now();
        Serial.println("⏰ Hora actual del RTC antes de sincronizar:");
        Serial.print("   ");
        Serial.println(getFormattedDateTime());
    }
    else
    {
        Serial.println("⚠️ RTC no está funcionando.");
    }

    // Inicializar cliente NTP
    if (initNTP())
    {
        // Intentar sincronizar con NTP
        Serial.println("\n🌐 Intentando sincronizar con servidor NTP...");

        if (syncRTCWithNTP())
        {
            Serial.println("✅ RTC sincronizado exitosamente con NTP");
        }
        else
        {
            Serial.println("❌ Error en sincronización NTP, usando hora de compilación");
            setRTCTimeFromCompilation();
        }
    }
    else
    {
        Serial.println("❌ Error inicializando NTP, usando hora de compilación");
        if (!rtc.isrunning())
        {
            setRTCTimeFromCompilation();
        }
    }

    // Mostrar hora final
    DateTime finalTime = rtc.now();
    Serial.println("\n✅ RTC DS1307 inicializado:");
    Serial.print("Fecha final: ");
    Serial.println(getFormattedDate());
    Serial.print("Hora final: ");
    Serial.println(getFormattedTime());
    Serial.print("Día: ");
    Serial.println(daysOfTheWeek[finalTime.dayOfTheWeek()]);
    Serial.println("------------------------------------------");
}
