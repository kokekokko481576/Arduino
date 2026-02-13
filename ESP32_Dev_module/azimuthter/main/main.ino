#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <Adafruit_NeoPixel.h>

/*---------LEDの設定-----------*/
#define PIN        33 
#define NUMPIXELS  3 // LEDの数

// NeoPixelの設定
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
/*---------LEDの設定-----------*/


/*---------ESP-nowの設定-----------*/
typedef struct struct_message {
  bool ledState;
} struct_message;

struct_message myData;

// 受信時のコールバック
void OnDataRecv(const esp_now_recv_info_t * recvInfo, const uint8_t * incomingData, int len){
  memcpy(&myData, incomingData, sizeof(myData));
  
  // デバッグ用ログ
  if (myData.ledState){
    Serial.println(">>>received: ON (White Mode)");
  } else {
    Serial.println(">>>received: OFF (waiting)");
  }
}
/*---------ESP-nowの設定-----------*/


void setup() {
  Serial.begin(115200);// シリアル設定

  /*-------LEDの初期化----------*/
  pixels.begin(); 
  pixels.clear(); 
  pixels.setBrightness(255); // 明るさ最大
  pixels.show();
  /*-------LEDの初期化----------*/

  /*---------ESP-nowの初期化-----------*/
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(9, WIFI_SECOND_CHAN_NONE); // 9ch固定

  if (esp_now_init() != ESP_OK){
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("Receiver Ready!");
  /*---------ESP-nowの初期化-----------*/
}

void loop() {
  // 1. ボタンが押されている時（ledState == true）は白く光らせて、ここで処理を終える
  if (myData.ledState) {
    for(int k = 0; k < NUMPIXELS; k++){
      pixels.setPixelColor(k, pixels.Color(255, 255, 255)); // 全白
    }
    pixels.show();
    delay(50); // 少し待機
    pixels.clear();
    return;    // loopの先頭に戻る
  }

  /*-------レインボー処理（ボタンが押されていない時だけ実行）----------*/
  
  // パターン1: 赤→青
  for (int i=0; i<255; i++){
    if (myData.ledState) return; // ★もし途中でボタンが押されたら即中断！
    
    for (int j=0; j<NUMPIXELS; j++){
      pixels.setPixelColor(j, pixels.Color(i, 0, 255-i));
    }
    pixels.show();
    delay(5);
  }

  // パターン2: 青→緑
  for (int i=0; i<255; i++){
    if (myData.ledState) return; // ★即中断チェック
    
    for (int j=0; j< NUMPIXELS ;j++){
      pixels.setPixelColor(j, pixels.Color(255-i, i, 0));    
    }
    pixels.show();
    delay(5);
  }

  // パターン3: 緑→赤
  for (int i=0; i<255; i++){
    if (myData.ledState) return; // ★即中断チェック
    
    for (int j=0; j<NUMPIXELS; j++){
      pixels.setPixelColor(j, pixels.Color(0, 255-i, i));      
    }
    pixels.show();
    delay(5);
  }
}