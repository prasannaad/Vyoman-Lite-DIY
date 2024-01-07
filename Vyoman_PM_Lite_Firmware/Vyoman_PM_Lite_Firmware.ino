#define uS_TO_S_FACTOR 1000000  
#define TIME_TO_SLEEP  10

#include <sps30.h>

#if defined(ESP32)
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;
  #define DEVICE "Vyoman_PM_Lite_002"
  #elif defined(ESP8266)
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;
  #define DEVICE "ESP8266"
  #endif
  
  #include <InfluxDbClient.h>
  #include <InfluxDbCloud.h>
  
  // WiFi AP SSID
const char* ssid     = "xxxxxxxxxxxxx";
const char* password = "xxxxxxxxxxxxxxx";

  
  #define INFLUXDB_URL "xxxxxxxxxxxxxxx"
  #define INFLUXDB_TOKEN "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
  #define INFLUXDB_ORG "xxxxxxxxxxxxxxxx"
  #define INFLUXDB_BUCKET "xxxxxxxxxxxxxxxxxx"
  
  // Time zone info
  #define TZ_INFO "IST-5:30"
  
  // Declare InfluxDB client instance with preconfigured InfluxCloud certificate
  InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
  
  // Declare Data point
  //Point sensor("wifi_status");
  
  Point pmSensor("pm_sensor_readings");
  
float pm1 = 0;
float pm2 = 0;
float pm4 = 0;
float pm10 = 0;
float NCPMp5 = 0;
float NCPM1p0 = 0;
float NCPM2p5 = 0;
float NCPM4p0 = 0;
float NCPM10p0 = 0;
float typicalsize = 0;
int count = 0;

unsigned long previousMillis = 0;
unsigned long interval = 30000;


void print_wakeup_reason()
{
esp_sleep_wakeup_cause_t wakeup_reason;

wakeup_reason = esp_sleep_get_wakeup_cause();
Serial.println();
Serial.println();
Serial.println();
switch(wakeup_reason)
{
case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
}
}


void initWiFi() 
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());

  initInflux();

}

    

void initInflux()
{
    // Accurate time is necessary for certificate validation and writing in batches
    // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
    // Syncing progress and the time will be printed to Serial.
    timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");
  
  
    // Check server connection
    if (client.validateConnection()) {
      Serial.print("Connected to InfluxDB: ");
      Serial.println(client.getServerUrl());
    } else {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
    }
  
   // Add tags to the data point
  //sensor.addTag("device", DEVICE);
  //sensor.addTag("SSID", WiFi.SSID());
  
  pmSensor.addTag("device", DEVICE);
  pmSensor.addTag("location", "office");
  pmSensor.addTag("sensor", "SPS30");  


}


void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_sta_disconnected.reason);
  Serial.println("Trying to Reconnect");
  
  initWiFi(); 

}

void setup() 
  {
    Serial.begin(115200);
    
    pinMode(LED_BUILTIN, OUTPUT);
  
  
  print_wakeup_reason();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds"); 


    // delete old config
    WiFi.disconnect(true);
    
    delay(1000);
   
    WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  
   initWiFi();

   
  int16_t ret;
  uint8_t auto_clean_days = 4;
  uint32_t auto_clean;

  Serial.begin(9600);
  delay(2000);

  sensirion_i2c_init();

  while (sps30_probe() != 0) {
    Serial.print("SPS sensor probing failed\n");
    delay(500);
  }

#ifndef PLOTTER_FORMAT
  Serial.print("SPS sensor probing successful\n");
#endif /* PLOTTER_FORMAT */

  ret = sps30_set_fan_auto_cleaning_interval_days(auto_clean_days);
  if (ret) {
    Serial.print("error setting the auto-clean interval: ");
    Serial.println(ret);
  }

  ret = sps30_start_measurement();
  if (ret < 0) {
    Serial.print("error starting measurement\n");
  }

#ifndef PLOTTER_FORMAT
  Serial.print("measurements started\n");
#endif /* PLOTTER_FORMAT */
  
  
  }



