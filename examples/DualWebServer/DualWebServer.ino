/*
 Dual Web Server
  
 A quick and dirty example of using two Ethernet adapters contemporary on Arduino GIGA R1 WIFI 
 please refer to ..
 for hardware examples

 2023 Dave Nardella

*/

/*
 Original credits
 -----------------------
 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 modified 02 Sept 2015
 by Arturo Guadalupi
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <Ethernet_SPI2.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac_SPI1[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

byte mac_SPI2[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF
};

IPAddress ip_SPI1(192, 168, 0, 177);
IPAddress ip_SPI2(192, 168, 0, 178);

// Initialize the Ethernet servers

EthernetServer server_1(80);
EthernetServer_SPI2 server_2(80);


void setup() {
  bool not_IF1;
  bool not_IF2;

// If you intend to use ETH0 on Ethernet shield and a Wiznet chip as ETH1 you could need 
// of an hardware reset pin.
// To do this connect the WXXXX reset pin on D8 (for example) and uncomment next lines
// DO NOT MODIFY THE ORDER OF THE FIRST TWO LINES
/*
  digitalWrite(8, LOW); 
  pinMode(8, OUTPUT);
  delay(650);
  pinMode(8, INPUT);
*/

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Double Ethernet WebServer Example");

  // start the Ethernet connection and the server on 1st SPI Interface
  Ethernet.init(10);   
  Ethernet.begin(mac_SPI1, ip_SPI1);

  // start the Ethernet connection and the server on 2nd SPI interface
  Ethernet_SPI2.init(9);   // Double Ethernet shield
  Ethernet_SPI2.begin(mac_SPI2, ip_SPI2);

  // Check for Ethernet hardware present on 1st SPI
  not_IF1 = Ethernet.hardwareStatus() == EthernetNoHardware;
  not_IF2 = Ethernet_SPI2.hardwareStatus() == EthernetNoHardware_SPI2;

  if (not_IF1 && not_IF2)
  {
    Serial.println("Ethernet adapters were not found on both SPI interfaces.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  else{
    if (not_IF1)
      Serial.println("Ethernet adapter was not found on 1st SPI");
    if (not_IF2)
      Serial.println("Ethernet adapter was not found on 2nd SPI");
  }

  // Check for link status
  if (Ethernet.linkStatus() == LinkOFF) 
    Serial.println("Ethernet cable is not connected on ETH0.");
 
  if (Ethernet_SPI2.linkStatus() == LinkOFF_SPI2) 
    Serial.println("Ethernet cable is not connected on ETH1.");
  
  // start the 1st server
  server_1.begin();
  Serial.print("ETH0 server is at ");
  Serial.println(Ethernet.localIP());

  // start the 2nd server
  server_2.begin();
  Serial.print("ETH1 server is at ");
  Serial.println(Ethernet_SPI2.localIP());
}


void loop() {
  // ***********************************  
  // SERVER ON ETH0
  // ***********************************  
  
  // listen for incoming clients
  EthernetClient client_1 = server_1.available();
  if (client_1) 
  {
    Serial.println("new client on ETH0");
    // an HTTP request ends with a blank line
    bool currentLineIsBlank = true;
    while (client_1.connected()) 
    {
      if (client_1.available()) {
        char c = client_1.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the HTTP request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard HTTP response header
          client_1.println("HTTP/1.1 200 OK");
          client_1.println("Content-Type: text/html");
          client_1.println("Connection: close");  // the connection will be closed after completion of the response
          client_1.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client_1.println();
          client_1.println("<!DOCTYPE HTML>");
          client_1.println("<html>");
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client_1.print("ETH0 analog input ");
            client_1.print(analogChannel);
            client_1.print(" is ");
            client_1.print(sensorReading);
            client_1.println("<br />");
          }          
          client_1.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client_1.stop();
    Serial.println("client on ETH0 disconnected");
  }

  // ***********************************  
  // SERVER ON ETH1
  // ***********************************  

  // listen for incoming clients
  EthernetClient_SPI2 client_2 = server_2.available();
  if (client_2) 
  {
    Serial.println("new client on ETH1");
    // an HTTP request ends with a blank line
    bool currentLineIsBlank = true;
    while (client_2.connected()) 
    {
      if (client_2.available()) {
        char c = client_2.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the HTTP request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard HTTP response header
          client_2.println("HTTP/1.1 200 OK");
          client_2.println("Content-Type: text/html");
          client_2.println("Connection: close");  // the connection will be closed after completion of the response
          client_2.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client_2.println();
          client_2.println("<!DOCTYPE HTML>");
          client_2.println("<html>");
          // output the value of each analog input pin
          for (int analogChannel = 0; analogChannel < 6; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client_2.print("ETH1 analog input ");
            client_2.print(analogChannel);
            client_2.print(" is ");
            client_2.print(sensorReading);
            client_2.println("<br />");
          }          
          client_2.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client_2.stop();
    Serial.println("client on ETH1 disconnected");
  }

}
