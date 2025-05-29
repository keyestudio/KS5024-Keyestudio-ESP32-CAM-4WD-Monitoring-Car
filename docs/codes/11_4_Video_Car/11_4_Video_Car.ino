/*
  Project: camera robot car
  Author: Keyestudio
  Function: We can control the car to move forward/backward and turn left/right, turn on/off LED and speed up/down speed through wifi
  Speed level: we divide the maximum value of 255 into three parts, so each is 85. low speed 85, mid speed 170, high speed 255
*/
#include "esp_camera.h"        //ESP32-CAM camera driver
#include <WiFi.h>              //WiFi library, used to connect to network
#include "esp_timer.h"         //timer library
#include "img_converters.h"    //image converter library, used to convert JPEG
#include "Arduino.h"           //Arduino library
#include "fb_gfx.h"            //Graphics library, used to display image buffers
#include "soc/soc.h"           // Used to disable brownout detection for ESP32
#include "soc/rtc_cntl_reg.h"  // Used to disable brownout detection for ESP32
#include "esp_http_server.h"   // ESP32 HTTP server library, used to handle Web requests

// Replace with your network credentials
 const char *ssid = "your_SSID";      // Set to your Wi-Fi name
 const char *password = "your_PASSWORD";  // Set to your Wi-Fi passwords

//Set camera pins
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

//Set motor pins
#define MOTOR_R_PIN_1 14
#define MOTOR_R_PIN_2 15
#define MOTOR_L_PIN_1 13
#define MOTOR_L_PIN_2 12
//Set LED pins
#define LED_GPIO_NUM 4  
//The variable of speed value is initially 170
int MOTOR_R_Speed = 170;
int MOTOR_L_Speed = 170;

#define PART_BOUNDARY "123456789000000000000987654321" // A boundary used to split MIME streams
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t camera_httpd = NULL;
httpd_handle_t stream_httpd = NULL;

void startCameraServer();

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  //disable brownout detector

  pinMode(MOTOR_R_PIN_1, OUTPUT);
  pinMode(MOTOR_R_PIN_2, OUTPUT);
  pinMode(MOTOR_L_PIN_1, OUTPUT);
  pinMode(MOTOR_L_PIN_2, OUTPUT);
  pinMode(LED_GPIO_NUM, OUTPUT);  // LED is initially in output mode
  Serial.begin(115200);
  Serial.setDebugOutput(false);

//Configure camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_HVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("Camera Stream Ready! Go to: http://");
  Serial.println(WiFi.localIP());

  // Start streaming web server
  startCameraServer();
}

void loop() {
}

