# Sistema de Monitoreo de Semáforos

## Descripción
Sistema basado en ESP32 que detecta el estado de 4 semáforos mediante entradas digitales, registra timestamps cuando se enciende/apaga la luz roja de cada semáforo, y envía estos datos mediante peticiones POST HTTP.

## Instalación y Compilación


### Paso 1: Instalar Visual Studio Code
1. Descarga VS Code desde [code.visualstudio.com](https://code.visualstudio.com/)
2. Instala con configuraciones por defecto
3. Abre VS Code

### Paso 2: Instalar PlatformIO Extension
1. En VS Code, haz clic en el ícono de **Extensions** (cuadrado con 4 cuadritos) en la barra lateral izquierda
2. Busca "**PlatformIO IDE**"
3. Haz clic en **Install** en la extensión oficial de PlatformIO
4. **Reinicia VS Code** después de la instalación

### Paso 3: Descargar el Código
#### Opción A: Clonar desde GitHub (Recomendado)
1. Abre la terminal en VS Code (`Ctrl + Shift + ` ` o `Terminal > New Terminal`)
2. Navega a donde quieres guardar el proyecto:
   ```bash
   cd C:\Users\TuUsuario\Documents
   ```
3. Clona el repositorio:
   ```bash
   git clone https://github.com/agustinbene/esp32-traffic-light-monitor
   ```
4. Abre la carpeta del proyecto:
   ```bash
   cd esp32-traffic-light-monitor
   code .
   ```

#### Opción B: Descargar ZIP
1. Ve al repositorio en GitHub
2. Haz clic en **Code** > **Download ZIP**
3. Extrae el archivo ZIP
4. En VS Code: **File** > **Open Folder** > Selecciona la carpeta extraída

### Paso 4: Abrir el Proyecto en PlatformIO
1. Una vez abierto el proyecto, VS Code debería detectar automáticamente que es un proyecto PlatformIO
2. En la barra de tareas inferior, verás el ícono de PlatformIO (🏠)
3. Si no aparece automáticamente, ve a **View** > **Command Palette** (`Ctrl + Shift + P`) y busca "PlatformIO: Home"

### Paso 5: Conectar el ESP32
1. Conecta tu ESP32 al puerto USB de tu computadora
2. **Instalar drivers** (si es necesario):
   - **ESP32 con chip CP2102**: [Descargar driver CP2102](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
   - **ESP32 con chip CH340**: [Descargar driver CH340](http://www.wch-ic.com/downloads/CH341SER_ZIP.html)
3. Reinicia la computadora después de instalar drivers

### Paso 6: Verificar la Configuración
Revisa que el archivo `platformio.ini` tenga la configuración correcta:

```ini
[env:esp32cam]
platform = espressif32
board = esp32cam
framework = arduino
lib_deps = 
    adafruit/RTClib@^2.1.1
    ethernet2@^1.0.4
    ArduinoJson@^6.21.3
monitor_speed = 115200
upload_speed = 921600
```

### Paso 7: Compilar el Proyecto
1. **Abrir PlatformIO Terminal**: En la barra inferior, haz clic en el ícono de terminal de PlatformIO
2. **Compilar**: Ejecuta el comando:
   ```bash
   pio run
   ```
   O usa el atajo: `Ctrl + Alt + B`

**Si todo está bien**, deberías ver algo como:
```
SUCCESS: Took X.XX seconds
```

### Paso 8: Subir el Código al ESP32
1. **Verificar puerto**: En la barra inferior de VS Code, asegúrate de que esté seleccionado el puerto correcto (ej: COM3, COM4)
2. **Subir código**:
   ```bash
   pio run --target upload
   ```
   O usa el atajo: `Ctrl + Alt + U`

### Paso 9: Monitorear la Salida Serial
Para ver los mensajes del ESP32:
```bash
pio device monitor
```
O usa el atajo: `Ctrl + Alt + S`

Para salir del monitor serial, presiona `Ctrl + C`

## Características Principales
- **Detección de 4 semáforos**: Monitoreo simultáneo de 4 luces rojas
- **Registro preciso de tiempo**: Utiliza RTC DS1307 sincronizado con NTP
- **Anti-rebote**: Sistema de debounce para evitar falsas detecciones
- **Buffer de sesiones**: Almacena hasta 20 sesiones completadas para envío
- **Conectividad Ethernet**: Envío de datos vía W5100
- **Retry automático**: Si falla el envío, los datos se conservan para reintento

## Conexiones de Hardware

### Semáforos (Entradas Digitales)
- **Semáforo 1**: Pin 2 (GPIO2)
- **Semáforo 2**: Pin 4 (GPIO4)  
- **Semáforo 3**: Pin 5 (GPIO5)
- **Semáforo 4**: Pin 18 (GPIO18)

**Configuración de entradas**: Pull-up interno activado
- **Lógica**: LOW = luz roja encendida, HIGH = luz roja apagada

### RTC DS1307 (I2C)
- **SDA**: Pin 16 (GPIO16)
- **SCL**: Pin 0 (GPIO0)

### Ethernet W5100 (SPI)
- **MISO**: Pin 12 (GPIO12)
- **MOSI**: Pin 13 (GPIO13)
- **SCK**: Pin 14 (GPIO14)
- **CS**: Pin 15 (GPIO15)

## Estructura de Datos JSON

### Datos de Semáforos (Enviados a `/traffic_lights`)
```json
{
  "device_id": "ESP32CAM_TRAFFIC_MONITOR",
  "request_number": 123,
  "uptime_seconds": 45678,
  "rtc_status": "running",
  "current_date": "2025-08-12",
  "current_time": "14:30:25",
  "unix_timestamp": 1723467025,
  "day_of_week": "Monday",
  "traffic_light_sessions": [
    {
      "traffic_light_id": 1,
      "start_timestamp": 1723467000,
      "end_timestamp": 1723467090,
      "duration_seconds": 90,
      "start_date": "2025-08-12",
      "start_time": "14:30:00",
      "end_date": "2025-08-12",
      "end_time": "14:31:30"
    }
  ],
  "total_sessions": 1
}
```

### Heartbeat (Enviado a `/w5100` cuando no hay datos de semáforos)
```json
{
  "device_id": "ESP32CAM_W5100_RTC",
  "request_number": 124,
  "uptime_seconds": 45683,
  "rtc_status": "running",
  "current_date": "2025-08-12",
  "current_time": "14:30:30",
  "unix_timestamp": 1723467030,
  "day_of_week": "Monday"
}
```

## Funcionamiento del Sistema

### 1. Detección de Estados
- El sistema lee continuamente las 4 entradas digitales
- Aplica debounce de 50ms para evitar falsas detecciones
- Detecta cambios de estado (rojo ON/OFF)

### 2. Registro de Sesiones
- **Inicio de sesión**: Cuando la luz roja se enciende, registra timestamp
- **Fin de sesión**: Cuando la luz roja se apaga, registra timestamp y calcula duración
- **Almacenamiento**: La sesión completada se guarda en buffer para envío

### 3. Envío de Datos
- **Prioridad**: Los datos de semáforos tienen prioridad sobre heartbeats
- **Endpoint**: `/traffic_lights` para datos de semáforos, `/w5100` para heartbeat
- **Retry**: Si falla el envío, los datos se conservan para reintento
- **Limpieza**: Buffer se limpia solo después de envío exitoso

### 4. Monitoreo y Debug
- Estado actual de todos los semáforos
- Sesiones activas en curso
- Sesiones pendientes para envío
- Información del RTC y conectividad

## Configuración

### Intervalos de Tiempo
- **Envío de datos**: 5000ms (5 segundos)
- **Debounce**: 50ms
- **Buffer máximo**: 20 sesiones

### Servidor de Destino
```cpp
const char *host = "bot.abenegas.com.ar";
const int port = 80;
```

### Modificación de Pines
Para cambiar los pines de los semáforos, editar en `traffic_lights.h`:
```cpp
#define TRAFFIC_LIGHT_1_PIN 2
#define TRAFFIC_LIGHT_2_PIN 4
#define TRAFFIC_LIGHT_3_PIN 5
#define TRAFFIC_LIGHT_4_PIN 18
```

## Salida Serial de Ejemplo

```
=== ESP32CAM + RTC DS1307 + W5100 + NTP + SEMÁFOROS ===
=== Inicializando módulo de red W5100 ===
✅ Módulo de red inicializado correctamente.
=== Inicializando sistema de semáforos ===
Semáforo 1 (Pin 2): 🟢 NO ROJO
Semáforo 2 (Pin 4): 🟢 NO ROJO
Semáforo 3 (Pin 5): 🔴 ROJO
Semáforo 4 (Pin 18): 🟢 NO ROJO
✅ Sistema de semáforos inicializado.

🚦 Cambio detectado en semáforo 1: 🔴 ROJO ENCENDIDO
   Timestamp inicio: 2025-08-12 14:30:00

🚦 Cambio detectado en semáforo 1: 🟢 ROJO APAGADO
   Timestamp fin: 2025-08-12 14:31:30
   ✅ Sesión registrada para envío

[#15] Enviando datos de 1 sesiones de semáforos:
✅ Datos de semáforos enviados exitosamente.
```

