#include <WiFi.h>

#define Rx 16
#define Tx 17

int Ledpin = 2;

const char* ssid = "KSDOM";
const char* password = "password";

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600,SERIAL_8N1,Rx,Tx);

  pinMode(Ledpin, OUTPUT);
  digitalWrite(Ledpin, LOW);

  delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
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
    
    server.begin();

}

void loop() {
  int tryb = 0; //zmienna definiująca tryb w jakim znajduje się centrala
  
  while(1){
  String command = Serial2.readStringUntil('\n');

  if(command == "czuwanie")
  {
    tryb = 1;
    digitalWrite(Ledpin, LOW);
    Serial.println("Czuwanie");
  }
  else if(command == "monitorowanie")
  {
    tryb = 2;
    digitalWrite(Ledpin, LOW);
    Serial.println("Monitorowanie\n");
  }
  else if(command == "alarm")
  {
    tryb = 3;
    digitalWrite(Ledpin, HIGH);
    Serial.println("Alarm");
  }

  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Kliknij <a href=\"/H\">tutaj</a> aby uzbroic alarm.<br>");
            client.print("Kliknij <a href=\"/L\">tutaj</a> aby rozbroic alarm.<br>");
            //client.print("Click <a href=\"/H\">here</a> to turn alarm on.<br>");
            //client.print("Click <a href=\"/L\">here</a> to turn alarm off.<br>");
            if(tryb == 1) {
              client.print("Czuwanie.<br>");
            }
            else if(tryb == 2){
              client.print("Monitorowanie.<br>");
            }
            else if(tryb == 3){
              client.print("Alarm naruszenie.<br>");
            }

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          Serial2.write("lock");  //przeslanie do arduino komunikatu o uzbrojeniu
        }
        if (currentLine.endsWith("GET /L")) {
          Serial2.write("unlock");   //przeslanie do arduino komunikatu o rozbrojeniu
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
  }
}
