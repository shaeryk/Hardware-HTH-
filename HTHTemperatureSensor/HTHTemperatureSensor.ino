#include <Adafruit_MLX90640.h>
#include <HTTPClient.h>
#include <WiFi.h>


Adafruit_MLX90640 mlx;
float frame[32 * 24];  // buffer for full frame of temperatures
int counter = 0;
bool occupancy = false;

// Tstat and ESP32 MUST be on the same network
const char* ssid = "tstat";
const char* password = "werty000!";

// uncomment *one* of the below
#define PRINT_TEMPERATURES
//#define PRINT_ASCIIART

#define SWITCH_PIN A3

void setup() {
  pinMode(SWITCH_PIN, INPUT);
  while (!Serial) delay(10);
  Serial.begin(115200);
  delay(100);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);  //wait until connection is established
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("ESP32 connected to Wifi network");

  Serial.println("Adafruit MLX90640 Simple Test");
  if (!mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
    Serial.println("MLX90640 not found!");
    while (1) delay(10);
  }
  Serial.println("Found Adafruit MLX90640");

  Serial.print("Serial number: ");
  Serial.print(mlx.serialNumber[0], HEX);
  Serial.print(mlx.serialNumber[1], HEX);
  Serial.println(mlx.serialNumber[2], HEX);

  //mlx.setMode(MLX90640_INTERLEAVED);
  mlx.setMode(MLX90640_CHESS);
  Serial.print("Current mode: ");
  if (mlx.getMode() == MLX90640_CHESS) {
    Serial.println("Chess");
  } else {
    Serial.println("Interleave");
  }

  mlx.setResolution(MLX90640_ADC_18BIT);
  Serial.print("Current resolution: ");
  mlx90640_resolution_t res = mlx.getResolution();
  switch (res) {
    case MLX90640_ADC_16BIT: Serial.println("16 bit"); break;
    case MLX90640_ADC_17BIT: Serial.println("17 bit"); break;
    case MLX90640_ADC_18BIT: Serial.println("18 bit"); break;
    case MLX90640_ADC_19BIT: Serial.println("19 bit"); break;
  }

  mlx.setRefreshRate(MLX90640_2_HZ);
  Serial.print("Current frame rate: ");
  mlx90640_refreshrate_t rate = mlx.getRefreshRate();
  switch (rate) {
    case MLX90640_0_5_HZ: Serial.println("0.5 Hz"); break;
    case MLX90640_1_HZ: Serial.println("1 Hz"); break;
    case MLX90640_2_HZ: Serial.println("2 Hz"); break;
    case MLX90640_4_HZ: Serial.println("4 Hz"); break;
    case MLX90640_8_HZ: Serial.println("8 Hz"); break;
    case MLX90640_16_HZ: Serial.println("16 Hz"); break;
    case MLX90640_32_HZ: Serial.println("32 Hz"); break;
    case MLX90640_64_HZ: Serial.println("64 Hz"); break;
  }
}

void loop() {

  delay(5000);  // Need to send backend okay or not okay state every 5 seconds
  counter = 0;
  occupancy = digitalRead(SWITCH_PIN);

  if (mlx.getFrame(frame) != 0) {
    Serial.println("Failed");
    return;
  }
  Serial.println("===================================");
  Serial.print("Ambient temperature = ");
  Serial.print(mlx.getTa(false));  // false = no new frame capture
  Serial.println(" degC");
  Serial.println();
  Serial.println();
  Serial.println("FrameStart");
  for (uint8_t h = 0; h < 24; h++) {
    for (uint8_t w = 0; w < 32; w++) {
      float pixTemp = frame[h * 32 + w];
      if (pixTemp > 36 && occupancy == false) {
        counter = counter + 1;
      }

#ifdef PRINT_TEMPERATURES
      Serial.print(pixTemp, 1);
      Serial.print(", ");
#endif
    }
    Serial.println();
  }
  if (counter >= 3) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin("http://jsonplaceholder.typicode.com/comments?id=10");  // Dummy HTTP server to verify logic works -> Returns a json in the serial monitor
      //http.begin ("http://{deviceIP}:8005/heatAlert/value=1"); // need to verify the HTTP command with backend

      //Assuming backend sends some data back to acknowledge request has been received
      int httpReturn = http.GET();
      if (httpReturn > 0) {
        String httpData = http.getString();
        Serial.println(httpData);
      } else {
        Serial.println("Error on HTTP Request");
      }
      http.end();
    }
    //Serial.println("1"); // Prints a 1 if an alert should be sent based on defined logic
    //delay(2500);
  }
  Serial.println("FrameEnd");
}