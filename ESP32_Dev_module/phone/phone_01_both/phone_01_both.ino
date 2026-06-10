#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <driver/i2s.h>

// ★全員宛て（ブロードキャスト）のMACアドレスに設定！
uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 

// 【I2S0】マイク（INMP441）のピン設定
#define I2S_MIC_WS   15
#define I2S_MIC_SCK  14
#define I2S_MIC_SD   32

// 【I2S1】アンプ（MAX98357A）のピン設定
#define I2S_SPK_LRC  25
#define I2S_SPK_BCLK 26
#define I2S_SPK_DIN  22

// 音声バッファの設定
#define SAMPLE_RATE 16000
#define BUFFER_SIZE 256
int32_t micBuffer[BUFFER_SIZE];

// ESP-NOW 送信用バッファ
#define SEND_SAMPLES 100
int16_t sendBuffer[SEND_SAMPLES];

// 【初期化】マイク用 I2S0 (RX)
void initI2SMic() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 64,
    .use_apll = false
  };
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_MIC_SCK,
    .ws_io_num = I2S_MIC_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SD
  };
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

// 【初期化】スピーカーアンプ用 I2S1 (TX)
void initI2SSpeaker() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 64,
    .use_apll = false
  };
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SPK_BCLK,
    .ws_io_num = I2S_SPK_LRC,
    .data_out_num = I2S_SPK_DIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  i2s_driver_install(I2S_NUM_1, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_1, &pin_config);
}

// ★ ESP-NOW パケット受信時のコールバック
void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  size_t bytesWritten;
  // 受信した音声データをスピーカー（I2S1）へ書き込んで鳴らす
  i2s_write(I2S_NUM_1, incomingData, len, &bytesWritten, portMAX_DELAY);
}

void setup() {
  Serial.begin(115200);
  delay(500); // 起動直後の安定待ち

  // 1. Wi-FiをSTAモードに設定
  WiFi.mode(WIFI_STA);
  
  // 2. ★超重要：ダミーでアクセスポイントに接続を試みるか、
  // 内部インターフェースを強制起動して「チャンネル1」を確定させる
  WiFi.disconnect(true); // 一度完全に初期化
  delay(100);
  
  // 3. ESP-NOWの初期化
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // 4. 受信コールバック関数を登録
  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));

  // 5. ★ピアの登録設定（チャンネルを明示的に 1 に指定する）
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo)); // メモリを綺麗にゼロクリアしておく（超大事！）
  memcpy(peerInfo.peer_addr, broadcastMac, 6);
  peerInfo.channel = 1;     // ➔ 0じゃなくて「1」を明示的に指定！
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA; // ➔ STAインターフェースを使うことを明示！

  // 6. ピアの追加
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // ハードウェアの初期化
  initI2SMic();
  initI2SSpeaker();

  Serial.println("Broadcast Voice System Ready!");
}

void loop() {
  size_t bytesRead = 0;
  
  // 1. マイクから音声を吸い上げる
  i2s_read(I2S_NUM_0, &micBuffer, sizeof(micBuffer), &bytesRead, portMAX_DELAY);
  
  int samplesRead = bytesRead / 4;
  int sendIdx = 0;
  
  for (int i = 0; i < samplesRead; i++) {
    int32_t sample = micBuffer[i] >> 14;
    sendBuffer[sendIdx] = (int16_t)(sample);
    sendIdx++;
    
    // 2. 100サンプル溜まったらブロードキャストアドレスへ送信！
    if (sendIdx >= SEND_SAMPLES) {
      esp_now_send(broadcastMac, (uint8_t *)sendBuffer, sizeof(sendBuffer));
      sendIdx = 0;
    }
  }
}