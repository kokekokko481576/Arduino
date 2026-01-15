const int PWM_PIN = 12;
const int FREQ = 5000;
const int RES = 8;

void setup() {
  // put your setup code here, to run once:

  ledcAttach(PWM_PIN, FREQ, RES);

  Serial.begin(115200);
  Serial.println("Hello PWM");
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0 ; i <= 500; i++){
    ledcWriteTone(PWM_PIN,i*10);
    Serial.println(i);
    delay(10);
  }
  for (int i = 500; i >= 0; i--){
    ledcWriteTone(PWM_PIN,i*10);
    Serial.println(i);
    delay(10);
  }

}
