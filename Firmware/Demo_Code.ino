#include <TMC2209.h>
#include "AS5600.h"

// This example will not work on Arduino boards without HardwareSerial ports,
// such as the Uno, Nano, and Mini.
//
// See this reference for more details:
// https://www.arduino.cc/reference/en/language/functions/communication/serial/

HardwareSerial & serial_stream = Serial2;

const int RX_PIN = 10;
const int TX_PIN = 14;
const int AS5600_ADC_PIN = 3;

const long SERIAL_BAUD_RATE = 115200;
const int DELAY = 1000;
const int32_t VELOCITY = 40000;

// current values may need to be reduced to prevent overheating depending on
// specific motor and power supply voltage
const uint8_t RUN_CURRENT_PERCENT = 100;
const float MOTOR_STEP_ANGLE = 1.8;
const uint16_t MICROSTEPS = 16;


// Instantiate TMC2209

TMC2209 stepper_driver;
//AS5600 as5600;   // Use default Wire


void setup()
{
  // Init Serial
  Serial.begin(SERIAL_BAUD_RATE);
  stepper_driver.setup(serial_stream, SERIAL_BAUD_RATE, TMC2209::SERIAL_ADDRESS_0, RX_PIN, TX_PIN);
  //delay(DELAY);

  // Init I2C interface
  //Wire.begin(16, 17);

  // AS5600 Setup
  // as5600.begin(4);  // Set direction pin
  // as5600.setDirection(AS5600_CLOCK_WISE);  // Default, just be explicit
  // int b = as5600.isConnected();
  // Serial.print("Connect: ");
  // Serial.println(b);
  // Serial.println(as5600.getAddress());
  // pinMode(AS5600_ADC_PIN, INPUT);


  // Driver Initial Setup
  stepper_driver.setRunCurrent(RUN_CURRENT_PERCENT);
  stepper_driver.enableAutomaticCurrentScaling();
  stepper_driver.enableAutomaticGradientAdaptation();
  stepper_driver.enableCoolStep();
  stepper_driver.setHardwareEnablePin(15);
  stepper_driver.setMicrostepsPerStep(MICROSTEPS);
  stepper_driver.enable();
  Serial.println("Stepper Configuration Complete!");

  delay(2000);


  // StealthChop Automatic Tuning
  int step_per_rotation = 360 / MOTOR_STEP_ANGLE;
  int32_t microsteps_per_period = (6 * MICROSTEPS * step_per_rotation) / 0.715;
  delay(200); //AT#1
  stepper_driver.moveAtVelocity(microsteps_per_period); //AT#2



  //stepper_driver.moveAtVelocity(VELOCITY);

}

void loop()
{
  if (not stepper_driver.isSetupAndCommunicating())
  {
    Serial.println("Stepper driver not setup and communicating!");
    return;
  }

  bool hardware_disabled = stepper_driver.hardwareDisabled();
  TMC2209::Settings settings = stepper_driver.getSettings();
  TMC2209::Status status = stepper_driver.getStatus();

  if (hardware_disabled)
  {
    Serial.println("Stepper driver is hardware disabled!");
  }
  else if (not settings.software_enabled)
  {
    Serial.println("Stepper driver is software disabled!");
  }
  else if ((not status.standstill))
  {
    Serial.print("Shaft_Velocity: ");
    Serial.print(VELOCITY);
    
    //Serial.print("\tAngle_Read: ");
    //Serial.print(as5600.readAngle()); //for I2C interface

    int rawADC = analogRead(AS5600_ADC_PIN); // Raw ADC reading
    float voltage = (rawADC * 3.3) / 4095.0; // Convert to voltage (3.3V reference, 12-bit ADC)
  
    Serial.print("\tV_out: ");
    Serial.print(voltage, 3);

    //uint32_t interstep_duration = stepper_driver.getInterstepDuration();
    //Serial.print("which is equal to an interstep_duration of ");
    //Serial.println(interstep_duration);
  }
  else
  {
    Serial.println("Not moving, something is wrong!");
  }

  Serial.println();
  delay(2000);
}
