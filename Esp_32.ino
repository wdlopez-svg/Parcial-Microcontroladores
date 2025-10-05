#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

// Credenciales WiFi
const char* ssid = "HCV 2932";
const char* password = "HOTELhcv";

// Broker MQTT
const char* mqtt_server = "192.168.10.26";

WiFiClient espClient;
PubSubClient client(espClient);

// =======================
// Conexión WiFi
// =======================
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("IP local: ");
  Serial.println(WiFi.localIP());
}

// =======================
// Reconectar MQTT
// =======================
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("conectado");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);  // SDA=21, SCL=22 en ESP32
  if (!rtc.begin()) {
    Serial.println("No se encontró el RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC reseteado, ajustando fecha/hora...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  DateTime now = rtc.now();

  // Crear mensaje con la hora
  char msg[32];
  snprintf(msg, sizeof(msg), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

  Serial.print("Hora enviada por MQTT: ");
  Serial.println(msg);

  // Publicar en el topic
  client.publish("esp32/hora", msg);

  delay(1000);
}