void loop() 

{
//    // Clear fields for reusing the point. Tags will remain the same as set above.
//    sensor.clearFields();
//    
//    // Store measured value into point
//    // Report RSSI of currently connected network
//    sensor.addField("rssi", WiFi.RSSI());
//  
//    // Print what are we exactly writing
//    Serial.print("Writing: ");
//    Serial.println(sensor.toLineProtocol());
//  
//    // Check WiFi connection and reconnect if needed
//    if (wifiMulti.run() != WL_CONNECTED) {
//      Serial.println("Wifi connection lost");
//    }
//  
//    // Write point
//    if (!client.writePoint(sensor)) {
//      Serial.print("InfluxDB write failed: ");
//      Serial.println(client.getLastErrorMessage());
//    }
//  
//    // Write point
//    if (!client.writePoint(pmSensor)) {
//      Serial.print("InfluxDB write failed: ");
//      Serial.println(client.getLastErrorMessage());
//    }    
    
  struct sps30_measurement m;
  char serial[SPS30_MAX_SERIAL_LEN];
  uint16_t data_ready;
  int16_t ret;

  for ( int i = 0; i<30; i++)
  {
  do {
    ret = sps30_read_data_ready(&data_ready);
    if (ret < 0) {
      Serial.print("error reading data-ready flag: ");
      Serial.println(ret);
    } else if (!data_ready)
      Serial.print("data not ready, no new measurement available\n");
    else
      break;
    delay(100); /* retry in 100ms */
  } while (1);

  ret = sps30_read_measurement(&m);
  if (ret < 0) {
    Serial.print("error reading measurement\n");
  } else 
  {
    digitalWrite(LED_BUILTIN, HIGH);

    Serial.print("PM  1.0: ");
    Serial.println(m.mc_1p0);
    Serial.print("PM  2.5: ");
    Serial.println(m.mc_2p5);
    Serial.print("PM  4.0: ");
    Serial.println(m.mc_4p0);
    Serial.print("PM 10.0: ");
    Serial.println(m.mc_10p0);

    Serial.print("NC  0.5: ");
    Serial.println(m.nc_0p5);
    Serial.print("NC  1.0: ");
    Serial.println(m.nc_1p0);
    Serial.print("NC  2.5: ");
    Serial.println(m.nc_2p5);
    Serial.print("NC  4.0: ");
    Serial.println(m.nc_4p0);
    Serial.print("NC 10.0: ");
    Serial.println(m.nc_10p0);

    Serial.print("Typical particle size: ");
    Serial.println(m.typical_particle_size);

    pm1 = m.mc_1p0;
    pm2 = m.mc_2p5;
    pm4 = m.mc_4p0;
    pm10 = m.mc_10p0;
    NCPMp5 = m.nc_0p5;
    NCPM1p0 = m.nc_1p0;
    NCPM2p5 = m.nc_2p5;
    NCPM4p0 = m.nc_4p0;
    NCPM10p0 = m.nc_10p0;
    typicalsize = m.typical_particle_size;

  
  }
  delay(1000);
  
  digitalWrite(LED_BUILTIN, LOW);
  
  }
  
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
    
    initInflux();
  
  }   
  
    pmSensor.addField("pm1", pm1);
    pmSensor.addField("pm2p5", pm2);
    pmSensor.addField("pm4", pm4);
    pmSensor.addField("pm10", pm10);
    pmSensor.addField("ncpm0p5", NCPMp5);
    pmSensor.addField("ncpm1p0", NCPM1p0);
    pmSensor.addField("ncpm2p5", NCPM2p5);
    pmSensor.addField("ncpm4p0", NCPM4p0);
    pmSensor.addField("ncpm10p0", NCPM10p0);
    pmSensor.addField("typicalsize", typicalsize);
    

  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(client.pointToLineProtocol(pmSensor));
  
  // Write point into buffer
  client.writePoint(pmSensor);

  // Clear fields for next usage. Tags remain the same.
  pmSensor.clearFields();
  
  //esp_deep_sleep_start();
  delay(20000);


}
