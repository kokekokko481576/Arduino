// 多くのESP32ボードでオンボードLEDは 2番ピン だよ
// もし光らなかったら、ここを LED_BUILTIN に書き換えてみてね
const int LED_PIN = 2;

void setup() {
  // ピンを出力モードに設定
  pinMode(LED_PIN, OUTPUT);
  
  // シリアルモニタも使えるようにしておくと便利（速度は115200が一般的）
  Serial.begin(115200);
  Serial.println("Hello ESP32!");
  }

void loop() {
  digitalWrite(LED_PIN, HIGH); // LEDをつける（3.3Vを出力）
  Serial.println("LED ON");
  delay(1000);                 // 1秒待つ

  digitalWrite(LED_PIN, LOW);  // LEDを消す（0Vを出力）
  Serial.println("LED OFF");
  delay(1000);                 // 1秒待つ
  }
