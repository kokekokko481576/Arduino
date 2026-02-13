#include "driver/twai.h"

// ピン定義
#define RX_PIN 25
#define TX_PIN 26

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("=== ESP32 ひとりぼっち診断 (Loopback) ===");

  // ★重要★ TWAI_MODE_NO_ACK ではなく TWAI_MODE_LOOPBACK にする
  // これにすると、外部の配線を無視して、チップ内部で信号を折り返すよ
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)TX_PIN, (gpio_num_t)RX_PIN, TWAI_MODE_NO_ACK);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("ドライバ: OK");
  } else {
    Serial.println("ドライバ: 失敗！");
    return;
  }

  if (twai_start() == ESP_OK) {
    Serial.println("TWAI開始: OK");
  } else {
    Serial.println("TWAI開始: 失敗！");
    return;
  }
}

void loop() {
  // 1. 自分で自分に送る
  twai_message_t tx_message;
  tx_message.identifier = 0x555;
  tx_message.flags = TWAI_MSG_FLAG_NONE;
  tx_message.data_length_code = 8;
  for (int i = 0; i < 8; i++) tx_message.data[i] = i;

  if (twai_transmit(&tx_message, pdMS_TO_TICKS(100)) == ESP_OK) {
    Serial.print("送信: OK -> ");
  } else {
    Serial.println("送信: 失敗...");
  }

  // 2. 自分で受け取る
  twai_message_t rx_message;
  if (twai_receive(&rx_message, pdMS_TO_TICKS(100)) == ESP_OK) {
    Serial.print("受信成功！ ID: 0x");
    Serial.println(rx_message.identifier, HEX);
  } else {
    Serial.println("受信: タイムアウト（届いてない）");
  }

  delay(1000);
}