# Proyecto: Comunicación RTC DS3231 con ESP32 y Arduino UNO

Este repositorio contiene dos ejemplos de código que trabajan en conjunto con el módulo **RTC DS3231**.  
- **ESP32**: Obtiene la hora desde el RTC y la publica mediante **MQTT**.  
- **Arduino UNO**: Muestra la hora en un **LCD I2C**, gestiona un **switch** con almacenamiento en EEPROM y utiliza **FreeRTOS** para manejar tareas concurrentes.  

---

## Hardware requerido
- Módulo RTC DS3231  
- ESP32 DevKit V1  
- Arduino UNO  
- Pantalla LCD 16x2 con adaptador I2C (dirección `0x27`)  
- Switch/botón con resistencia interna (INPUT_PULLUP)  

---

## Archivos del proyecto
- **`esp32_mqtt_rtc.ino`** → Código para el ESP32.  
- **`arduino_uno_rtos_lcd.ino`** → Código para el Arduino UNO.  

---

## ESP32: Publicación de la hora vía MQTT

### Funcionalidad
- Se conecta a una red WiFi definida en el código.  
- Se comunica con un **broker MQTT** configurado en la variable `mqtt_server`.  
- Obtiene la hora del **RTC DS3231** mediante I2C.  
- Publica cada segundo la hora en el tópico `esp32/hora`.  

### Librerías necesarias
- `WiFi.h`  
- `PubSubClient.h`  
- `Wire.h`  
- `RTClib.h`  

### Flujo principal
1. Conexión a WiFi.  
2. Conexión al broker MQTT.  
3. Lectura de la hora actual.  
4. Publicación del mensaje en formato `HH:MM:SS`.  

---

## Arduino UNO: Visualización y gestión con FreeRTOS

### Funcionalidad
- Configura el UNO como **esclavo I2C** en la dirección `0x08`.  
- Obtiene la hora del RTC DS3231 y la muestra en el **LCD I2C**.  
- Utiliza **FreeRTOS** para gestionar varias tareas en paralelo:  
  - **Startup**: Muestra en el LCD el último estado guardado del switch.  
  - **Switch**: Lee el estado del botón y lo guarda en EEPROM.  
  - **Hora**: Muestra la hora actualizada en la primera línea y la fecha en la segunda.  
- Responde al maestro I2C (ej. el ESP32) enviando la hora como string.  

### Librerías necesarias
- `Wire.h`  
- `RTClib.h`  
- `LiquidCrystal_I2C.h`  
- `Arduino_FreeRTOS.h`  
- `EEPROM.h`  

### Flujo principal
1. Se inicializa el LCD y el RTC.  
2. Se ejecuta la tarea de arranque, mostrando el último estado del switch guardado en EEPROM.  
3. Posteriormente, se ejecutan las tareas de lectura de switch y actualización de hora.  
4. Cuando otro dispositivo (ej. ESP32) solicita datos vía I2C, se responde con la hora.  

---

## Integración ESP32 + Arduino UNO
- El **ESP32** actúa como maestro I2C y puede solicitar la hora al **UNO**, que responde con el string generado en `requestEvent()`.  
- En paralelo, el ESP32 publica por MQTT la hora leída directamente desde el RTC.  
- De esta forma, se tiene un sistema híbrido:  
  - El **UNO** muestra y guarda estado en LCD/EEPROM.  
  - El **ESP32** comunica la información a la red.  

---

## Cómo usar
1. Cargar `esp32_mqtt_rtc.ino` en la ESP32.  
2. Cargar `arduino_uno_rtos_lcd.ino` en el Arduino UNO.  
3. Conectar ambos al **RTC DS3231** compartiendo líneas SDA/SCL.  
4. Conectar el LCD al Arduino UNO.  
5. Configurar las credenciales WiFi y la IP del broker MQTT en el código de la ESP32.  
6. Iniciar el broker MQTT (ej. Mosquitto) y suscribirse al tópico `esp32/hora`.  

---

## Ejemplo de salida ESP32
