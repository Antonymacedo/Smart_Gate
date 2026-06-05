#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>

// --- Configurações WiFi e SinricPro
#define WIFI_SSID "JACQUE"
#define WIFI_PASS "1108257370"
#define APP_KEY "e3276e31-8ae1-4d5a-86d0-282d37908333"
#define APP_SECRET "6cc0b9de-6027-4a36-8dbe-659bc897002b-35787aa1-b003-42a1-8f9b-067ed3923edc"
#define SWITCH_ID "687c3e69929fca43027f9536"

// --- Pinos
#define RELAY_PIN D5
#define BUTTON_PIN D6

ESP8266WebServer server(80);

bool estadoAnteriorBotao = HIGH; // pull-up
bool estadoRelé = false;

// --- Função para acionar o relé por 1 segundo
void acionarRele() {
  digitalWrite(RELAY_PIN, LOW);  // Ativa relé (nível baixo)
  delay(800);                   // Mantém acionado por 0,8s
  digitalWrite(RELAY_PIN, HIGH); // Desativa relé
  Serial.println("Relé acionado");
}

// --- Callback SinricPro (Alexa)
bool onPowerState(const String &deviceId, bool &state) {
  if (deviceId == SWITCH_ID) {
    acionarRele();
    return true;
  }
  return false;
}

// --- Requisição HTTP para acionar relé
void handleAcionar() {
  acionarRele();
  server.send(200, "text/html", "<h1>Portão acionado</h1><a href=\"/acionar\"><button>Acionar Novamente</button></a>");
}

// --- Configura Wi-Fi
void setupWiFi() {
  Serial.print("Conectando ao WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Relé desativado inicialmente

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  setupWiFi();

  server.on("/acionar", handleAcionar);
  server.begin();
  Serial.println("Servidor HTTP iniciado");

  SinricProSwitch &meuSwitch = SinricPro[SWITCH_ID];
  meuSwitch.onPowerState(onPowerState);

  SinricPro.begin(APP_KEY, APP_SECRET);
  SinricPro.restoreDeviceStates(true);
}

void loop() {
  SinricPro.handle();
  server.handleClient();

  // Leitura do botão com debounce simples
  bool estadoBotao = digitalRead(BUTTON_PIN);
  if (estadoBotao == LOW && estadoAnteriorBotao == HIGH) {
    Serial.println("Botão pressionado");
    acionarRele();
    delay(300); // debounce
  }
  estadoAnteriorBotao = estadoBotao;
}
