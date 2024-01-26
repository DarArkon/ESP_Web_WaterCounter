#include <SPI.h>                      //бібілотека для роботи з інтерефейсом SPI
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>         //бібіліотеки роботи з дисплеєм
#include <ESP8266WiFi.h>
#include <WiFiClient.h>               //бібіліотекки для роботи з WiFi
#include <ESP8266WebServer.h>         //бібіліотека для роботи з веб-сервером
#include <ArduinoJson.h>              //бібліотека для роботи з JSON
#include "index.h"                    //веб-сторінка

Adafruit_PCD8544 display = Adafruit_PCD8544(2, 0, 4, 5, 16);    //піни підключення дисплею

const char* ssid = "TP-LINK_606A";    //назва мережі
const char* password = "30117402";    //пароль від мережі

ESP8266WebServer server(80);

String receivedAdr = "";            //Рядок для зберігання отриманих даних адреси
String receivedNum = "";            //Рядок для зберігання отриманих даних номеру
unsigned int time_counter = 0;      //лічильник часу
double water_counter = 0.00;        //лічильник витрати
float water_flow = 0.00;            //витрата води
unsigned long UpdateTime1 = 0;      //лічильник для таймера міліс
unsigned long UpdateTime2 = 0;      //лічильник для таймера міліс
unsigned long UpdateTime3 = 0;      //лічильник для таймера міліс
int sensor_value;
float linit_ADC;

String apiKeyValue = "*******************";   //унікалькний код давачів
const char* httpsSite = "https://********";   //сорінка куди відправляєм дані
const uint8_t fingerprint[20] = {********};   //hss ключ, з сертифікату сайту



void setup() {
  Serial.begin(115200);   //Ініціалізація UART

  display.begin();        //Ініціалізація дисплею
  display.setContrast(60);
  //delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0, 0);
  display.println("Connecting....");
  display.display();

  WiFi.begin(ssid, password);   //Ініціалізація WiFi

  while (WiFi.status() != WL_CONNECTED) {   //підключення до WiFi за вказанимим даними
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connected to:");
  display.println(ssid);
  display.println("IP address:");
  display.println(WiFi.localIP());
  display.display();
  delay(2000);

  //Формування запитів
  server.on("/", handleRoot);               // домашня сторінка
  server.on("/update", handleUpdate);       // оновлення даних
  server.on("/editUser", editUser);         // оновлення даних про користувача
  server.on("/submit", handleSubmit);       // оновлення даних про користувача

  server.begin();     //Ініціалізація Веб-сервера
}

void loop() {
  server.handleClient();

  if (millis() - UpdateTime1 >= 200) { //таймер оновлення даних і екрану
    UpdateTime1 = millis();
    sensor_value = analogRead(A0);
    linit_ADC = (sensor_value - 51) * 0.01;
    water_flow = ((24 * linit_ADC) / 9.72);
    water_counter = water_counter + (water_flow / 5);
    w_flow_display();
  }
  if (millis() - UpdateTime2 >= 1000) { //часовий таймер
    UpdateTime2 = millis();
    time_counter++;
  }
  if (millis() - UpdateTime3 >= 10000) { //часовий відправки даних
    UpdateTime3 = millis();
    String Data = "api_key=" + apiKeyValue + "&adres=" + receivedAdr +
                  "&number=" + receivedNum + "&value1=" + water_flow +
                  "&value2=" + water_counter + "&value3=" + time_counter;
    httpGET(Data);//відправляєм на сервер
  }
}
//=== Функція, яка обробляє головну сторінку ===========================
void handleRoot() {
  String s = webpage;
  server.send(200, "text/html", s);
}
//=== Функція, яка обробляє AJAX-запит на оновлення даних ===================
void handleUpdate() {
  String response = "{\"variable1\":" + String(time_counter) + ",";
  response += "\"variable2\":" + String(round(water_counter)) + ",";
  response += "\"variable3\":" + String(water_flow) + "}";

  server.send(200, "application/json", response);
}
//=== Функція, яка обробляє сторінку змыни даних про користувача =================
void editUser() {
  String s2 = webpage2;
  server.send(200, "text/html", s2);
}
void handleSubmit() {
  receivedAdr = server.arg("adr");
  receivedNum = server.arg("num");
  //Serial.println("Received Adres: " + receivedAdr);
  //Serial.println("Received Number: " + receivedNum);

  server.send(200, "text/html", "<meta http-equiv='refresh' content='0; url=/' />");
}
//=== Функція, яка виводить дані на дисплей =====================
void w_flow_display() {
  int water_counter_ = round(water_counter);
  int a_ = water_counter_ / 1000;
  int b_ = water_counter_ % 1000;
  int flow_a = length_(a_);
  int flow_b = length_(b_);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Counter");
  display.setTextSize(2);
  switch (flow_a) {
    case 1: display.print("0000"); break;
    case 2: display.print("000"); break;
    case 3: display.print("00"); break;
    case 4: display.print("0"); break;
  }
  display.print(a_);
  display.setTextSize(1);
  display.print(".");
  switch (flow_b) {
    case 1: display.print("00"); break;
    case 2: display.print("0"); break;
  }
  display.println(b_);
  display.println("           m^3");
  display.println("Flowmeter");
  display.print(water_flow);
  display.print(" l/s");
  display.display();
}
int length_(int n) {
  int l = 1;
  for (; n /= 10; ++l);
  return l;
}
//=== Функція, яка відправляє дані на сервер =====================

String httpGET(String httpsRequestData) {
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    bool mfln = client->probeMaxFragmentLength(httpsSite, 443, 1024);
    if (mfln) {
      client->setBufferSizes(1024, 1024);
    }
    client->setFingerprint(fingerprint);
    HTTPClient https;
    if (https.begin(*client, httpsSite)) {
      int httpCode = https.GET();
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
          https.addHeader("Content-Type", "application/x-www-form-urlencoded");
          int httpsResponseCode = https.POST(httpsRequestData);
          https.end();
          httpsRequestData.clear();
        }
      }
    }
  }
  return httpsRequestData;
}
