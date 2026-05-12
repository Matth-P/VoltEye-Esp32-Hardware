#include <WiFi.h>
#include <HTTPClient.h>
#include <math.h>
#include "secrets.h"

// ==========================
// WIFI
// ==========================

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// ==========================
// SUPABASE
// ==========================

const char* supabaseUrl =
  SUPABASE_URL;

const char* supabaseKey =
  SUPABASE_KEY;

// ==========================
// PINOS
// ==========================

// ACS712
const int pinoCorrente = 33;
// Divisor resistivo
const int pinoTensao = 32;

// LED PWM
const int ledEletrodomestico = 25;

// ==========================
// PWM
// ==========================

const int frequenciaPWM = 5000;
const int resolucaoPWM = 8;

// ==========================
// ADC
// ==========================

const float VREF = 3.3;
const int ADC_RESOLUTION = 4095;

// ==========================
// ACS712
// ==========================

// 5A = 185mV/A
const float sensibilidadeACS = 0.185;

// Offset inicial
float offsetCorrente = 2.5;

// ==========================
// SETUP
// ==========================

void setup() {

  Serial.begin(115200);

  delay(1000);

  Serial.println("Iniciando ESP32...");

  analogSetAttenuation(ADC_11db);

  ledcAttach(
    ledEletrodomestico,
    frequenciaPWM,
    resolucaoPWM
  );

  // ==========================
  // WIFI
  // ==========================

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.println("Conectando WiFi...");
  }

  Serial.println("WiFi conectado!");

  // ==========================
  // CALIBRAÇÃO ACS712
  // ==========================

  long soma = 0;

  for (int i = 0; i < 1000; i++) {

    soma += analogRead(pinoCorrente);

    delay(1);
  }

  float mediaADC = soma / 1000.0;

  offsetCorrente =
    (mediaADC * VREF) / ADC_RESOLUTION;

  Serial.print("Offset calibrado: ");

  Serial.println(offsetCorrente, 3);
}



void loop() {

  // ==========================
  // LEITURA CORRENTE
  // ==========================

  float somaQuadrados = 0;

  int amostras = 1000;

  for (int i = 0; i < amostras; i++) {

    int leituraADC =
      analogRead(pinoCorrente);

    float tensaoSensor =
      (leituraADC * VREF)
      / ADC_RESOLUTION;

    float corrente =
      (tensaoSensor - offsetCorrente)
      / sensibilidadeACS;

    somaQuadrados +=
      corrente * corrente;

    delayMicroseconds(200);
  }

  float correnteRMS =
    sqrt(
      somaQuadrados
      / amostras
    );

  // ==========================
  // LEITURA TENSAO
  // ==========================

  int leituraTensao =
    analogRead(pinoTensao);

  float tensaoADC =
    (leituraTensao * VREF)
    / ADC_RESOLUTION;

  float tensaoReal =
    tensaoADC * 127.0;

  float potencia =
    tensaoReal * correnteRMS;

  // ==========================
  // LED PWM
  // ==========================

  int brilho = map(
    (int)potencia,
    0,
    2500,
    0,
    255
  );

  brilho = constrain(
    brilho,
    0,
    255
  );

  ledcWrite(
    ledEletrodomestico,
    brilho
  );

  // ==========================
  // ALERTAS
  // ==========================

  if (correnteRMS < 0.2) {

    Serial.println(
      "CORRENTE BAIXA!"
    );
  }

  if (tensaoReal < 200) {

    Serial.println(
      "QUEDA DE TENSAO!"
    );
  }

  // ==========================
  // SERIAL
  // ==========================

  Serial.println(
    "========================"
  );

  Serial.print("Tensao: ");

  Serial.print(
    tensaoReal,
    2
  );

  Serial.println(" V");

  Serial.print("Corrente: ");

  Serial.print(
    correnteRMS,
    2
  );

  Serial.println(" A");

  Serial.print("Potencia: ");

  Serial.print(
    potencia,
    2
  );

  Serial.println(" W");

  // ==========================
  // ENVIO SUPABASE
  // ==========================

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    http.begin(
      supabaseUrl
    );

    http.addHeader(
      "Content-Type",
      "application/json"
    );

    http.addHeader(
      "apikey",
      supabaseKey
    );

    http.addHeader(
      "Authorization",
      String("Bearer ")
      + supabaseKey
    );

    String json = "{";

    json += "\"corrente\":";
    json += String(
      correnteRMS,
      2
    );

    json += ",";

    json += "\"tensao\":";
    json += String(
      tensaoReal,
      2
    );

    json += "}";

    Serial.println(
      "JSON enviado:"
    );

    Serial.println(json);

    int response =
      http.POST(json);

    Serial.print(
      "HTTP Response: "
    );

    Serial.println(response);

    http.end();

  } else {

    Serial.println(
      "WiFi desconectado!"
    );
  }

  delay(5000);
}