/*This sketch demonstrates how to set up a simple HTTP-like server.
  The server will set a GPIO pin depending on the request
  http://server_ip/gpio/0 will set the GPIO2 low,
  http://server_ip/gpio/1 will set the GPIO2 high
  server_ip is the IP address of the ESP8266 module, will be
  printed to Serial when the module is connected.*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "SLT-4G-AA7E";
const char* password = "LQAJLDDYB8Q";

const char* mqtt_server = "test.mosquitto.org";
const char* outTopic = "ENTC/EN2560/out/000001"; //Public topic
const char* inTopic = "ENTC/EN2560/in/000001"; //Subcribe topic

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);
char AP_IP[32];
String server_ip="";


//...................................................................................................................................................
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
//...................................................................................................................................................
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println();
}

//................................................................................................................................................... 
void reconnect_mqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(outTopic, "hello world");
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//................................................................................................................................................... 
void make_server() {
  Serial.print("Hi Wellcome");
  // prepare LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();
  Serial.println("HTTP server started");
  sprintf(AP_IP, "%d.%d.%d.%d", myIP[0], myIP[1], myIP[2], myIP[3]);
  server_ip=String(AP_IP); 
}

//................................................................................................................................................... 

void new_client() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  Serial.println(F("new client"));
  client.setTimeout(5000); // default is 1000
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(F("request: "));
  Serial.println(req);
  // Match the request
  int val;
  if (req.indexOf(F("/gpio/0")) != -1) {
    val = 0;
  } else if (req.indexOf(F("/gpio/1")) != -1) {
    val = 1;
  } else {
    Serial.println(F("invalid request"));
    val = digitalRead(LED_BUILTIN);
  }
  // Set LED according to the request
  digitalWrite(LED_BUILTIN, val);

  // read/ignore the rest of the request
  // do not client.flush(): it is for output only, see below
  while (client.available()) { 
    // byte by byte is not very efficient
    client.read();
  }
}

//................................................................................................................................................... 

void setup() {
  Serial.begin(9600);
  setup_wifi();
  make_server();
  client.setServer(inTopic, 1883);
  client.setCallback(callback);
}

void loop() {
  reconnect_mqtt();
  client.setCallback(callback);

  // Send the response to the client
  // it is OK for multiple small client.print/write,
  // because nagle algorithm will group them into one single packet
  
  /*
  client.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now "));
  client.print((val) ? F("low") : F("high"));
  client.print(F("<br><br>Click <a href='http://"));
  client.print(server_ip);
  client.print(F("/gpio/1'>here</a> to switch LED GPIO off, or <a href='http://"));
  client.print(server_ip);
  client.print(F("/gpio/0'>here</a> to switch LED GPIO on .</html>"));*/

  // The client will actually be *flushed* then disconnected
  // when the function returns and 'client' object is destroyed (out-of-scope)
  // flush = ensure written data are received by the other side
  
  
  //Serial.println(F("Disconnecting from client"));
}
