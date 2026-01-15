#include "Arduino.h" // WiFi.hの代わりにこっちを使ってみる
#include "esp_mac.h" 

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  uint8_t mac[6];
  // チップ内部のベースMACアドレスを取得
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  
  Serial.println("\n--- MAC Address Read Success ---");
  Serial.printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void loop() {
  // ループでも連呼
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  Serial.printf("Looping MAC:%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2],mac[3], mac[4], mac[5]);
  delay(2000);
}
