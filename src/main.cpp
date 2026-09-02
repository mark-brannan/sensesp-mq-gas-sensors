// SensESP MQ gas sensor scaffold.
//
// Reads a DHT22 for ambient temperature and humidity, publishes both to
// Signal K, and feeds them into an MQ-135 wrapper whose corrected CO2
// reading is published as well.

#include <memory>

#include <DHTesp.h>

#include "mq_135.h"
#include "sensesp.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

// Signal K zone this device sits in: environment.inside.<zone>.*
const char* kZone = "cabin";

// DHT22 data pin and read interval. The DHT22 cannot be read faster than
// every 2 s.
const uint8_t kDhtPin = 16;
const unsigned int kDhtReadInterval = 5000;

// MQ-135 analog pin and read interval.
const uint8_t kMq135Pin = 34;
const unsigned int kMq135ReadInterval = 5000;
// Sensor resistance in clean air, kOhm. Measure this for your own sensor.
const float kMq135R0kOhm = 76.63;

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

  // ---- DHT22: temperature and humidity ----

  auto dht = std::make_shared<DHTesp>();
  dht->setup(kDhtPin, DHTesp::DHT22);

  // Signal K wants Kelvin.
  auto temperature = std::make_shared<RepeatSensor<float>>(
      kDhtReadInterval, [dht]() { return dht->getTemperature() + 273.15; });

  // Signal K wants a ratio, not percent.
  auto humidity = std::make_shared<RepeatSensor<float>>(
      kDhtReadInterval, [dht]() { return dht->getHumidity() / 100.0; });

  auto temperature_sk_output = std::make_shared<SKOutput<float>>(
      String("environment.inside.") + kZone + ".temperature",
      "/Sensors/DHT22/Temperature/SK Path",
      std::make_shared<SKMetadata>("K", "Air temperature"));

  ConfigItem(temperature_sk_output)
      ->set_title("Temperature SK Output Path")
      ->set_description("Signal K path for the DHT22 air temperature")
      ->set_sort_order(100);

  temperature->connect_to(temperature_sk_output);

  auto humidity_sk_output = std::make_shared<SKOutput<float>>(
      String("environment.inside.") + kZone + ".relativeHumidity",
      "/Sensors/DHT22/Humidity/SK Path",
      std::make_shared<SKMetadata>("ratio", "Relative humidity"));

  ConfigItem(humidity_sk_output)
      ->set_title("Humidity SK Output Path")
      ->set_description("Signal K path for the DHT22 relative humidity")
      ->set_sort_order(200);

  humidity->connect_to(humidity_sk_output);

  // ---- MQ-135: CO2, corrected for ambient conditions ----

  auto mq135 = std::make_shared<MQ135Wrapper>(kMq135Pin, kMq135R0kOhm);

  // Feed the wrapper the latest ambient readings (back in C and percent).
  auto temperature_to_mq = std::make_shared<LambdaConsumer<float>>(
      [mq135](float kelvin) { mq135->set_temperature(kelvin - 273.15); });
  temperature->connect_to(temperature_to_mq);

  auto humidity_to_mq = std::make_shared<LambdaConsumer<float>>(
      [mq135](float ratio) { mq135->set_humidity(ratio * 100.0); });
  humidity->connect_to(humidity_to_mq);

  auto co2 = std::make_shared<RepeatSensor<float>>(
      kMq135ReadInterval, [mq135]() { return mq135->read_co2_ppm(); });

  // Note: Signal K has no standard key for CO2 under environment.inside;
  // this path is a local convention.
  auto co2_sk_output = std::make_shared<SKOutput<float>>(
      String("environment.inside.") + kZone + ".co2",
      "/Sensors/MQ135/CO2/SK Path",
      std::make_shared<SKMetadata>("ppm", "CO2 concentration"));

  ConfigItem(co2_sk_output)
      ->set_title("CO2 SK Output Path")
      ->set_description("Signal K path for the MQ-135 corrected CO2 reading")
      ->set_sort_order(300);

  co2->connect_to(co2_sk_output);

  // To avoid garbage collecting all shared pointers created in setup(),
  // loop from here.
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }
