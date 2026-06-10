/**
 * ESP32 I2S ループバック v2
 *
 * 修正点：
 *   MCLK衝突エラー "Selected io is already mapped by another signal" の解消
 *   → i2s_set_pin の mck_io_num を I2S_PIN_NO_CHANGE に明示
 *     （Arduino ESP32 core のバージョンによって構造体メンバが異なるため両対応）
 *
 * マイク : INMP441  → I2S0  SCK=14, WS=15, SD=32
 * アンプ : MAX98357A → I2S1  BCLK=26, LRC=25, DIN=22
 */

#include <driver/i2s.h>

// ============================================================
//  定数
// ============================================================
#define SAMPLE_RATE       16000
#define DMA_BUF_COUNT     8
#define DMA_BUF_LEN       256

#define READ_BUF_SAMPLES  DMA_BUF_LEN
#define READ_BUF_BYTES    (READ_BUF_SAMPLES * 4)          // 32bit = 4byte
#define WRITE_BUF_SAMPLES (READ_BUF_SAMPLES * 2)          // モノ→ステレオ
#define WRITE_BUF_BYTES   (WRITE_BUF_SAMPLES * 4)

// ============================================================
//  バッファ
// ============================================================
static int32_t rxBuf[READ_BUF_SAMPLES];
static int32_t txBuf[WRITE_BUF_SAMPLES];

// ============================================================
//  I2S0 セットアップ（マイク RX）
// ============================================================
bool setupMic() {
  Serial.println("[MIC] installing I2S0...");

  i2s_config_t cfg = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate          = SAMPLE_RATE,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = DMA_BUF_COUNT,
    .dma_buf_len          = DMA_BUF_LEN,
    .use_apll             = false,
    .tx_desc_auto_clear   = false,
    .fixed_mclk           = 0
  };

  // ★ mck_io_num を明示的に I2S_PIN_NO_CHANGE
  i2s_pin_config_t pins = {
    .mck_io_num    = I2S_PIN_NO_CHANGE,   // MCLK不使用
    .bck_io_num    = 14,
    .ws_io_num     = 15,
    .data_out_num  = I2S_PIN_NO_CHANGE,
    .data_in_num   = 32
  };

  esp_err_t r1 = i2s_driver_install(I2S_NUM_0, &cfg, 0, NULL);
  Serial.printf("[MIC] i2s_driver_install: %d\n", r1);
  if (r1 != ESP_OK) return false;

  esp_err_t r2 = i2s_set_pin(I2S_NUM_0, &pins);
  Serial.printf("[MIC] i2s_set_pin:        %d\n", r2);
  if (r2 != ESP_OK) return false;

  Serial.println("[MIC] OK");
  return true;
}

// ============================================================
//  I2S1 セットアップ（アンプ TX）
// ============================================================
bool setupAmp() {
  Serial.println("[AMP] installing I2S1...");

  i2s_config_t cfg = {
    .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate          = SAMPLE_RATE,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = DMA_BUF_COUNT,
    .dma_buf_len          = DMA_BUF_LEN,
    .use_apll             = false,
    .tx_desc_auto_clear   = true,
    .fixed_mclk           = 0
  };

  // ★ mck_io_num を明示的に I2S_PIN_NO_CHANGE
  i2s_pin_config_t pins = {
    .mck_io_num    = I2S_PIN_NO_CHANGE,   // MCLK不使用
    .bck_io_num    = 26,
    .ws_io_num     = 25,
    .data_out_num  = 22,
    .data_in_num   = I2S_PIN_NO_CHANGE
  };

  esp_err_t r1 = i2s_driver_install(I2S_NUM_1, &cfg, 0, NULL);
  Serial.printf("[AMP] i2s_driver_install: %d\n", r1);
  if (r1 != ESP_OK) return false;

  esp_err_t r2 = i2s_set_pin(I2S_NUM_1, &pins);
  Serial.printf("[AMP] i2s_set_pin:        %d\n", r2);
  if (r2 != ESP_OK) return false;

  Serial.println("[AMP] OK");
  return true;
}

// ============================================================
//  setup
// ============================================================
bool systemReady = false;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== ESP32 I2S Loopback v2 ===");
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());

  bool micOk = setupMic();
  bool ampOk = setupAmp();

  if (!micOk || !ampOk) {
    Serial.println("\n[FATAL] I2S init failed. Halting.");
    while (true) { delay(1000); }
  }

  delay(200);
  systemReady = true;
  Serial.println("\n[OK] Loopback starting...");
}

// ============================================================
//  loop
// ============================================================
void loop() {
  if (!systemReady) return;

  size_t bytesRead    = 0;
  size_t bytesWritten = 0;

  // 1. マイクから読む
  esp_err_t rErr = i2s_read(
    I2S_NUM_0, rxBuf, READ_BUF_BYTES, &bytesRead, portMAX_DELAY
  );
  if (rErr != ESP_OK || bytesRead != READ_BUF_BYTES) {
    Serial.printf("[ERR] read: err=%d bytes=%d\n", rErr, (int)bytesRead);
    return;
  }

  // 2. モノ → ステレオ展開
  size_t samples = bytesRead / 4;
  for (int i = (int)samples - 1; i >= 0; i--) {
    int32_t s        = rxBuf[i];
    txBuf[i * 2 + 0] = s;
    txBuf[i * 2 + 1] = s;
  }

  // 3. アンプへ書く
  size_t writeBytes = samples * 2 * 4;
  i2s_write(I2S_NUM_1, txBuf, writeBytes, &bytesWritten, portMAX_DELAY);
}