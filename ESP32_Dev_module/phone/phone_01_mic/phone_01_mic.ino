#include <Arduino.h>
#include <driver/i2s.h>

// I2S0のピン設定（マイク用）
#define I2S_MIC_WS   15
#define I2S_MIC_SCK  14
#define I2S_MIC_SD   32

// バッファサイズ設定
#define BUFFER_SIZE 128
int32_t micBuffer[BUFFER_SIZE];

void initI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX), // 受信モード
    .sample_rate = 16000,                               // 16kHz
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,       // INMP441は32bit
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,        // 左チャンネル固定
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,  // 標準I2S
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,                                 // テスト用なのでバッファ数は少なめでレスポンス重視
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

void setup() {
  // シリアル通信を爆速（115200bps）で開始
  Serial.begin(115200);
  
  // I2Sマイク初期化
  initI2S();
  delay(500);
}

void loop() {
  size_t bytesRead = 0;
  
  // マイクからデータを取得（128サンプル分）
  i2s_read(I2S_NUM_0, &micBuffer, sizeof(micBuffer), &bytesRead, portMAX_DELAY);
  
  int samplesRead = bytesRead / 4;
  int16_t maxVolume = 0;
  
  for (int i = 0; i < samplesRead; i++) {
    int32_t sample = micBuffer[i] >> 14;
    int16_t volume = abs((int16_t)(sample)); // 絶対値にしてプラスの音量にする
    
    // このバッファ内での最大音量をみつける
    if (volume > maxVolume) {
      maxVolume = volume;
    }
  }
  
  // ★ 128サンプルのうちの「一番デカかった音量」だけを1行出力！
  // これでデータの流れる速度が128分の1になってめちゃくちゃ見やすくなるよ
  Serial.println(maxVolume);
  
  // プロッタが早すぎる場合は、ここにちょっとだけディレイを入れてもOK
  // delay(5); 
}