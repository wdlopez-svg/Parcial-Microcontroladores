#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>
#include <Arduino_FreeRTOS.h>
#include <EEPROM.h>

RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

String hora_str = "";
const int pinSwitch = 2;
int lastState = HIGH;   // Estado anterior del switch

// EEPROM: guardamos en dirección 0
#define EEPROM_ADDR 0

// Prototipos de tareas
void TaskStartup(void *pvParameters);
void TaskSwitch(void *pvParameters);
void TaskHora(void *pvParameters);

void setup() {
  Wire.begin(0x08);               // Dirección I2C del UNO = 0x08
  Wire.onRequest(requestEvent);   // Callback cuando maestro pide datos

  rtc.begin();
  lcd.init();
  lcd.backlight();

  pinMode(pinSwitch, INPUT_PULLUP);

  // Crear SOLO la tarea de arranque
  xTaskCreate(TaskStartup, "Startup", 128, NULL, 3, NULL); // máxima prioridad
}

void loop() {
  // vacío, RTOS maneja todo
}

// -------------------- TAREA STARTUP --------------------
void TaskStartup(void *pvParameters) {
  (void) pvParameters;

  // Leer último estado guardado
  int savedState = EEPROM.read(EEPROM_ADDR);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Ultimo estado:");
  lcd.setCursor(0,1);
  if (savedState == LOW) {
    lcd.print("PULSADO");
  } else {
    lcd.print("LIBERADO");
  }

  vTaskDelay(pdMS_TO_TICKS(5000)); // mostrar 5 segundos

  // Crear las demás tareas DESPUÉS del arranque
  xTaskCreate(TaskSwitch, "Switch", 128, NULL, 2, NULL); // alta prioridad
  xTaskCreate(TaskHora,   "Hora",   256, NULL, 1, NULL); // baja prioridad

  // Luego esta tarea se destruye
  vTaskDelete(NULL);
}

// -------------------- TAREA SWITCH --------------------
void TaskSwitch(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    int estado = digitalRead(pinSwitch);
    if (estado != lastState) {
      lastState = estado;

      // Guardar en EEPROM
      EEPROM.update(EEPROM_ADDR, estado);

      // Mostrar cambio en la segunda línea
      lcd.setCursor(0,1);
      lcd.print("                "); // limpiar línea
      lcd.setCursor(0,1);
      if (estado == LOW) {
        lcd.print("PULSADO");
      } else {
        lcd.print("LIBERADO");
      }

      vTaskDelay(pdMS_TO_TICKS(1000)); // Mostrar 1s
    }
    vTaskDelay(pdMS_TO_TICKS(50)); // pequeña pausa para antirrebote
  }
}

// -------------------- TAREA HORA --------------------
void TaskHora(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    DateTime now = rtc.now();

    // Preparar cadena para enviar al ESP32
    hora_str = String(now.hour()) + "," + String(now.minute()) + "," + String(now.second());

    // Mostrar hora en primera línea
    lcd.setCursor(0,0);
    lcd.print("Hora: ");
    if(now.hour()<10) lcd.print("0");
    lcd.print(now.hour()); lcd.print(":");
    if(now.minute()<10) lcd.print("0");
    lcd.print(now.minute()); lcd.print(":");
    if(now.second()<10) lcd.print("0");
    lcd.print(now.second());

    // Si no hubo cambio reciente, mantener fecha en la segunda línea
    if (digitalRead(pinSwitch) == lastState) {
      lcd.setCursor(0,1);
      if(now.day()<10) lcd.print("0");
      lcd.print(now.day()); lcd.print("/");
      if(now.month()<10) lcd.print("0");
      lcd.print(now.month()); lcd.print("/");
      lcd.print(now.year());
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// -------------------- EVENTO I2C --------------------
void requestEvent() {
  Wire.write(hora_str.c_str());
}
