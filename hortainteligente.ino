#define BLYNK_TEMPLATE_ID "TMPL2E5SHR6uU"
#define BLYNK_TEMPLATE_NAME "Temperatura"
#define BLYNK_AUTH_TOKEN "keRSDOyRycx85TLoShWhPOfMYsl6bNy7"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp8266.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DS18B20 4  //Coneção com GPIO2 
OneWire ourWire(DS18B20);
DallasTemperature sensor(&ourWire);


char auth[] = BLYNK_AUTH_TOKEN;

// Credenciais da rede WiFi.
char ssid[] = "internet";
char pass[] = "senha_internet";

int bomba = 14;  // D5 - Liga bomba de irrigação
int led = 2;     // D4 - Led da modulo ESP8266
int ldr = 16;    // D0 - Sensor de luminosidade
int ena = 12;    // D6 - Enable A
int in1 = 13;    // D7 - in 1 Regulador
int in2 = 15;    // D8 - in 2 Regulador
int valorSeco = 750;  // valor lido com o solo seco (referência)
int valorMolhado = 200; // valor lido com o solo saturado (referência)

int valueLed = 1;
int valueBomba = 1;
int manual = 0; // indicador de acionamento manual
float delta;
float lastHumidity;
float umidade;

BlynkTimer timer;

void sendSensor() {
  sensor.requestTemperatures();
  int tempC = sensor.getTempCByIndex(0);
  delay(1000);
  Blynk.virtualWrite(V0, tempC);
  delay(500);
}

void lerSensores() {

  sensorUmidade();
}

void sensorUmidade() {
  int valorSensor = analogRead(A0);
  int percentualUmidade = (valorSensor - valorSeco) * (100 / (valorMolhado - valorSeco));
  if (percentualUmidade < 0) {
    percentualUmidade = 0;
  } else if (percentualUmidade > 100) {
    percentualUmidade = 100;
  }
  umidade = percentualUmidade;
  Blynk.virtualWrite(V3, percentualUmidade)
}

void lightAct() {
  if (!manual) {
    if (digitalRead(ldr)) {
      digitalWrite(led, 0);
      valueLed = 0;
      Serial.print("Valor do Led");
      Serial.println(valueLed);
    } else {
      digitalWrite(led, 1);
      valueLed = 1;
      Serial.print("Valor do Led");
      Serial.println(valueLed);
    }
  }
}

BLYNK_WRITE(V1) {  // se acontece um evento no botão da luz entra aqui blynk manual  + comanda Luz
  if (manual) {
    valueLed = param.asInt();
    valueLed = !valueLed;
    digitalWrite(led, valueLed);
  }
}

void humidityAct() {
  if (!manual) {
    if (umidade > 60) {
      analogWrite(ena, LOW);
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
      digitalWrite(bomba, 1);
      valueBomba = 1;
    } else if (umidade <= 60) {
      analogWrite(ena, 200);
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      digitalWrite(bomba, 0);
      valueBomba = 0;
    }
  }
}



BLYNK_WRITE(V6) {  // se acontece um evento no pino virtual V6 entra aqui
  if (manual) {
    valueBomba = param.asInt();
    if (valueBomba == 1) { // liga bomba
      analogWrite(ena, 200);
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      digitalWrite(bomba, 0);
      valueBomba = 0;
    } else {
      valueBomba = !valueBomba; // desliga bomba
      analogWrite(ena, LOW);
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
      digitalWrite(bomba, 1);
    }
  }
}

BLYNK_WRITE(V7) {  // se o botão manual é clicado entra aqui
  manual = param.asInt();
}

void buttonLedWidget() {
  Blynk.virtualWrite(V1, !valueLed);    // Led informativo de luz ligada/desligada
  Blynk.virtualWrite(V6, !valueBomba);  // led informativo de bomba ligada/desligada
}

void setup() {

  Serial.begin(9600);
  sensor.begin();

  Blynk.begin(auth, ssid, pass);
  pinMode(ldr, INPUT);
  pinMode(led, OUTPUT);
  pinMode(bomba, OUTPUT);
  digitalWrite(bomba, 1);
  digitalWrite(led, 1);
  timer.setInterval(1000, sendSensor);
  timer.setInterval(1000, lerSensores);
  timer.setInterval(500, buttonLedWidget);
  timer.setInterval(500, humidityAct);
  timer.setInterval(1000, lightAct);
  pinMode(ena, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  analogWrite(ena, LOW);
  analogWrite(in1, LOW);
  analogWrite(in2, LOW);
}

void loop() {
  Blynk.run();
  timer.run();
}
