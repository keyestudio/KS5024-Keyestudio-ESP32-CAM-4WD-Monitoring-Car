/*
  Project: Web Page Display
  Author: Keyestudio
  Function: how to connect ESP32 to WiFi and design web page to show “Hello World”
*/
#include <WiFi.h>
#include "esp_camera.h"
#include "esp_http_server.h"

// Replace with your network credentials
const char *ssid = "your_SSID";          // Set to your Wi-Fi name
const char *password = "your_PASSWORD";  // Set to your Wi-Fi passwords

httpd_handle_t camera_httpd = NULL;  // HTTP server handle, used to start the server

// A simplified HTML page that contains only the content of the title
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<html>
  <head>
  <body>
    <h1>Hello World</h1>  <!-- 页面正文中的标题文本 -->
  </body>
</html>
)rawliteral";

// Callback function that handles root URL ("/") requests
static esp_err_t index_handler(httpd_req_t *req) {
  httpd_resp_set_type(req, "text/html");  // Set the content type of the response to HTML
  return httpd_resp_send(req, (const char *)INDEX_HTML, strlen(INDEX_HTML));  // Send an HTML response with a title
}

// Start the HTTP Server and register requests to handle the root path ("/")
void startCameraServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();  // Use the default HTTP configuration
  config.server_port = 80;  // Set the HTTP server port to 80

  // Configure request handling for the root path ("/")
  httpd_uri_t index_uri = {
    .uri = "/",               // Set the request URI path to "/"
    .method = HTTP_GET,       // Set the request method to GET
    .handler = index_handler, // Set the callback function that handles the request
    .user_ctx = NULL          // No additional context data
  };

  // Start the HTTP Server and register the root path for request processing
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(camera_httpd, &index_uri);  // Register URI handlers
  }
}

void setup() {
  Serial.begin(115200);  // Set baud rate to 115200

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);  // Start the Wi-Fi connection
  while (WiFi.status() != WL_CONNECTED) {  // If Wi-Fi is not connected, wait
    delay(500);  // Print every 500ms
    Serial.print(".");  // Print dots on the serial monitor to indicate that a connection is being attempted
  }
  Serial.println("");  // wrap
  Serial.println("WiFi connected");  // Print "WiFi connected" when the Wi-Fi connection is successful
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());   // Print IP address
  // Start the HTTP Server
  startCameraServer();  // Start the HTTP Server and register the handler
}

void loop() {
  // The main loop is empty because the HTTP server processes the request in the background
}
