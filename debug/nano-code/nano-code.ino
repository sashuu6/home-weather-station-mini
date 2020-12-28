/*
 * Project Name: Home Weather Station Mini
 * Program Name: Arduino Nano debug code
 * Created on: 13/12/2020 08:04:00 PM
 * Last Modified: 28/12/2020 08:40:00 AM
 * Created by: Sashwat K
 */

// DHT22
#include "DHT.h"
#define DHTPIN 3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// BMP280
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp;

// RTC Module
#include <RTClib.h>
RTC_DS1307 rtc;

// Rain Sensor
#define RAINSENSORPIN A0

// Rain Guage
#define RAINGUAGEPIN 4

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("\n=========================");
  Serial.println("Home Weather Station Mini");
  Serial.println("------ Arduino Nano -------");
  Serial.println("------- Debug Mode --------");
  Serial.println("=========================\n");

  Serial.println("*************************");
  Serial.println("*************************");
  Serial.println("Initialising sensors...");
  
  // Initialise DHT22
  Serial.println("1. Initialising DHT22..");
  dht.begin();
  
  // Initialise BMP280
  Serial.println("2. Initialising BMP280..");
  int bmp_lib = bmp.begin();
  if(!bmp_lib){
    Serial.println("\t BMP init failed!!");
    while(1);
  }
  else {
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  }

  // Initialise RTC module
  Serial.println("3. Initialising RTC..");
  if (! rtc.begin()) {
    Serial.println("\t RTC module not found..");
    Serial.flush();
    abort();
  }
  if (! rtc.isrunning()) {
    Serial.println("\t RTC time error... Resetting date and time...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Initialise rain sensor
  Serial.println("4. Initialising rain sensor..");
  pinMode(INPUT, RAINSENSORPIN);
  
  Serial.println("5. Initialising rain guage..");
  pinMode(INPUT, RAINGUAGEPIN);
  
  Serial.println("Initialization complete.");
  Serial.println("*************************");
  Serial.println("*************************\n\n\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("---------------------------\n");
  
  // Read data from DHT22
  float dht_humidity = dht.readHumidity();
  float dht_temperature = dht.readTemperature();
  float dht_heat_index = dht.computeHeatIndex(dht_temperature, dht_humidity, false);

  Serial.println("1. DHT22 Data:-");
  Serial.print("\t i. Humidity: "); Serial.println(dht_humidity);
  Serial.print("\t ii. Temperature: "); Serial.println(dht_temperature);
  Serial.print("\t iii. Heat Index: "); Serial.println(dht_heat_index);

  // Read data from BMP280
  double bmp_temperature, bmp_pressure, bmp_altitude;
  bmp_temperature = bmp.readTemperature();
  /* bmp_pressure = pressure_value / 100 + const
   * const = 1018.33 - (get pressure value of your location from https://en.allmetsat.com/metar-taf/)
   */
  bmp_pressure = bmp.readPressure() / 100.00 + 5.33;
  bmp_altitude = bmp.readAltitude(bmp_pressure);
  
  Serial.println("2. BMP280 Data:-");
  Serial.print("\t i. Pressure: "); Serial.println(bmp_pressure);
  Serial.print("\t ii. Altitude: "); Serial.println(bmp_altitude);
  Serial.print("\t iii. Temperature: "); Serial.println(bmp_temperature);

  // Read data from RTC module
  Serial.println("3. RTC module data:-");
  DateTime now = rtc.now();
  //Full Timestamp
  Serial.print("\t i. Date & Time (TIMESTAMP): "); Serial.println(now.timestamp(DateTime::TIMESTAMP_FULL));
  //Date Only
  Serial.print("\t ii. Date & Time (DATE): "); Serial.println(now.toString("DD-MM-YYYY"));
  //Full Timestamp
  Serial.print("\t iii. Date & Time (TIME): "); Serial.println(now.toString("hh:mm:ss"));

  // Read data from Rain Sensor
  int rain_sensor_data = analogRead(RAINSENSORPIN);
  Serial.println("4. Rain Sensor Data:-");
  Serial.print("\t i. Value: ");Serial.println(rain_sensor_data);

  // Read data from Rain Guage
  int rain_guage_data = digitalRead(RAINGUAGEPIN);
  Serial.println("5. Rain Guage Data:-");
  Serial.print("\t i. Value: ");Serial.println(rain_guage_data);
  
  Serial.println("---------------------------\n\n");
  
  delay(5000);
}
