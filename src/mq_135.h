#ifndef SENSESP_MQ_GAS_SENSORS_MQ_135_H_
#define SENSESP_MQ_GAS_SENSORS_MQ_135_H_

#include <MQUnifiedsensor.h>

#include "mq_corrections.h"

/**
 * @brief Minimal MQ-135 wrapper: reads the sensor resistance through
 * MQUnifiedsensor and converts it to CO2 ppm with the temperature and
 * humidity corrections in mq_corrections.h.
 *
 * R0 is the sensor resistance in clean air (about 400 ppm CO2), in kOhm.
 * It is board-specific and must be measured once after the sensor has
 * pre-heated for 24 h or more; no calibration routine is provided here.
 */
class MQ135Wrapper {
 public:
  MQ135Wrapper(uint8_t pin, float r0_kohm, float rl_kohm = 10.0)
      : mq_("ESP-32", 3.3, 12, pin, "MQ-135"), r0_kohm_(r0_kohm) {
    mq_.setRegressionMethod(1);
    mq_.setRL(rl_kohm);
    mq_.setR0(r0_kohm);
    mq_.init();
  }

  /// Ambient conditions used for the correction, degrees C and percent RH.
  void set_temperature(float celsius) { temperature_ = celsius; }
  void set_humidity(float percent) { humidity_ = percent; }

  /// Read the sensor and return corrected CO2 concentration in ppm.
  float read_co2_ppm() {
    mq_.update();
    // mq_corrections.h works in ohms and truncates to long.
    const long rs_ohm = static_cast<long>(mq_.getRS() * 1000.0);
    const long r0_ohm = static_cast<long>(r0_kohm_ * 1000.0);
    return getCorrectedPPM(rs_ohm, temperature_, humidity_, r0_ohm);
  }

 private:
  MQUnifiedsensor mq_;
  float r0_kohm_;
  float temperature_ = 20.0;
  float humidity_ = 50.0;
};

#endif  // SENSESP_MQ_GAS_SENSORS_MQ_135_H_
