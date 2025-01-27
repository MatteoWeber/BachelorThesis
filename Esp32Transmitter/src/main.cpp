#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

uint8_t receiverMAC[] = {0x08, 0xd1, 0xf9, 0xcf, 0xa7, 0x74};

Adafruit_MPU6050 mpu;

const int LED_PIN = 5;
const int LED_COUNT = 6;
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

const float maxVoltage = 4.2;
const float sensitiviy = 4.2 / 1023;

// Calibration values for the cubes gyro replace further down (248-250)
const float gyroXOffsetCube1 = -0.0460;
const float gyroYOffsetCube1 = 0.0099;
const float gyroZOffsetCube1 = -0.0121;

const float gyroXOffsetCube2 = -0.0368;
const float gyroYOffsetCube2 = -0.0236;
const float gyroZOffsetCube2 = -0.0124;

const float gyroXOffsetCube3 = -0.1391;
const float gyroYOffsetCube3 = -0.0305;
const float gyroZOffsetCube3 = -0.0268;

const float gyroXOffsetCube4 = -0.3877;
const float gyroYOffsetCube4 = -0.0796;
const float gyroZOffsetCube4 = -0.2421;

// Structure to hold sensor data for transmission
typedef struct
{
  float accelX;
  float accelY;
  float accelZ;
  float gyroX;
  float gyroY;
  float gyroZ;
  float battery;
  String topFace;
} SendData;
SendData sendData;

typedef struct
{
  int color[3];
} ReceivedData;
ReceivedData receivedData;

esp_now_peer_info_t peerInfo;

String currentTopFace = "Unknown";
String previousTopFace = "Startup";
String previousTopFaceDebugSave = "Startup";

// Sets the color
void color(uint32_t color)
{
  for (int i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, color);
    strip.show();
  }
}

// Debug function to check if Message was send correctly (delivery failed if not connected to esp32 reciever)
void onSent(const uint8_t *mac_addr, esp_now_send_status_t sendStatus)
{
  Serial.print("Send Status: ");
  Serial.println(sendStatus == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback function to handle received ESP-NOW data
void onReceiveData(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
  if (data_len == sizeof(ReceivedData))
  {
    memcpy(&receivedData, data, sizeof(receivedData));

    // DEBUG
    int colorValueRed = receivedData.color[0];
    int colorValueGreen = receivedData.color[1];
    int colorValueBlue = receivedData.color[2];

    Serial.println("RGB Color ");
    Serial.print(colorValueRed);
    Serial.print(", ");
    Serial.print(colorValueGreen);
    Serial.print(", ");
    Serial.print(colorValueBlue);

    color(strip.Color(colorValueRed, colorValueGreen, colorValueBlue));
  }
}

void setup(void)
{
  Serial.begin(115200);

  Wire.begin(17, 16); // SDA = GPIO17, SCL = GPIO16

  // Try to initialize!
  if (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip");
    while (1)
    {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  strip.begin();
  strip.show();
  strip.setBrightness(50);
  strip.Color(0, 255, 0);
  delay(500);

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange())
  {
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
  switch (mpu.getGyroRange())
  {
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

  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth())
  {
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

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != 0)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callback function to receive data
  esp_now_register_recv_cb(esp_now_recv_cb_t(onReceiveData));

  // Register callback for send status
  esp_now_register_send_cb(onSent);

  // Register peer
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}

// Determines the Axis with the highest acceleration which is the axis that is affected by the gravitation in case the cube is placed
void detectTopFace()
{
  if (fabs(sendData.accelZ) > fabs(sendData.accelX) && fabs(sendData.accelZ) > fabs(sendData.accelY))
  {
    currentTopFace = (sendData.accelZ > 0) ? "Top Face: Z+" : "Top Face: Z-";
  }
  else if (fabs(sendData.accelX) > fabs(sendData.accelY))
  {
    currentTopFace = (sendData.accelX > 0) ? "Top Face: X+" : "Top Face: X-";
  }
  else
  {
    currentTopFace = (sendData.accelY > 0) ? "Top Face: Y+" : "Top Face: Y-";
  }
}

// Check if the top face of the cube changed if so return true and update previousTopFace
bool didTheTopFaceChange()
{
  if (currentTopFace != previousTopFace)
  {
    previousTopFaceDebugSave = previousTopFace; // Save previousTopFace to check in Serial Monitor for debbuging
    previousTopFace = currentTopFace;
    return true;
  }
  return false;
}

// Sends the entire sensor data to ESP32 if the top face of the cube changed
void sendDataIfFaceChanged()
{
  if (didTheTopFaceChange())
  {
    sendData.topFace = currentTopFace;
    esp_now_send(receiverMAC, (uint8_t *)&sendData, sizeof(sendData));
    Serial.print("Top Face: " + currentTopFace + "\n");
  }
}

void loop()
{
  // Read data from the GY-521 sensor
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);
  float battery = analogReadMilliVolts(36); // GPIO36 (SVN)(Analog to Digital Pin)
  Serial.print("Raw adc: ");
  Serial.println(int(battery));

  // Fill the Data structure with values
  sendData.accelX = accel.acceleration.x;
  sendData.accelY = accel.acceleration.y;
  sendData.accelZ = accel.acceleration.z;
  sendData.gyroX = gyro.gyro.x + gyroXOffsetCube4; // Change for the correct cube
  sendData.gyroY = gyro.gyro.y + gyroYOffsetCube4;
  sendData.gyroZ = gyro.gyro.z + gyroZOffsetCube4;
  sendData.battery = battery;

  // DEBUG
  Serial.print("Acceleration X: ");
  Serial.print(sendData.accelX);
  Serial.print(", Y: ");
  Serial.print(sendData.accelY);
  Serial.print(", Z: ");
  Serial.print(sendData.accelZ);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(sendData.gyroX);
  Serial.print(", Y: ");
  Serial.print(sendData.gyroY);
  Serial.print(", Z: ");
  Serial.print(sendData.gyroZ);
  Serial.println(" rad/s");

  Serial.println(sendData.battery);

  detectTopFace();
  sendDataIfFaceChanged();

  delay(500); // Delay in Milliseconds
}