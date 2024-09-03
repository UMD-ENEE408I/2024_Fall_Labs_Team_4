#include <Arduino.h>
#include <Adafruit_MCP3008.h>

Adafruit_MCP3008 adc1;
Adafruit_MCP3008 adc2;

const unsigned int ADC_1_CS = 2;
const unsigned int ADC_2_CS = 17;

int adc1_buf[8];
int adc2_buf[8];

void readADC() {
  for (int i = 0; i < 8; i++) {
    if (adc1.readADC(i) > 700){
      adc1_buf[i] = 0;
    }
    else{
      adc1_buf[i] = 1;
    }

    if (adc2.readADC(i) > 700){
      adc2_buf[i] = 0;
    }
    else{
      adc2_buf[i] = 1;
    }

    //adc1_buf[i] = adc1.readADC(i);
    //adc2_buf[i] = adc2.readADC(i);

    if (i<7) {
      Serial.print(adc1_buf[i]); Serial.print("\t");
    }

    if (i<6) {
      Serial.print(adc2_buf[i]); Serial.print("\t");
    }
  }
}

void setup() {
  // Stop the right motor by setting pin 14 low
  // this pin floats high or is pulled
  // high during the bootloader phase for some reason
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);
  delay(100);

  Serial.begin(115200);

  adc1.begin(ADC_1_CS);  
  adc2.begin(ADC_2_CS);

}

void loop() {

  int t_start = micros();
  readADC();
  int t_end = micros();

  Serial.print("time: \t"); Serial.print(t_end - t_start); Serial.print("\n");
  Serial.println();

  delay(100);

}
