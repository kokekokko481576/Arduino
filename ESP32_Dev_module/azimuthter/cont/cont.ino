#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h> 

// 受信機のMACアドレス(0xをつけてカンマ区切り)
uint8_t broadcastAddress[] = {0x68, 0xFE, 0x71, 0x90, 0x6C, 0x78};

// 送信データ構造
typedef struct struct_message {
  bool ledState;
} struct_message;

struct_message myData;

esp_now_peer_info_t peerInfo;

// BOOTボタンはたいてい GPIO 0
const int BUTTON_PIN = 0;

void setup() {
  // シリアル設定
  Serial.begin(115200);

  // BOOTボタンの設定
  // 内部プルアップはいらない（基板上で既にプルアップされてるこが多いけど念のため）
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  


  /*---------------ESP_nowの設定--------------------*/
  WiFi.mode(WIFI_STA);

  esp_wifi_set_channel(9, WIFI_SECOND_CHAN_NONE);
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // ペアリング登録
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
  Serial.println("Failed to add peer");
  return;
  }
  
  Serial.println("Sender Ready! Press BOOT button.");
  /*---------------ESP_nowの設定--------------------*/

}
  
void loop() {
  // BOOTボタンは押すとLOWになるよ
  bool isPressed = (digitalRead(BUTTON_PIN) == LOW);
  
  myData.ledState = isPressed;
  
  // データ送信！
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*) &myData, sizeof(myData));
  
  if (result == ESP_OK) {
    Serial.println(isPressed ? "Sent: ON (Button Pushed!)" : "Sent: OFF");
  } else {
  Serial.println("Error sending the data");
  }
  
  delay(100); // チャタリング防止＆連打防止
}
