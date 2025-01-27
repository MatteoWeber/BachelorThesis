#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include <LittleFS.h>

#pragma region dataprocessing
// Server Settings
const char *ssid = "CubeNetwork";
const char *password = "TestCube1234";

WebServer server(80);

// MAC Address of the ESP32 HUB:
// uint8_t receiverMAC[] = {0x08, 0xd1, 0xf9, 0xcf, 0xa7, 0x74};

uint8_t cubeMacAddresses[][6] = {
    {0xc8, 0x2e, 0x18, 0x22, 0xc8, 0x78},   // Cube 1
    {0xc8, 0x2e, 0x18, 0x22, 0xda, 0xbc},   // Cube 2
    {0xc8, 0x2e, 0x18, 0x24, 0xb9, 0xb0},   // Cube 3
    {0xc8, 0x2e, 0x18, 0x24, 0xc0, 0x84}    // Cube 4

};

// Structure to hold received data
typedef struct
{
  float accelX;
  float accelY;
  float accelZ;
  float gyroX;
  float gyroY;
  float gyroZ;
  float battery;
  String topFace; // Example "Top Face: X+"
} ReceivedData;
ReceivedData dataFromCube1;
ReceivedData dataFromCube2;
ReceivedData dataFromCube3;
ReceivedData dataFromCube4;

// Structure to hold data for Feedback
typedef struct
{
  int color[3]; //[0]Red, [1]Green, [2]Blue
} SendData;
SendData dataForCube1;
SendData dataForCube2;
SendData dataForCube3;
SendData dataForCube4;
SendData dataForAllCubes;

typedef void (*ApplicationFunction)();
ApplicationFunction currentApplication;
typedef struct
{
  String id;
  String name;
  ApplicationFunction function;
} Application;

esp_now_peer_info_t peerInfo;

