// Signal K application template file.
//
// This application demonstrates core SensESP concepts in a very
// concise manner. You can build and upload the application as is
// and observe the value changes on the serial port monitor.
//
// You can use this source file as a basis for your own projects.
// Remove the parts that are not relevant to you, and add your own code
// for external hardware libraries.

#include <memory>

#include <MQ135.h>
#include <MQUnifiedsensor.h>
//#include "SH1106Wire.h"
#include <Wire.h>
//#include "DHTesp.h"
#include <DHT.h>
#include <DHT_U.h>
#include <Ticker.h>
#include "mq_corrections.h"

#include "sensesp.h"
#include "sensesp/sensors/analog_input.h"
#include "sensesp/sensors/digital_input.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

// DHT22 with data on pin 16
DHT dht(16, DHT22);
DHT_Unified dht_unified(16, DHT22); // not needed?

//MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

// The setup function performs one-time application initialization.
void setup() {
  SetupLogging(ESP_LOG_DEBUG);

  // Construct the global SensESPApp() object
  SensESPAppBuilder builder;
  sensesp_app = (&builder)
    // Set a custom hostname for the app.
    ->set_hostname("sensesp-mq-gas-sensors")
    // Optionally, hard-code the WiFi and Signal K server
    // settings. This is normally not needed.
    //->set_wifi_client("My WiFi SSID", "my_wifi_password")
    //->set_wifi_access_point("My AP SSID", "my_ap_password")
    //->set_sk_server("192.168.10.3", 80)
    ->get_app();

  sensor_t sensor;

  // mq2 analog data on 17
  // mq135 analog data on 17

  auto temperatureSensor = std::make_shared<RepeatSensor<float>>(
      1000, [&]() { return dht.readTemperature(); });

  auto humiditySensor = std::make_shared<RepeatSensor<float>>(
      1000, [&]() { return dht.readHumidity(); });

  // To avoid garbage collecting all shared pointers created in setup(),
  // loop from here.
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }
