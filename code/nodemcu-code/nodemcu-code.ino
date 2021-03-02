/*
 * Project Name: Weather Station Cluster
 * Program Name: NodeMCU code
 * Created on: 20/11/2020 02:11:00 AM
 * Last Modified: 27/02/2021 03:51:00 PM
 * Created by: Sashwat K
 */

// WiFi manager & server
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#define WEB_PORT 80

WiFiManager wifiManager;
ESP8266WebServer server(WEB_PORT);
WiFiClient client;

// Micro SD card Module
#include <SPI.h>
#include <SD.h>
File sd_card;
#define CS_PIN  15

// JSON connection
#include "ArduinoJson.h"

// Software Serial
#include <SoftwareSerial.h> // Library for Software Serial
SoftwareSerial s_serial_to_mega(2,0); //RX, TX

// LED - D4 - GPIO2
#define SDCRDERRLEDPIN 2

String admin_username = "admin";
String admin_password;
String server_url;
String server_api_key;
String gps_longitude;
String gps_latitude;

int timer = 0;
int addr = 0;

// Store serial data from Software Serial
float dht_humidity_global;
float dht_temperature_global;
float dht_heat_index_global;
double bmp_temperature_global;
double bmp_pressure_global;
double bmp_altitude_global;
String rtc_day_global;
String rtc_month_global;
String rtc_year_global;
String rtc_hour_global;
String rtc_minutes_global;
String rtc_seconds_global;
int rain_sensor_data_analog_global;
int rain_sensor_data_digital_global;
int rain_guage_data_global;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("-------------------------");
  Serial.println(" Weather Station Cluster ");
  Serial.println("-------------------------");
  Serial.println("Sensor Initialisation \n");

  // Initialise LED
  pinMode(SDCRDERRLEDPIN, OUTPUT);

  // Initialise LED to off
  digitalWrite(SDCRDERRLEDPIN, LOW);

  Serial.print("1. Initialising SD card module");
  if (!SD.begin(CS_PIN)) {
    digitalWrite(SDCRDERRLEDPIN, HIGH);
    Serial.println("\t Failed");
    while (1);
  }
  else {
    Serial.println("\t Success");
  }

  wifiManager.autoConnect("Weather Station Cluster");

  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname("Weather Station Cluster");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  
  server.on("/", handle_OnConnect);
  server.on("/change_hostname", change_hostname);
  server.on("/change_server_ip", change_server_ip);
  server.on("/change_api_key", change_api_key);
  server.on("/change_lattitude", change_lattitude);
  server.on("/change_longitude", change_longitude);
  server.on("/change_admin_password", change_admin_password);
  server.on("/reset_network", reset_network);
  server.on("/reset_everything", reset_everything);
  server.onNotFound(handleNotFound);
  server.begin();
  
  Serial.println("Home weather station connected.");

  s_serial_to_mega.begin(4800);
  Serial.println("Initialisation complete");
  Serial.println("--------------------------------");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (s_serial_to_mega.available()) {
    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, s_serial_to_mega);
    if (err == DeserializationError::Ok) {
      float dht_humidity = doc["dht_humidity"];
      float dht_temperature = doc["dht_temperature"];
      float dht_heat_index = doc["dht_heat_index"];
      double bmp_temperature = doc["bmp_temperature"];
      double bmp_pressure = doc["bmp_pressure"];
      double bmp_altitude = doc["bmp_altitude"];
      String rtc_day = doc["rtc_day"];
      String rtc_month = doc["rtc_month"];
      String rtc_year = doc["rtc_year"];
      String rtc_hour = doc["rtc_hour"];
      String rtc_minutes = doc["rtc_minutes"];
      String rtc_seconds = doc["rtc_seconds"];
      int rain_sensor_data_analog = doc["rain_sensor_data_analog"];
      int rain_sensor_data_digital = doc["rain_sensor_data_digital"];
      int rain_guage_data = doc["rain_guage_data"];
      Serial.println(dht_humidity);

      // ArduinoJSON Library bug, needs spliting
      dht_humidity_global = dht_humidity;
      dht_temperature_global = dht_temperature;
      dht_heat_index_global = dht_heat_index;
      bmp_temperature_global = bmp_temperature;
      bmp_pressure_global = bmp_pressure;
      bmp_altitude_global = bmp_altitude; 
      rtc_day_global = rtc_day;
      rtc_month_global = rtc_month;
      rtc_year_global = rtc_year;
      rtc_hour_global = rtc_hour;
      rtc_minutes_global = rtc_minutes;
      rtc_seconds_global = rtc_seconds;
      rain_sensor_data_analog_global = rain_sensor_data_analog;
      rain_sensor_data_digital_global = rain_sensor_data_digital;
      rain_guage_data_global = rain_guage_data;
      
      Serial.println("--------------------------------------------");
      Serial.print("1. Humidity (DHT22): ");Serial.println(dht_humidity);
      Serial.print("2. Temperature (DHT22): ");Serial.println(dht_temperature);
      Serial.print("3. Heat Index (DHT22): ");Serial.println(dht_heat_index);
      Serial.print("4. Temperature (BMP280): ");Serial.println(bmp_temperature);
      Serial.print("5. Pressure (BMP280): ");Serial.println(bmp_pressure);
      Serial.print("6. Altitude (BMP280): ");Serial.println(bmp_altitude);
      Serial.print("7. Day (RTC): ");Serial.println(rtc_day);
      Serial.print("8. Month (RTC): ");Serial.println(rtc_month);
      Serial.print("9. Year (RTC): ");Serial.println(rtc_year);
      Serial.print("10. Hour (RTC): ");Serial.println(rtc_hour);
      Serial.print("11. Minutes (RTC): ");Serial.println(rtc_minutes);
      Serial.print("12. Seconds (RTC): ");Serial.println(rtc_seconds);
      Serial.print("13. Rain Sensor (Analog): ");Serial.println(rain_sensor_data_analog);
      Serial.print("14. Rain Sesnor (Digital): ");Serial.println(rain_sensor_data_digital);
      Serial.print("15. Rain Guage: ");Serial.println(rain_guage_data);

      if (WiFi.status() == WL_CONNECTED) {
        while(WiFi.status() == WL_CONNECTED){
          ArduinoOTA.handle();
          if (client.available()) {
            server.handleClient();
          }
        }
      }
    }
    else {
      // Print error to the "debug" serial port
      Serial.print("deserializeJson() returned ");
      Serial.println(err.c_str());

      // Flush all bytes in the "link" serial port buffer
      while (s_serial_to_mega.available() > 0)
        s_serial_to_mega.read();
    }
  }
}

void handle_OnConnect() {
  server.send(200, "text/html", dashboard());
}

void change_hostname() {
  WiFi.hostname(server.arg("device_hostname"));
  jump_to_home();
}

void change_server_ip() {
  server_url = server.arg("server_ip");
  jump_to_home();
}

void change_api_key() {
  server_api_key = server.arg("api_key");
  jump_to_home();
}

void change_lattitude() {
  gps_latitude = server.arg("change_latitude");
  jump_to_home();
}

void change_longitude() {
  gps_longitude = server.arg("change_longitude");
  jump_to_home();
}

void change_admin_password() {
  if (admin_password == server.arg("old_password")) {
    if (server.arg("new_password") == server.arg("confirm_new_password")) {
      admin_password = server.arg("new_password");
      jump_to_home();
    }
    else {
      server.send(200, "text/html", "<script>if(window.confirm(\"New Password not same.\")){document.location.href=\"/\";}</script>");
    }
  }
  else {
    server.send(200, "text/html", "<script>if(window.confirm(\"Old Password incorrect.\")){document.location.href=\"/\";}</script>");
  }
}

void reset_network() {
  wifiManager.resetSettings();
  ESP.restart();
}

void reset_everything() {
  admin_username = "admin";
  admin_password = "";
  server_url = "sashwat.in";
  server_api_key = "-";
  gps_longitude = "0";
  gps_latitude = "0";
  reset_network();
}

void jump_to_home() {
    server.send(200, "text/html", "<script>document.location.href=\"/\";</script>");
}

void handleNotFound() {
  server.send(404, "text/html",the_404_page());
}

String dashboard() {
  String wifi_ssid = WiFi.SSID().c_str();
  String device_hostname = WiFi.hostname().c_str();
  String device_ip_address = WiFi.localIP().toString();
  String device_subnet = WiFi.subnetMask().toString();
  String device_gateway = WiFi.gatewayIP().toString();
  String device_macaddress = WiFi.macAddress().c_str();
  String server_connection_status;

  if (client.connect(server_url, 80)) {
    server_connection_status = "UP";
  }
  else {
    server_connection_status = "DOWN";
  }
  
  String ptr = "<!DOCTYPE html> <html>\n";
  
  ptr += "<head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Weather Station Cluster</title>\n";
  ptr += "<style>\n";
  ptr += "html {font-family: Helvetica; font-weight: bold; display: inline-block; margin: 0px auto; text-align: center; color: whitesmoke;}\n";
  ptr += "table, th, td {margin-left: auto; margin-right: auto; border: 1px solid whitesmoke;}\n";
  ptr += "body{margin-top: 50px;}\n";
  ptr += "h1 {color: whitesmoke;margin: 50px auto 30px;}\n";
  ptr += "h2 {color: whitesmoke;margin-bottom: 50px;}\n";
  ptr += "button {background-color: #4CAF50; border: none; color: white; text-align: center; text-decoration: none; display: inline-block; margin: 4px 2px; cursor: pointer; border-radius: 4px;}\n";
  ptr += "body {background: linear-gradient(-45deg, #ee7752, #e73c7e, #23a6d5, #23d5ab);background-size: 400% 400%;animation: gradient 15s ease infinite;}\n";
  ptr += "@keyframes gradient {0% {background-position: 0% 50%;}50% {background-position: 100% 50%;}100% {background-position: 0% 50%;}}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "<script type = \"text/javascript\">\n";
  ptr += "function login() {\n";
  ptr += "var username = prompt(\"Please enter your username\");\n";
  ptr += "var password = prompt(\"Please enter your password\");\n";
  ptr += "if (username == \"" + admin_username +"\" && password == \"" + admin_password + "\") {\n";
  ptr += "alert(\"Login Success\"); }\n";
  ptr += "else {\n";
  ptr += "alert(\"Invalid username and password\");window.location.reload(); } }\n";
  ptr += "login();\n";
  ptr += "</script>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Weather Station Cluster Dashboard</h1>\n";
  ptr += "<h2>Device Status</h2>\n";
  ptr += "<table> <tr> <td>WiFi SSID: </td> <td colspan=\"2\">" + wifi_ssid + "</td> </tr> <tr> <td>Hostname: </td> <td colspan=\"2\">" + device_hostname + "</td> </tr> <tr> <td>IP address: </td><td colspan=\"2\">" + device_ip_address + "</td> </tr> ";
  ptr += "<tr> <td>Subnet: </td> <td colspan=\"2\">" + device_subnet + "</td> </tr> <tr> <td>Gateway: </td> <td colspan=\"2\">" + device_gateway + "</td> </tr> ";
  ptr += "<tr> <td>MAC Address: </td> <td colspan=\"2\">" + device_macaddress + "</td> </tr> <tr> <td>Server IP Address: </td> <td colspan=\"2\">" + server_url + "</td> </tr>";
  ptr += "<tr> <td>Server Status: </td> <td colspan=\"2\">" + server_connection_status + "</td> </tr> <tr> <td>API Key: </td> <td colspan=\"2\">" + server_api_key + "</td> </tr> ";
  ptr += "<tr> <td>Connection Status: </td> <td colspan=\"2\">Authenticated</td> </tr> <tr> <td>GPS : </td> <td>" + gps_latitude + ", " + gps_longitude + "</td> <td><a href=\"https://www.google.com/maps/@" + gps_latitude + "," + gps_longitude + ",15z\" target=\"_blank\">Open in maps</a></td> ";
  ptr += "</tr> <tr> <td colspan=\"3\"><button onclick = \"window.location.reload();\">Refresh</button></td> </tr> </table>\n";
  ptr += "<h2>Data From Station</h2>\n";
  ptr += "<table> <tr> <td>Date: </td> <td colspan=\"2\">" + String(rtc_day_global) + "-" + String(rtc_month_global) + "-" + String(rtc_year_global) + "</td> </tr> <tr> <td>Time: </td> <td colspan=\"2\">" + String(rtc_hour_global) + ":" + String(rtc_minutes_global) + ":" + String(rtc_seconds_global) + "</td> </tr> ";
  ptr += "<tr> <td>Humidity (DHT22): </td><td colspan=\"2\">" + String(dht_humidity_global) + "</td> </tr> <tr> <td>Temperature (DHT22): </td><td colspan=\"2\">" + String(dht_temperature_global) + "</td> </tr> <tr> <td>Heat Index (DHT22): </td><td colspan=\"2\">" + String(dht_heat_index_global) + "</td> </tr>";
  ptr += "<tr> <td>Temperature (BMP280): </td><td colspan=\"2\">" + String(bmp_temperature_global) + "</td> </tr> <tr> <td>Pressure (BMP280): </td><td colspan=\"2\">" + String(bmp_pressure_global) + "</td> </tr> <tr> <td>Altitude (BMP280): </td><td colspan=\"2\">" + String(bmp_altitude_global) + "</td> </tr>";
  ptr += "<tr> <td>Rain Sensor (Analog): </td><td colspan=\"2\">" + String(rain_sensor_data_analog_global) + "</td> </tr> <tr> <td>Rain Sensor (Digital): </td><td colspan=\"2\">" + String(rain_sensor_data_digital_global) + "</td> </tr>";
  ptr += "</tr> <tr> <td>Rain Guage: </td><td colspan=\"2\">" + String(rain_guage_data_global) + "</td> </tr> </table>\n";
  ptr += "<h2>Device Settings</h2>\n";
  ptr += "<table> <tr> <td>Hostname</td> <td> <form action=\"/change_hostname\" method=\"POST\"> <input type=\"text\" name=\"device_hostname\"></td> <td><button onclick=\"return confirm('Are you sure?');\">Change</button></form></td> </tr>\n";
  ptr += "<tr> <td>Server IP:</td> <td> <form action=\"/change_server_ip\" method=\"POST\"><input type=\"text\" name=\"server_ip\"></td> <td><button onclick=\"return confirm('Are you sure?');\">Change</button></form></td> </tr>\n";
  ptr += "<tr> <td>API Key: </td> <td> <form action=\"/change_api_key\" method=\"POST\"> <input type=\"text\" name=\"api_key\"></td> <td><button onclick=\"return confirm('Are you sure?');\">Change</button></form></td> </tr>\n";
  ptr += "<tr> <td>GPS Lattitude: </td> <td> <form action=\"/change_lattitude\" method=\"POST\"> <input type=\"text\" name=\"change_latitude\"> </td> <td> <button onclick=\"return confirm('Are you sure?');\">Change</button> </form> </td>\n";
  ptr += "<tr> <td>GPS Longitude: </td> <td> <form action=\"/change_longitude\" method=\"POST\"> <input type=\"text\" name=\"change_longitude\"> </td> <td> <button onclick=\"return confirm('Are you sure?');\">Change</button> </form> </td> </table>\n";
  ptr += "<h2>Admin Settings</h2>\n";
  ptr += "<form action=\"/change_admin_password\" method=\"POST\"> <table> <tr> <td>Old password</td> <td><input type=\"password\" name=\"old_password\"></td> </tr> <tr> <td>New password</td> <td><input type=\"password\" name=\"new_password\"> </td> </tr>\n";
  ptr += "<tr> <td>Confirm new password</td> <td><input type=\"password\" name=\"confirm_new_password\"></td> </tr> <tr> <td colspan=\"2\"> <button onclick=\"return confirm('Are you sure?');\">Change</button></td> </tr> </table> </form>\n";
  ptr += "<h2>Reset</h2>\n";
  ptr += "<table> <tr> <td> Reset Network </td> <td> <form action=\"/reset_network\" method=\"POST\"> <button onclick=\"return confirm('Are you sure?');\">Submit</button> </form> </td> </tr>\n";
  ptr += "<tr> <td> All </td> <td> <form action=\"/reset_everything\" method=\"POST\"> <button onclick=\"return confirm('Are you sure?');\">Submit</button> </form> </td> </tr> </table>";
  ptr += "</body>\n";
  ptr += "</html>\n";
  
  return ptr;
}

