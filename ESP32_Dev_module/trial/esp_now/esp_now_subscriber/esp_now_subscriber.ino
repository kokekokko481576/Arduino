#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

typedef struct struct_message {
  bool ledState;
} struct_message;

struct_message myData;
const int LED_PIN = 2;

void OnDataRecv(const esp_now_recv_info_t * recvInfo, const uint8_t * incomingData, int len){
  memcpy(&myData, incomingData, sizeof(myData));
  
  if (myData.ledState){
    digitalWrite(LED_PIN, HIGH);
    Serial.println(">>>received: ON");
  } else{
    digitalWrite(LED_PIN, LOW);
    Serial.println(">>>received: OFF");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);

  esp_wifi_set_channel(9, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK){
    Serial.println("Error initializing ESP-NOW");
  }

  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Receiver Ready! Waiting for data...");
}

void loop() {
  // put your main code here, to run repeatedly:

}