//Design control web page
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
  <head>
    <title>ESP32-CAM Robot</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8"/>

    <style>
      body {
        font-family: Arial;
        text-align: center;
        margin: 0 auto;
        padding-top: 20px;
      }

      .button-container {
        display: grid;
        grid-template-areas:
          "keyes forward led"
          "left stop right"
          "plus backward minus";  /* Adjust position */
        grid-gap: 10px;
        justify-content: center;
        align-content: center;
        margin-top: 20px;
      }

      .button {
        background-color: #2f4468;
        color: white;
        border: none;
        padding: 20px 0;
        text-align: center;
        font-size: 18px;
        cursor: pointer;
        width: 90px; /* Uniform width */
        height: 60px; /* Uniform heigth */
        border-radius: 15px; /* Fillet corner */
      }

      .led-button {
        background-color: #777; /* Initial gray, LED off */
        color: white;
        border: none;
        padding: 20px 0;
        text-align: center;
        font-size: 18px;
        cursor: pointer;
        width: 90px;
        height: 60px;
        border-radius: 15px;
      }

      .led-on {
        background-color: #f0c40f; /* Yellow, LED on */
        color: black;
      }

      .forward { grid-area: forward; }
      .led { grid-area: led; }
      .left { grid-area: left; }
      .stop { grid-area: stop; }
      .right { grid-area: right; }
      .backward { grid-area: backward; }
      .backwa { grid-area: backwa; }
      .plus { grid-area: plus; }
      .minus { grid-area: minus; }
      .keyes { grid-area: keyes; }

      img {
        width: auto;
        max-width: 100%;
        height: auto;
        border: 2px solid #2f4468; /* Give the video a border */
        border-radius: 10px;
        margin-top: 20px;
      }
    </style>
  </head>
  <body>
    <h1>ESP32-CAM Robot</h1>
    
    <!-- Video stream display -->
    <img src="" id="photo">

    <!-- Button container -->
    <div class="button-container">
      <!-- Forward -->
      <button class="button forward" onmousedown="toggleCheckbox('forward');" ontouchstart="toggleCheckbox('forward');" onmouseup="toggleCheckbox('stop');" ontouchend="toggleCheckbox('stop');">↑</button>
      
      <!-- LED on/off -->
      <button id="ledButton" class="led-button led" onclick="toggleLED()">OFF</button>
      
      <!-- other buttons -->
      <button class="button left" onmousedown="toggleCheckbox('left');" ontouchstart="toggleCheckbox('left');" onmouseup="toggleCheckbox('stop');" ontouchend="toggleCheckbox('stop');">←</button>
      <button class="button stop" onmousedown="toggleCheckbox('stop');">●</button>
      <button class="button right" onmousedown="toggleCheckbox('right');" ontouchstart="toggleCheckbox('right');" onmouseup="toggleCheckbox('stop');" ontouchend="toggleCheckbox('stop');">→</button>
      <button class="button backward" onmousedown="toggleCheckbox('backward');" ontouchstart="toggleCheckbox('backward');" onmouseup="toggleCheckbox('stop');" ontouchend="toggleCheckbox('stop');">↓</button>
      <button class="button plus"  onmouseup="toggleCheckbox('plus');">+</button>
      <button class="button minus" onmouseup="toggleCheckbox('minus');">-</button>
      <button class="button keyes" >Keyes</button>
    </div>

    <script>
      // Video stream loading
      window.onload = function () {
        document.getElementById("photo").src = window.location.href.slice(0, -1) + ":81/stream";
      };

      // Control button request
      function toggleCheckbox(action) {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/action?go=" + action, true);
        xhr.send();
      }

      // Logic of LED on/off
      let ledState = false; // LED state
      const ledButton = document.getElementById("ledButton");

      function toggleLED() {
        ledState = !ledState; // switch state
        if (ledState) {
          ledButton.classList.add("led-on");
          ledButton.textContent = "ON";
        } else {
          ledButton.classList.remove("led-on");
          ledButton.textContent = "OFF";
        }

        // Send LED state to server
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/action?led=" + (ledState ? "on" : "off"), true);
        xhr.send();
      }
    </script>
  </body>
</html>
)rawliteral";

static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));
}

static esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t *_jpg_buf = NULL;
  char *part_buf[64];

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) {
    return res;
  }

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if (fb->width > 400) {
        if (fb->format != PIXFORMAT_JPEG) {
          bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if (!jpeg_converted) {
            Serial.println("JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          _jpg_buf_len = fb->len;
          _jpg_buf = fb->buf;
        }
      }
    }
    if (res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if (_jpg_buf) {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if (res != ESP_OK) {
      break;
    }
    //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
  }
  return res;
}

// Control action processing
static esp_err_t action_handler(httpd_req_t *req) {
  char query[100];
  int len = httpd_req_get_url_query_len(req) + 1;
  if (len > sizeof(query)) {
    httpd_resp_send_404(req);
    return ESP_OK;
  }

  if (httpd_req_get_url_query_str(req, query, len) == ESP_OK) {
    if (strstr(query, "go=forward")) {
      // Forward
      Serial.println("Forward");
      analogWrite(MOTOR_R_PIN_1, 0);
      analogWrite(MOTOR_R_PIN_2, MOTOR_R_Speed);
      analogWrite(MOTOR_L_PIN_1, MOTOR_L_Speed);
      analogWrite(MOTOR_L_PIN_2, 0);
    } else if (strstr(query, "go=backward")) {
      // Backward
      Serial.println("Backward");
      analogWrite(MOTOR_R_PIN_1, MOTOR_R_Speed);
      analogWrite(MOTOR_R_PIN_2, 0);
      analogWrite(MOTOR_L_PIN_1, 0);
      analogWrite(MOTOR_L_PIN_2, MOTOR_L_Speed);
    } else if (strstr(query, "go=left")) {
      // Left
      Serial.println("Left");
      analogWrite(MOTOR_R_PIN_1, 0);
      analogWrite(MOTOR_R_PIN_2, MOTOR_R_Speed);
      analogWrite(MOTOR_L_PIN_1, 0);
      analogWrite(MOTOR_L_PIN_2, MOTOR_L_Speed);
    } else if (strstr(query, "go=right")) {
      // Right
      Serial.println("Right");
      analogWrite(MOTOR_R_PIN_1, MOTOR_R_Speed);
      analogWrite(MOTOR_R_PIN_2, 0);
      analogWrite(MOTOR_L_PIN_1, MOTOR_L_Speed);
      analogWrite(MOTOR_L_PIN_2, 0);
    } else if (strstr(query, "go=stop")) {
      // Stop
      Serial.println("Stop");
      analogWrite(MOTOR_R_PIN_1, 0);
      analogWrite(MOTOR_R_PIN_2, 0);
      analogWrite(MOTOR_L_PIN_1, 0);
      analogWrite(MOTOR_L_PIN_2, 0);
    } else if (strstr(query, "led=on")) {
      Serial.println("LED ON");
      digitalWrite(LED_GPIO_NUM, HIGH);  // LED开启
    } else if (strstr(query, "led=off")) {
      Serial.println("LED OFF");
      digitalWrite(LED_GPIO_NUM, LOW);  // LED关闭
    } else if (strstr(query, "go=plus")) {  

      MOTOR_R_Speed = MOTOR_R_Speed + 85;
      MOTOR_L_Speed = MOTOR_L_Speed + 85;
      if (MOTOR_L_Speed >= 255) MOTOR_L_Speed = 255;
      if (MOTOR_R_Speed >= 255) MOTOR_R_Speed = 255;
      // Serial.println("Speed +");
      // Serial.print("MOTOR_L_Speed:");
      // Serial.println(MOTOR_L_Speed);
      // Serial.print("MOTOR_R_Speed:");
      // Serial.println(MOTOR_R_Speed);
    } else if (strstr(query, "go=minus")) {

      MOTOR_R_Speed = MOTOR_R_Speed - 85;
      MOTOR_L_Speed = MOTOR_L_Speed - 85;
      if (MOTOR_L_Speed <= 85) MOTOR_L_Speed = 85;
      if (MOTOR_R_Speed <= 85) MOTOR_R_Speed = 85;
      // Serial.println("Speed -");
      // Serial.print("MOTOR_L_Speed:");
      // Serial.println(MOTOR_L_Speed);
      // Serial.print("MOTOR_R_Speed:");
      // Serial.println(MOTOR_R_Speed);
    }
  }

  httpd_resp_send(req, "", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_handler,
    .user_ctx = NULL
  };

  httpd_uri_t cmd_uri = {
    .uri = "/action",
    .method = HTTP_GET,
    .handler = action_handler,
    .user_ctx = NULL
  };
  httpd_uri_t stream_uri = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = stream_handler,
    .user_ctx = NULL
  };
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);
    httpd_register_uri_handler(camera_httpd, &cmd_uri);
  }
  config.server_port += 1;
  config.ctrl_port += 1;
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(stream_httpd, &stream_uri);
  }
}