String the_404_page() {
  String ptr = "<!DOCTYPE html> <html>\n";

  ptr += "<head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>Home Weather Station Mini</title>\n";
  ptr += "<link href=\"https://fonts.googleapis.com/css2?family=Nunito+Sans:wght@600;900&display=swap\" rel=\"stylesheet\">\n";
  ptr += "<style>body {background-color: #95c2de;}\n";
  ptr += ".mainbox {background-color: #95c2de;margin: auto;height: 600px;width: 600px;position: relative;}\n";
  ptr += ".err {color: #ffffff;font-family: 'Nunito Sans', sans-serif;font-size: 11rem;position:absolute;left: 20%;top: 8%;}\n";
  ptr += ".far {position: absolute;font-size: 8.5rem;left: 42%;top: 15%;color: #ffffff;}\n";
  ptr += ".err2 {color: #ffffff;font-family: 'Nunito Sans', sans-serif;font-size: 11rem;position:absolute;left: 68%;top: 8%;}\n";
  ptr += ".msg {text-align: center;font-family: 'Nunito Sans', sans-serif;font-size: 1.6rem;position:absolute;left: 16%;top: 45%;width: 75%;}\n";
  ptr += "a {text-decoration: none; color: white;}\n";
  ptr += "a:hover {text-decoration: underline;}</style>\n";
  ptr += "<script src=\"https://kit.fontawesome.com/4b9ba14b0f.js\" crossorigin=\"anonymous\"></script>\n";
  ptr += "</head> <body>\n";
  ptr += "<div class=\"mainbox\"><div class=\"err\">4</div><i class=\"far fa-question-circle fa-spin\"></i><div class=\"err2\">4</div>\n";
  ptr += "<div class=\"msg\">Maybe this page moved? Got deleted? Is hiding out in quarantine? Never existed in the first place?<p>Let's go <a href=\"/\">home</a> and try from there.</p></div>";
  ptr += "</div></body></html>\n";
  
  return ptr;
}

