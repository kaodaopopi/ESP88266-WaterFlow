#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

volatile int  flow_frequency;  // Measures flow meter pulses
unsigned int  l_hour;          // Calculated litres/hour
unsigned char flowmeter = 4;  // Flow Meter Pin number(D2)
unsigned long currentTime;
unsigned long cloopTime;

// 設定無線基地台SSID跟密碼
const char* ssid     = "WiFi名稱";     
const char* password = "WiFi密碼";     
WiFiClient wifiClient;  //wifi設定

void IRAM_ATTR flow()                  // Interruot function
{
  flow_frequency++;
}

void setup() {
  Serial.begin(115200);  // 設定速率 感測器

  // 連接無線基地台
  WiFi.begin(ssid, password);

  // 等待連線，並從 Console顯示 IP
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting.....");
  }

  pinMode(flowmeter, INPUT);
  attachInterrupt(digitalPinToInterrupt(flowmeter), flow, RISING); // Setup Interrupt
  sei();                            // Enable interrupts
  currentTime = millis();
  cloopTime = currentTime;
}

void loop() {
  currentTime = millis();
  if (currentTime >= (cloopTime + 1000))
  {
    cloopTime = currentTime;              // Updates cloopTime
                                          // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min. (Results in +/- 3% range)
    l_hour = (flow_frequency * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flow rate in L/hour
    flow_frequency = 0;                   // Reset Counter
    Serial.print(l_hour, DEC);            // Print litres/hour
    Serial.println(" L/hour");

    if (l_hour != 0) {
      //執行API Request
      WiFiClient client;
      const uint16_t port = 80;
      const char * host = "IP位置";
      client.connect(host, port);

      // 檢查連線是否成功
      if (!client.connect(host, port)) {
        Serial.println("connection failed!");
        return;
      } else {
        Serial.println("Success");
        Serial.print(l_hour, DEC);            // Print litres/hour
        Serial.println(" L/hour");

        String postStr = "";
        postStr += "WaterFlowCode=";
        postStr += String("1");
        postStr += "&WaterFlowValues=";
        postStr += String(l_hour);

        String url = "/api/ESP8266WaterFlow?" + postStr;
        client.print(String("GET ")  + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection:close\r\n\r\n");

        //查看回傳值
        while (client.available()) {
          String line = client.readStringUntil('\r');
          Serial.print(line);
        }
      }
      Serial.println("連線關閉");
      client.stop();
    }
  }
}
