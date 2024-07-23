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
int blink = 0; //if set time it is 1 else 0 
bool ledState = LOW;


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
  if (blink == 0){
    digitalWrite(4, ledState);  // Update the actual LED
  }else {
    digitalWrite(4, HIGH);
    delay(onTime);
    digitalWrite(4, LOW);
    delay(offTime);
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

            // Parse HTTP POST parameters
            
              if (header.indexOf("command=on") > 0) {
                digitalWrite(4, HIGH);
                ledState = HIGH;
                blink = 0;
                client.println("The light is now on.");
              } else if (header.indexOf("command=off") > 0) {
                digitalWrite(4, LOW);
                ledState = LOW;
                blink = 0;
                client.println("The light is now off.");
              } else if (header.indexOf("ledOnTime") > 0 && header.indexOf("ledOffTime") > 0) {
                int ledOnTimeIndex = header.indexOf("ledOnTime=") + 10;
                int ampIndex = header.indexOf("&", ledOnTimeIndex);
                String ledOnTimeStr = header.substring(ledOnTimeIndex, ampIndex);
                onTime = ledOnTimeStr.toInt();

                int ledOffTimeIndex = header.indexOf("ledOffTime=") + 11;
                int endIndex = header.indexOf(" ", ledOffTimeIndex);
                String ledOffTimeStr = header.substring(ledOffTimeIndex, endIndex);
                offTime = ledOffTimeStr.toInt();

                blink = 1;

                client.println("LED times set successfully.");
              }else if(header.indexOf("ledState") > 0){
                if(blink == 1){
                  client.print("LED is Blinking.");
                  client.print("on : ");
                  client.print(onTime);
                  client.print("off : ");
                  client.println(offTime);
                }else{
                  if(ledState == LOW){
                    client.println("LED is OFF.");
                  }else{
                    client.println("LED is ON.");
                  }
                }
              }
               else {
                client.println("Invalid command.");
              }
           

            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }

        
      }
      Serial.println(header);
    }

    // Close the connection
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