void print_diagnostic_data(double bmp_temperature, double bmp_pressure, double bmp_altitude, float dht_humidity, float dht_temperature, float dht_heat_index) {
  Serial.println("\n=============================================");
  Serial.println("=============================================\n");
  Serial.println("WiFi Details");
  Serial.println("============");
  Serial.print("SSID: ");Serial.println(WiFi.SSID());
  Serial.print("IP address: ");Serial.println(WiFi.localIP());
  Serial.print("Subnet: ");Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");Serial.println(WiFi.gatewayIP());

  Serial.println("\nDevice Details");
  Serial.print("MAC address: ");Serial.println(WiFi.macAddress());
  Serial.print("Hostname: ");Serial.println(WiFi.hostname());
  
  Serial.println("\nBMP280 Sensor");
  Serial.println("===============");
  Serial.print("Temperature: ");Serial.println(bmp_temperature);
  Serial.print("Pressure: ");Serial.println(bmp_pressure);
  Serial.print("Altitude: ");Serial.println(bmp_altitude);
  
  Serial.println("\nDHT22 Sensor");
  Serial.println("============");
  Serial.print("Humidity: ");Serial.println(dht_humidity);
  Serial.print("Temperature: ");Serial.println(dht_temperature);
  Serial.print("dht_heat_index: ");Serial.println(dht_heat_index);
  
  Serial.println("\n=============================================");
  Serial.println("=============================================\n");
}
