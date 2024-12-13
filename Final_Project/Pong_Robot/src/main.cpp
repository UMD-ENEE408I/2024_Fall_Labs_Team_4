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
const char* host = "172.20.10.8";  // Replace with the IP address of server
const uint16_t port = 9500;

// Create a client
WiFiClient client;

int i = 1;
int robo_num = 1; //change to 1,2,3 (2 is ball)

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

// PID
float e;
float p_e;
float d_e;
float total_e;

// Assign values to the following feedback constants:
float Kp = 4;
float Kd = 0;
float Ki = 0;

const float time_to_turn_180 = 260.0;
int start = 0;
int in_pos = 0;

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

void turn(bool clockwise, int right_wheel, int left_wheel, float angle) {
  int delay_time = (int)((angle/180.0) * time_to_turn_180);
  
  if (clockwise) {
    M1_forward(left_wheel);
    M2_backward(right_wheel);
  } 
  
  else {
    M1_backward(left_wheel);
    M2_forward(right_wheel);
  }

  delay(delay_time);

  // Stop the robot
  M1_stop();
  M2_stop();

  delay(1000);
}

void handle_messages(int right_wheel, int left_wheel){
  Data response;
  client.readBytes((char*)&response, sizeof(response));

  if(response.text == "Game Over"){
    while(1){
      // trap in here
      M1_stop();
      M2_stop();
    }
  }

  else if(response.text == "backwards"){
    M1_backward(left_wheel);
    M2_backward(right_wheel);
  }

  else if(response.text == "forwards"){
    M1_forward(left_wheel);
    M2_forward(right_wheel);
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

void loop(){
  int u;
  int rightWheelPWM;
  int leftWheelPWM;

  int pidRight;
  int pidLeft;
  float pos;

  // Define the PID errors
  float DT = .5;
  e = 6 - pos;
  d_e = (e - p_e) / DT;
  total_e += e*DT;

  // Update the previous error
  p_e = e;

  // Implement PID control (include safeguards for when the PWM values go below 0 or exceed maximum)
  u = Kp * e + 0 * d_e + Ki * total_e; //need to integrate e

  // Implement PID control (include safeguards for when the PWM values go below 0 or exceed maximum)
  pidRight = 150 - u;
  pidLeft = 136 + u;

  // Constrain the PWM values
  if (pidRight < 0) {
    pidRight = 0;
  } else if (pidRight > PWM_MAX) {
    pidRight = PWM_MAX;
  }

  if (pidLeft < 0) {
    pidLeft = 0;
  } else if (pidLeft > PWM_MAX) {
    pidLeft = PWM_MAX;
  }

  while(client.available() && in_pos == 0){
    Data response;
    client.readBytes((char*)&response, sizeof(response));

    if(response.text == "2"){
      turn(1, rightWheelPWM, leftWheelPWM, 90.0);
      
      M1_forward(pidLeft);
      M2_forward(pidRight);
      delay(1000); //change this

      M1_stop();
      M2_stop();
      delay(1000);

      turn(0, rightWheelPWM, leftWheelPWM, 90.0);
    }

    Data data;
    data.seq = 1;
    data.distance = 1000;
    data.voltage = 3.7f;
    strncpy(data.text, "in position", sizeof(data.text) - 1);
    data.text[sizeof(data.text) - 1] = '\0';

    in_pos = 1;
  }

  while(client.available() && start == 0){
    Data response;
    client.readBytes((char*)&response, sizeof(response));
    
    if(response.text == "Start"){
      start = 1;
    }
    else{
      Serial.println("Waiting to start game...");
    }
  }

  while(client.available()){
    handle_messages(pidRight, pidLeft);
  }
}