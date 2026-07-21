#include <Arduino.h>
#include <math.h>

#include <WiFi.h>
#include <HTTPClient.h>

#include "secrets.h"


// ==========================
// PINOS
// ==========================

const int pinoCorrente = 33;
const int pinoTensao = 32;



// ==========================
// ADC
// ==========================

const float VREF = 3.3;
const int ADC_RESOLUTION = 4095;



// ==========================
// ACS712 COM DIVISOR
// 1k + 2k
// ==========================

const float sensibilidadeACS = 0.066;

float offsetADC = 0;



// ==========================
// ZMPT101B
// divisor 1k/2k
// ==========================

float fatorTensao = 607.0;



// ==========================
// WIFI
// ==========================

void conectarWiFi(){


  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );


  Serial.print("Conectando WiFi");


  while(WiFi.status() != WL_CONNECTED){


    delay(500);

    Serial.print(".");

  }


  Serial.println();

  Serial.println("WiFi conectado!");

}



// ==========================
// ENVIO SUPABASE
// ==========================

void enviarDados(
  float corrente,
  float tensao,
  float potencia
){

  if(WiFi.status() == WL_CONNECTED){


    HTTPClient http;


    http.begin(API_URL);


    http.addHeader(
      "apikey",
      SUPABASE_KEY
    );


    http.addHeader(
      "Authorization",
      "Bearer " SUPABASE_KEY
    );


    http.addHeader(
      "Content-Type",
      "application/json"
    );



    String json = "{";


    json += "\"device_id\":\"";
    json += DEVICE_ID;
    json += "\",";


    json += "\"voltage\":";

    json += String(
      tensao,
      2
    );


    json += ",";


    json += "\"current\":";

    json += String(
      corrente,
      2
    );


    json += "}";



    Serial.println("JSON enviado:");

    Serial.println(json);



    int resposta = http.POST(json);



    Serial.print("HTTP:");

    Serial.println(resposta);



    Serial.println(
      http.getString()
    );



    http.end();

  }

}




void setup(){


  Serial.begin(115200);


  delay(1000);


  analogSetAttenuation(ADC_11db);



  conectarWiFi();



  // ==========================
  // CALIBRA ACS712
  // ==========================


  Serial.println("Calibrando ACS712...");



  long soma = 0;



  for(int i = 0; i < 10000; i++){


    soma += analogRead(pinoCorrente);



    delayMicroseconds(500);

  }



  offsetADC =
  soma / 10000.0;



  Serial.print("Offset ACS ADC: ");

  Serial.println(offsetADC);



}








void loop(){


  int amostras = 3000;



  // ==========================
  // CORRENTE RMS ACS712
  // ==========================


  float somaCorrente = 0;



  for(int i = 0; i < amostras; i++){



    int leitura =
    analogRead(pinoCorrente);



    float diferencaADC =
    leitura - offsetADC;



    if(abs(diferencaADC) < 20){

      diferencaADC = 0;

    }



    float tensao =
    (diferencaADC * VREF)
    /
    ADC_RESOLUTION;



    float corrente =
    tensao / sensibilidadeACS;



    somaCorrente += corrente*corrente;



    delayMicroseconds(200);


  }





  float correnteRMS =
  sqrt(
    somaCorrente/amostras
  );



  if(correnteRMS < 1.0){

    correnteRMS = 0;

  }








  // ==========================
  // TENSAO RMS ZMPT101B
  // ==========================


  float somaADC = 0;



  for(int i=0;i<amostras;i++){


    somaADC += analogRead(pinoTensao);



    delayMicroseconds(200);

  }




  float mediaADC =
  somaADC/amostras;






  float somaQuadrados = 0;



  for(int i=0;i<amostras;i++){



    float leitura =
    analogRead(pinoTensao);



    float diferenca =
    leitura-mediaADC;



    somaQuadrados +=
    diferenca*diferenca;



    delayMicroseconds(200);

  }




  float rmsADC =
  sqrt(
    somaQuadrados/amostras
  );





  float tensaoReal =
  (
    rmsADC *
    VREF /
    ADC_RESOLUTION
  )
  *
  fatorTensao;



  // filtro ruído

  if(tensaoReal < 50){

    tensaoReal = 0;

  }







  // ==========================
  // POTENCIA
  // ==========================


  float potencia =
  tensaoReal * correnteRMS;







  // ==========================
  // SERIAL
  // ==========================


  Serial.println("===================");



  Serial.print("Tensao: ");

  Serial.print(tensaoReal,2);

  Serial.println(" V");



  Serial.print("Corrente: ");

  Serial.print(correnteRMS,2);

  Serial.println(" A");



  Serial.print("Potencia: ");

  Serial.print(potencia,2);

  Serial.println(" W");



  Serial.print("ADC ZMPT: ");

  Serial.println(
    analogRead(pinoTensao)
  );




  // ENVIA PARA API

  enviarDados(
    correnteRMS,
    tensaoReal,
    potencia
  );



  delay(10000);


}