#include <Adafruit_NeoPixel.h>

// ここを接続してるピン番号に変えてね！
#define PIN        33 
// LEDの数
#define NUMPIXELS  300

// NeoPixelの設定（大体このままで動くはず！）
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin(); // 初期化
  pixels.clear(); // いったん全部消す
  pixels.setBrightness(50);
}

void loop() {

  for (int i=0;i<255;i++){
    for (int j=0; j<NUMPIXELS;j++){
      pixels.setPixelColor(j, pixels.Color(i, 0, 255-i));
        
    }
    pixels.show();
    delay(10);
    pixels.clear();
  }
  for (int i=0;i<255;i++){
    for (int j=0; j< NUMPIXELS ;j++){
      pixels.setPixelColor(j, pixels.Color(255-i, i, 0));    
    }
    pixels.show();
    delay(10);
    pixels.clear();
  }
  for (int i=0;i<255;i++){
    for (int j=0; j<NUMPIXELS;j++){
      pixels.setPixelColor(j, pixels.Color(0, 255-i, i));      
    }
    pixels.show();
    delay(10);
    pixels.clear();
  }
}