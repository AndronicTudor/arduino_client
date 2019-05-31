#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoHttpClient.h>
#include <SoftwareSerial.h>
#include <dht.h>
#include <TinyGPS++.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

String arduino_id = "ard_001";

int gps_rx = 2;
int gps_tx = 3;
int temperature_humidity_pin = 5;
int light_pin = A1;
long previousMillis = 0;
long interval = 60000;

char serverAddress[] = "192.168.1.2"; 
int port = 8000; 

int gps_cond = 0;

EthernetClient eth;
HttpClient client = HttpClient(eth, serverAddress, port);
dht DHT11;
SoftwareSerial gps_serial(gps_rx, gps_tx); 
TinyGPSPlus gps;

String latitude="0", longitude="0";
int temperature, humidity, light;

void setup() {
  Serial.begin(9600);
  if(Ethernet.begin(mac)!=0)
  {
  
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
    Serial.println("Connecting...");
  }
  gps_serial.begin(9600);
  Serial.println("GPS Start");
}

void loop()
{
  unsigned long currentMillis = millis();
  get_gps_position();
  temperature_humidity_light();
  print_serial();
  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis; 
    post_data();
    get_data();
  }
  if(gps_cond == 1)
    post_data_gps();
}

void get_gps_position()
{
  gps_cond = 0;
  if (gps_serial.available() > 0){
    if (gps.encode(gps_serial.read())){
      latitude = String(gps.location.lat(), 6);
      longitude = String(gps.location.lng(), 6);
      gps_cond = 1;
    }
  }
}

void temperature_humidity_light()
{
  int chk = DHT11.read11(temperature_humidity_pin); 
  temperature = DHT11.temperature;
  humidity = DHT11.humidity;
  light = analogRead(light_pin);
}

void print_serial()
{
  Serial.print("Arduino id: ");  
  Serial.print(arduino_id);
  Serial.print(", Temperature: ");
  Serial.print(temperature);
  Serial.print("C, Humidity: ");
  Serial.print(humidity);
  Serial.print("%, Light: ");
  Serial.print(light);
  Serial.print(", Latitude= "); 
  Serial.print(latitude);
  Serial.print(", Longitude= ");
  Serial.print(longitude);
  Serial.println();
}

void post_data()
{
  String arduino_id_send = "id=" + arduino_id;
  String sensor_send = "hum=" + String(humidity) + "&temp=" + String(temperature) + "&lig=" + light; 
  String data_to_send = arduino_id_send + "&" + sensor_send;
  Serial.println("making POST request");
  String contentType = "application/x-www-form-urlencoded";
  client.post("/api/arduino/", contentType, data_to_send);
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.print(statusCode);
  Serial.println(response);
}

void post_data_gps()
{
  String arduino_id_send = "id=" + arduino_id;
  String location_send = arduino_id_send + "&lat=" + latitude + "&lon=" + longitude;
  String data_to_send = arduino_id_send + "&" + location_send + "&update=True";
  Serial.println("making POST request");
  String contentType = "application/x-www-form-urlencoded";
  client.post("/api/arduino/", contentType, data_to_send);
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.print(statusCode);
  Serial.println(response);
}

void get_data()
{
  Serial.println("making GET request");
  client.get("/api/arduino/ard_001/");
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Get sensor data from Data Base: ");
  Serial.println(response);
}
