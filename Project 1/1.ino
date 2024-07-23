#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
const char *ssid = "***";
const char *password = "***";

// Create a server at port 80
WiFiServer server(80);

// Variables to store the On and Off times
unsigned long onTime = 1000;  // Default to 1 second on
unsigned long offTime = 1000; // Default to 1 second off
bool ledState = LOW;
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  pinMode(4, OUTPUT);  // Set the LED pin mode

  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Start the server
  server.begin();
}

void loop() {
  // Handle LED timing
  unsigned long currentMillis = millis();

  if (ledState == HIGH && currentMillis - previousMillis >= onTime) {
    ledState = LOW;  // Turn it off
    previousMillis = currentMillis; // Remember the time
    digitalWrite(4, ledState);  // Update the actual LED
  } else if (ledState == LOW && currentMillis - previousMillis >= offTime) {
    ledState = HIGH;  // Turn it on
    previousMillis = currentMillis; // Remember the time
    digitalWrite(4, ledState);  // Update the actual LED
  }

  WiFiClient client = server.available();  // Listen for incoming clients

  if (client) {
    Serial.println("New Client.");
    String currentLine = "";  // Make a String to hold incoming data from the client
    String header = "";       // Store the HTTP request

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        header += c;

        if (c == '\n') {
          // If the current line is blank, you got two newline characters in a row
          // that's the end of the client HTTP request, so send a response
          if (currentLine.length() == 0) {
            // HTTP response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // HTML content
            client.print("<html><form action=\"\" method=\"get\" class=\"form-example\">");
            client.print("<label for=\"ON\">Time(ms) on: </label>");
            client.print("<input type=\"text\" name=\"On\" id=\"On\" required><br><br>");
            client.print("<label for=\"OFF\">Time(ms) off: </label>");
            client.print("<input type=\"text\" name=\"off\" id=\"off\" required><br><br>");
            client.print("<input type=\"submit\" value=\"Send\"></form></html>");

            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    // Parse HTTP GET parameters
    if (header.indexOf("GET /?On=") >= 0) {
      int onIndex = header.indexOf("On=") + 3;
      int ampIndex = header.indexOf("&", onIndex);
      String onTimeStr = header.substring(onIndex, ampIndex);
      onTime = onTimeStr.toInt();

      int offIndex = header.indexOf("off=") + 4;
      int endIndex = header.indexOf(" ", offIndex);
      String offTimeStr = header.substring(offIndex, endIndex);
      offTime = offTimeStr.toInt();

      Serial.print("Received On Time: ");
      Serial.println(onTime);
      Serial.print("Received Off Time: ");
      Serial.println(offTime);
    }

    // Close the connection
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