// Adds the MAC address of the cube
void addCubeAsPeer(uint8_t *macAddress, int cubeIndex)
{
  memcpy(peerInfo.peer_addr, macAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Debug function to check if MAC address of the cube was added correctly
  if (esp_now_add_peer(&peerInfo) == ESP_OK)
  {
    Serial.printf("Cube %d added successfully\n", cubeIndex);
  }
  else
  {
    Serial.printf("Failed to add Cube %d\n", cubeIndex);
  }
}

// Debug function to check if Message was send correctly (delivery failed if not connected to esp32 receiver)
void onSent(const uint8_t *mac_addr, esp_now_send_status_t sendStatus)
{
  Serial.print("Send Status: ");
  Serial.println(sendStatus == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Function to send data to the cubes
void sendFeedbackData(uint8_t *cubeMacAddress, SendData dataForCubeX)
{
  esp_now_send(cubeMacAddress, (uint8_t *)&dataForCubeX, sizeof(dataForCubeX));
}

// Debug function to print received data to serial monitor
void printReceivedData(ReceivedData dataFromCubeX, int cube)
{
  Serial.printf("Data received from Cube %d\n", cube);
  Serial.print("Accel X: ");
  Serial.print(dataFromCubeX.accelX);
  Serial.print(", Y: ");
  Serial.print(dataFromCubeX.accelY);
  Serial.print(", Z: ");
  Serial.println(dataFromCubeX.accelZ);

  Serial.print("Gyro X: ");
  Serial.print(dataFromCubeX.gyroX);
  Serial.print(", Y: ");
  Serial.print(dataFromCubeX.gyroY);
  Serial.print(", Z: ");
  Serial.println(dataFromCubeX.gyroZ);

  Serial.print(dataFromCubeX.topFace);

  Serial.print("Battery: ");
  Serial.println(dataFromCubeX.battery);
}

// Callback function to handle received ESP-NOW data
void onReceiveData(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
  if (data_len == sizeof(ReceivedData))
  {
    if (memcmp(mac_addr, cubeMacAddresses[0], sizeof(cubeMacAddresses[0])) == 0)
    {
      memcpy(&dataFromCube1, data, sizeof(dataFromCube1));
      // Debug
      printReceivedData(dataFromCube1, 1);
      //sendFeedbackData(cubeMacAddresses[0], dataForCube1);
    }
    else if (memcmp(mac_addr, cubeMacAddresses[1], sizeof(cubeMacAddresses[1])) == 0)
    {
      memcpy(&dataFromCube2, data, sizeof(dataFromCube2));
      // Debug
      printReceivedData(dataFromCube2, 2);
      //sendFeedbackData(cubeMacAddresses[1], dataForCube2);
    }
    else if (memcmp(mac_addr, cubeMacAddresses[2], sizeof(cubeMacAddresses[2])) == 0)
    {
      memcpy(&dataFromCube3, data, sizeof(dataFromCube3));
      // Debug
      printReceivedData(dataFromCube3, 3);
      //sendFeedbackData(cubeMacAddresses[2], dataForCube3);
    }
    else if (memcmp(mac_addr, cubeMacAddresses[3], sizeof(cubeMacAddresses[3])) == 0)
    {
      memcpy(&dataFromCube4, data, sizeof(dataFromCube4));
      // Debug
      printReceivedData(dataFromCube4, 4);
      //sendFeedbackData(cubeMacAddresses[3], dataForCube4);
    }

    if (currentApplication != nullptr)
    {
      currentApplication();
    }
  }
}
#pragma endregion dataprocessing

#pragma region applications

void colorReset()
{
  dataForAllCubes.color[0] = 0;
  dataForAllCubes.color[1] = 0;
  dataForAllCubes.color[2] = 0;
  for (auto &macAddress : cubeMacAddresses)
  {
    sendFeedbackData(macAddress, dataForAllCubes);
  }
}

void colorTestApplication()
{
    dataForAllCubes.color[0] = 0;
    dataForAllCubes.color[1] = 255;
    dataForAllCubes.color[2] = 0;
    for (auto &macAddress : cubeMacAddresses)
    {
      sendFeedbackData(macAddress, dataForAllCubes);
    }

}

void colorTheoryApplication()
{
  String ColorCube1;
  String ColorCube2;

  // Base Color for Cube 1
  if (dataFromCube1.topFace == "Top Face: Y+" || dataFromCube1.topFace == "Top Face: Y-")
  {
    dataForCube1.color[0] = 0;
    dataForCube1.color[1] = 0;
    dataForCube1.color[2] = 255;
    ColorCube1 = "Blue";
    sendFeedbackData(cubeMacAddresses[0], dataForCube1);
  }
  else if (dataFromCube1.topFace == "Top Face: X+" || dataFromCube1.topFace == "Top Face: X-")
  {
    dataForCube1.color[0] = 255;
    dataForCube1.color[1] = 255;
    dataForCube1.color[2] = 0;
    ColorCube1 = "Yellow";
    sendFeedbackData(cubeMacAddresses[0], dataForCube1);
  }
  else if (dataFromCube1.topFace == "Top Face: Z+" || dataFromCube1.topFace == "Top Face: Z-")
  {
    dataForCube1.color[0] = 255;
    dataForCube1.color[1] = 0;
    dataForCube1.color[2] = 0;
    ColorCube1 = "Red";
    sendFeedbackData(cubeMacAddresses[0], dataForCube1);
  }

  // Base color for Cube 2
  if (dataFromCube2.topFace == "Top Face: Y+" || dataFromCube2.topFace == "Top Face: Y-")
  {
    dataForCube2.color[0] = 0;
    dataForCube2.color[1] = 0;
    dataForCube2.color[2] = 255;
    ColorCube2 = "Blue";
    sendFeedbackData(cubeMacAddresses[1], dataForCube2);
  }
  else if (dataFromCube2.topFace == "Top Face: X+" || dataFromCube2.topFace == "Top Face: X-")
  {
    dataForCube2.color[0] = 255;
    dataForCube2.color[1] = 255;
    dataForCube2.color[2] = 0;
    ColorCube2 = "Yellow";
    sendFeedbackData(cubeMacAddresses[1], dataForCube2);
  }
  else if (dataFromCube2.topFace == "Top Face: Z+" || dataFromCube2.topFace == "Top Face: Z-")
  {
    dataForCube2.color[0] = 255;
    dataForCube2.color[1] = 0;
    dataForCube2.color[2] = 0;
    ColorCube2 = "Red";
    sendFeedbackData(cubeMacAddresses[1], dataForCube2);
  }

  // Data for Cube 3 based on color Combination
  if (ColorCube1 == "Blue" && ColorCube2 == "Blue")
  {
    dataForCube3.color[0] = 0;
    dataForCube3.color[1] = 0;
    dataForCube3.color[2] = 255;
    sendFeedbackData(cubeMacAddresses[2], dataForCube3);
  }
  else if (ColorCube1 == "Yellow" && ColorCube2 == "Yellow")
  {
    dataForCube3.color[0] = 255;
    dataForCube3.color[1] = 255;
    dataForCube3.color[2] = 0;
    sendFeedbackData(cubeMacAddresses[2], dataForCube3);
  }
  else if (ColorCube1 == "Red" && ColorCube2 == "Red")
  {
    dataForCube3.color[0] = 255;
    dataForCube3.color[1] = 0;
    dataForCube3.color[2] = 0;
    sendFeedbackData(cubeMacAddresses[2], dataForCube3);
  }
  else if (ColorCube1 == "Blue" && ColorCube2 == "Yellow" || ColorCube1 == "Yellow" && ColorCube2 == "Blue") // Green
  {
    dataForCube3.color[0] = 0;
    dataForCube3.color[1] = 255;
    dataForCube3.color[2] = 0;
    sendFeedbackData(cubeMacAddresses[2], dataForCube3);
  }
  else if (ColorCube1 == "Blue" && ColorCube2 == "Red" || ColorCube1 == "Red" && ColorCube2 == "Blue") // Purple
  {
    dataForCube3.color[0] = 127;
    dataForCube3.color[1] = 0;
    dataForCube3.color[2] = 255;
    sendFeedbackData(cubeMacAddresses[2], dataForCube3);
  }
  else if (ColorCube1 == "Yellow" && ColorCube2 == "Red" || ColorCube1 == "Red" && ColorCube2 == "Yellow") // Orange
  {
    dataForCube3.color[0] = 255;
    dataForCube3.color[1] = 64;
    dataForCube3.color[2] = 0;
    sendFeedbackData(cubeMacAddresses[2], dataForCube3);
  }
}

void basicMathApplication() {
  int NumberCube1;
  int NumberCube2;
  int NumberCube3;

  // Save the number of the top face of Cube 1
  if (dataFromCube1.topFace == "Top Face: Y+" || dataFromCube1.topFace == "Top Face: Y-")
  {
    NumberCube1 = 1;
  }
  else if (dataFromCube1.topFace == "Top Face: X+" || dataFromCube1.topFace == "Top Face: X-")
  {
    NumberCube1 = 2;
  }
  else if (dataFromCube1.topFace == "Top Face: Z+" || dataFromCube1.topFace == "Top Face: Z-")
  {
    NumberCube1 = 3;
  }

  // Save the number of the top face of Cube 2
  if (dataFromCube2.topFace == "Top Face: Y+" || dataFromCube2.topFace == "Top Face: Y-")
  {
    NumberCube2 = 1;
  }
  else if (dataFromCube2.topFace == "Top Face: X+" || dataFromCube2.topFace == "Top Face: X-")
  {
    NumberCube2 = 2;
  }
  else if (dataFromCube2.topFace == "Top Face: Z+" || dataFromCube2.topFace == "Top Face: Z-")
  {
    NumberCube2 = 3;
  }

  //Save the number of the top face of Cube 3
  if (dataFromCube3.topFace == "Top Face: Y+")
  {
    NumberCube3 = 1;
  }
  else if (dataFromCube3.topFace == "Top Face: Y-")
  {
    NumberCube3 = 2;
  }
  else if (dataFromCube3.topFace == "Top Face: X+")
  {
    NumberCube3 = 3;
  }
  else if (dataFromCube3.topFace == "Top Face: X-")
  {
    NumberCube3 = 4;
  }
  else if (dataFromCube3.topFace == "Top Face: Z+")
  {
    NumberCube3 = 5;
  }
  else if (dataFromCube3.topFace == "Top Face: Z-")
  {
    NumberCube3 = 6;
  }

if (NumberCube1 + NumberCube2 == NumberCube3) {
    dataForAllCubes.color[0] = 0;
    dataForAllCubes.color[1] = 255;
    dataForAllCubes.color[2] = 0;
} else {
    dataForAllCubes.color[0] = 255;
    dataForAllCubes.color[1] = 0;
    dataForAllCubes.color[2] = 0;
}

sendFeedbackData(cubeMacAddresses[0], dataForAllCubes); //Cube 1 
sendFeedbackData(cubeMacAddresses[1], dataForAllCubes); //Cube 2
sendFeedbackData(cubeMacAddresses[2], dataForAllCubes); //Cube 3

}
Application applications[] = {
    {"0", "Color Reset", colorReset},
    {"1", "Color Test", colorTestApplication},
    {"2", "Color Theory", colorTheoryApplication},
    {"3", "Basic Math", basicMathApplication}};

const int numApplications = sizeof(applications) / sizeof(applications[0]);

#pragma endregion applications

#pragma region webserver
void handleRoot()
{
  File file = LittleFS.open("/index.html", "r");
  if (file)
  {
    server.streamFile(file, "text/html");
    file.close();
  }
  else
  {
    server.send(404, "Failed to open index.html");
  }
}

void handleSelectApplication()
{
  String selectedId = server.arg("application");
  bool validSelection = false;

  for (int i = 0; i < numApplications; i++)
  {
    if (applications[i].id == selectedId)
    {
      currentApplication = applications[i].function; // Assign the function pointer
      server.send(200, "text/plain", "Selected: " + applications[i].name);
      validSelection = true;
      break;
    }
  }

  if (!validSelection)
  {
    server.send(400, "text/plain", "Invalid selection.");
  }
}

void handleGetApplication()
{
  String response = "[";
  for (int i = 0; i < numApplications; i++)
  {
    response += "{\"id\": \"" + applications[i].id + "\", \"name\": \"" + applications[i].name + "\"}";
    if (i < numApplications - 1)
    {
      response += ", ";
    }
  }
  response += "]";
  server.send(200, "application/json", response);
}

void handleApplicationInfoUpdates()
{
  String info = "Current application State:\n";
  info += "Cube 1 Data: " + String(dataFromCube1.topFace) + ", " + String(dataFromCube1.accelX) + ", " + String(dataFromCube1.accelY) + ", " + String(dataFromCube1.accelZ) + "\n";
  info += "Cube 2 Data: " + String(dataFromCube2.topFace) + ", " + String(dataFromCube2.accelX) + ", " + String(dataFromCube2.accelY) + ", " + String(dataFromCube2.accelZ) + "\n";
  info += "Cube 3 Data: " + String(dataFromCube3.topFace) + ", " + String(dataFromCube3.accelX) + ", " + String(dataFromCube3.accelY) + ", " + String(dataFromCube3.accelZ) + "\n";
  info += "Cube 4 Data: " + String(dataFromCube4.topFace) + ", " + String(dataFromCube4.accelX) + ", " + String(dataFromCube4.accelY) + ", " + String(dataFromCube4.accelZ) + "\n";
  server.send(200, "text/plain", info);
  Serial.println(info);
}
#pragma endregion webserver

#pragma region ESP_Setup
void setup()
{
  Serial.begin(115200);

  // File setup
  if (!LittleFS.begin())
  {
    Serial.println("Error initializing LittleFS");
    return;
  }
  else
  {
    Serial.println("LittleFS initialized successfully");
  }

  // Server setup
  WiFi.mode(WIFI_AP_STA);

  WiFi.softAP(ssid, password);
  Serial.println("Access Point started.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.serveStatic("/style.css", LittleFS, "/style.css");
  server.serveStatic("/script.js", LittleFS, "/script.js");

  server.on("/select", handleSelectApplication);
  server.on("/info", handleApplicationInfoUpdates);
  server.on("/applications", handleGetApplication);

  server.begin();
  Serial.println("HTTP server started");

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callback function to receive data
  esp_now_register_recv_cb(esp_now_recv_cb_t(onReceiveData));

  // Register callback function to send data
  esp_now_register_send_cb(onSent);

  // Register cubes and add as peers
  int cubeIndex = 1;
  for (auto &macAddress : cubeMacAddresses)
  {
    addCubeAsPeer(macAddress, cubeIndex);
    cubeIndex++;
  }
}
#pragma endregion ESP_Setup

#pragma region ESP_Runtime
void loop()
{
  server.handleClient();

  if (Serial.available())
  {
    char command = Serial.read();
    if (command == '1')
    {
      currentApplication = colorTestApplication;
      Serial.println("Switched to Application: Base");
    }
    else if (command == '2')
    {
      currentApplication = colorTheoryApplication;
      Serial.println("Switched to Application: Color Combination");
    }
  }
}
#pragma endregion ESP_Runtime