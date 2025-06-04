#include <MQUnifiedsensor.h>
#define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm  

// Note: 400 Offset for CO2 source: https://github.com/miguel5612/MQSensorsLib/issues/29
/*
  Motivation:
  We add 400 PPM for CO2 because when the library is calibrated it assumes the current state of the
  air as 0 PPM, and it is considered today that the CO2 present in the atmosphere is around 400 PPM.
  https://www.lavanguardia.com/natural/20190514/462242832581/concentracion-dioxido-cabono-co2-atmosfera-bate-record-historia-humanidad.html
*/
#define CO2_PPM_OFFSET 400


class MQ135Wrapper {
}
l

/*****************************  MQ CAlibration ********************************************/ 
// Explanation: 
// In this routine the sensor will measure the resistance of the sensor supposedly before being pre-heated
// and on clean air (Calibration conditions), setting up R0 value.
// We recomend executing this routine only on setup in laboratory conditions.
// This routine does not need to be executed on each restart, you can load your R0 value from eeprom.
// Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
void run_lab_calibration_for_MQ135(MQUnifiedsensor mq135) {
  Serial.print("Calibrating MQ-135 using %f as the assumed ratio for clean air; please wait.");
  const int num_calibrations = 10; // Number of calibrations to average
  float calcR0 = 0;
  for(int i = 1; i<=num_calibrations; i ++)
  {
    mq135.update(); // Update data, the arduino will read the voltage from the analog pin
    float r0 = mq135.calibrate(RatioMQ135CleanAir);
    Serial.print(".. R0=");
    Serial.print(r0);
    calcR0 += r0; // Add the R0 value to the total
  }
  mq135.setR0(calcR0/num_calibrations);
  Serial.println("...done.  Set R0=%f" + String(mq135.getR0(), 2) + " KOhms");
  
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}
}

void printMQ135Header() {
  Serial.println("** Values from MQ-135 ****");
  Serial.println("|    CO   |  Alcohol |   CO2  |  Toluen  |  NH4  |  Aceton  |");  
}

void printMQ135Values(MQUnifiedsensor MQ135) {
  MQ135.update(); // Update data, reading voltage from the analog pin

 // For each subsequent read, the Sensor will read PPM concentration using the model with a and b prior values
 
  // Configure the equation to calculate CO concentration value
  MQ135.setA(605.18);
  MQ135.setB(-3.937);
  float CO = MQ135.readSensor();

  MQ135.setA(77.255); MQ135.setB(-3.18); //Configure the equation to calculate Alcohol concentration value
  float Alcohol = MQ135.readSensor(); // SSensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(110.47); MQ135.setB(-2.862); // Configure the equation to calculate CO2 concentration value
  float CO2 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(44.947); MQ135.setB(-3.445); // Configure the equation to calculate Toluen concentration value
  float Toluen = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  
  MQ135.setA(102.2 ); MQ135.setB(-2.473); // Configure the equation to calculate NH4 concentration value
  float NH4 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(34.668); MQ135.setB(-3.369); // Configure the equation to calculate Aceton concentration value
  float Aceton = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  Serial.print("|   "); Serial.print(CO); 
  Serial.print("   |   "); Serial.print(Alcohol);
  Serial.print("   |   "); Serial.print(CO2 + CO2_PPM_OFFSET); // Add the offset for CO2 in normal air
  Serial.print("   |   "); Serial.print(Toluen); 
  Serial.print("   |   "); Serial.print(NH4); 
  Serial.print("   |   "); Serial.print(Aceton);
  Serial.println("   |"); 
  /*
    Exponential regression:
  GAS      | a      | b
  CO       | 605.18 | -3.937  
  Alcohol  | 77.255 | -3.18 
  CO2      | 110.47 | -2.862
  Toluen  | 44.947 | -3.445
  NH4      | 102.2  | -2.473
  Aceton  | 34.668 | -3.369
  */
}