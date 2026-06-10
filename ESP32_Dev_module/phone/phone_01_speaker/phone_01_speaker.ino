#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>

#define BUTTON_PIN   0

// 【I2S1】アンプ（MAX98357A）のピン設定
#define I2S_SPK_LRC  25
#define I2S_SPK_BCLK 26
#define I2S_SPK_DIN  22

#define SAMPLE_RATE     16000
#define WAVE_FREQUENCY  1000  // 1kHzのテスト音
#define BUFFER_SIZE     128

// MAX98357Aが正常にデコードできるように32bit(int32_t)でバッファを作成
int32_t toneBuffer[BUFFER_SIZE];

void initI2SSpeaker32Bit() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX), // 送信モード
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,       // ★ここを32bitに固定！
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,       // 左右（ステレオ）形式
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,  // 標準I2S
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

void setup() {
  Serial.begin(115200);
  delay(500);

  // BOOTボタンのピン設定
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // 32bitスピーカーの初期化
  initI2SSpeaker32Bit();

  // 32bitのフルスケール（最大値約21億）に対して、
  // アンプが歪まないようにちょっと控えめな振幅（約1000万）でサイン波を生成
  float amplitude = 10000000.0; 
  for (int i = 0; i < BUFFER_SIZE; i++) {
    toneBuffer[i] = (int32_t)(amplitude * sin(2.0 * M_PI * WAVE_FREQUENCY * i / SAMPLE_RATE));
  }

  Serial.println("32bit Speaker Standalone Test Ready!");
  Serial.println("Press and hold the BOOT button to play 1kHz Tone.");
}

void loop() {
  size_t bytesWritten = 0;

  // BOOTボタンが押されている（LOW）の間だけアンプに32bitデータを送りつける
  if (digitalRead(BUTTON_PIN) == LOW) {
    i2s_write(I2S_NUM_1, &toneBuffer, sizeof(toneBuffer), &bytesWritten, portMAX_DELAY);
  } else {
    // ボタンを離している間は、無駄な処理をさせずに10ミリ秒待機
    delay(10);
  }
}