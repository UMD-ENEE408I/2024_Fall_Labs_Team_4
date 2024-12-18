#include <Arduino.h>
#include <Adafruit_MCP3008.h>
#include <Encoder.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>

struct __attribute__((packed)) Data {
    int16_t robo;    // robot number
    char text[50];   // text
};

// WiFi network credentials
const char* ssid = "iPhone (18)";
const char* password = "lillit12";

// Server IP and port
const char* host = "172.20.10.11";  // Replace with the IP address of server
const uint16_t port = 9500;

// Create a client
WiFiClient client;

int i = 1;
int robo_num = 3; //change to 1,2,3 (2 is ball)
char* state = "forwards";

// IMU
Adafruit_MPU6050 mpu;

const unsigned int ADC_1_CS = 2;
const unsigned int ADC_2_CS = 17;

// Encoders
const unsigned int M1_ENC_A = 39;
const unsigned int M1_ENC_B = 38;
const unsigned int M2_ENC_A = 37;
const unsigned int M2_ENC_B = 36;

// Motors
const unsigned int M1_IN_1 = 13;
const unsigned int M1_IN_2 = 12;
const unsigned int M2_IN_1 = 25;
const unsigned int M2_IN_2 = 14;

const unsigned int M1_IN_1_CHANNEL = 8;
const unsigned int M1_IN_2_CHANNEL = 9;
const unsigned int M2_IN_1_CHANNEL = 10;
const unsigned int M2_IN_2_CHANNEL = 11;

const unsigned int M1_I_SENSE = 35;
const unsigned int M2_I_SENSE = 34;

const unsigned int PWM_MAX = 255;
const int freq = 5000;
const int resolution = 8; // 8-bit resolution -> PWM values go from 0-255

// LED
const int ledChannel = 0;

/*
 *  Movement functions
 */
void M1_forward(int pwm_value) {
  ledcWrite(M1_IN_1_CHANNEL, 0);
  ledcWrite(M1_IN_2_CHANNEL, pwm_value);
  //Serial.println("I got to M1FWD ");

}
void M2_forward(int pwm_value) {
  ledcWrite(M2_IN_1_CHANNEL, 0);
  ledcWrite(M2_IN_2_CHANNEL, pwm_value);
  //Serial.println("I got to M2FWD ");
}

void M1_backward(int pwm_value) {
  ledcWrite(M1_IN_1_CHANNEL, pwm_value);
  ledcWrite(M1_IN_2_CHANNEL, 0);
}
void M2_backward(int pwm_value) {
  ledcWrite(M2_IN_1_CHANNEL, pwm_value);
  ledcWrite(M2_IN_2_CHANNEL, 0);
}

void M1_stop() {
  ledcWrite(M1_IN_1_CHANNEL, 0);
  ledcWrite(M1_IN_2_CHANNEL, 0);
}
void M2_stop() {
  ledcWrite(M2_IN_1_CHANNEL, 0);
  ledcWrite(M2_IN_2_CHANNEL, 0);
}

void handle_messages(int right_wheel, int left_wheel, Data response){
  client.readBytes((char*)&response, sizeof(response));
  if((int)response.robo == robo_num || (int)response.robo == 4){
    if(response.text == "over"){
      while(1){
        // trap in here
        M1_stop();
        M2_stop();
      }
    }

    else if(response.text == "backwards"){
      M1_backward(left_wheel);
      M2_backward(right_wheel);
      state = "backwards";
    }

    else if(response.text == "forwards"){
      M1_forward(left_wheel);
      M2_forward(right_wheel);
      state = "forwards";
    }
  }

  else if(robo_num == 2){
    if(state = "backwards"){
      M1_backward(left_wheel);
      M2_backward(right_wheel);
    }
    else{
      M1_forward(left_wheel);
      M2_forward(right_wheel);
    }
  }
}

void setup() {
  Serial.begin(115200);

  ledcSetup(M1_IN_1_CHANNEL, freq, resolution);
  ledcSetup(M1_IN_2_CHANNEL, freq, resolution);
  ledcSetup(M2_IN_1_CHANNEL, freq, resolution);
  ledcSetup(M2_IN_2_CHANNEL, freq, resolution);

  ledcAttachPin(M1_IN_1, M1_IN_1_CHANNEL);
  ledcAttachPin(M1_IN_2, M1_IN_2_CHANNEL);
  ledcAttachPin(M2_IN_1, M2_IN_1_CHANNEL);
  ledcAttachPin(M2_IN_2, M2_IN_2_CHANNEL);

  pinMode(M1_I_SENSE, INPUT);
  pinMode(M2_I_SENSE, INPUT);

  M1_stop();
  M2_stop();

  // IMU Stop
  
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  delay(100);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  // Connect to the server
  if (client.connect(host, port)) {
    Serial.println("Connected to server!");
  } else {
    Serial.println("Connection to server failed.");
    return;
  }
}

void loop() {
    // Prepare data packet
    Data data;
    data.robo = i;
    strncpy(data.text, "Hello from ESP32!", sizeof(data.text) - 1);
    data.text[sizeof(data.text) - 1] = '\0'; // Ensure null termination
    //Serial.printf("seq %d distance %ld voltage %f text %s\n", data.seq, data.distance, data.voltage, data.text);

    // Check if connected to the server
    if (client.connected()) {
      // Read server's response (if any)
      while (client.available()) {
        Data response;
        //client.readBytes((char*)&response, sizeof(response)); // Read data from the server and unpack it into the response struct
        handle_messages(100, 100, response);
        delay(1000);
        M1_stop();
        M2_stop();
      }

      // Send data to the server
      client.write((uint8_t*)&data, sizeof(data));

      // Increment sequence number for the next packet and add a delay between messages
      data.robo++;
      i += 1;
      delay(5000); // Send data every 5 seconds 
    } else {
      Serial.println("Disconnected from server.");
      // Connect to the server
      if (client.connect(host, port)) {
        Serial.println("Connected to server!");
      } else {
        Serial.println("Connection to server failed.");
        return;
      }
    }
}