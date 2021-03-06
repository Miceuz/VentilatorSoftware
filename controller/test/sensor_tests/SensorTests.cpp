/* Copyright 2020, RespiraWorks

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/*
 * module contributors: verityRF
 *
 * The purpose of this module is to provide unit testing for the sensors
 * controller module.  Module is to be run on an x86 host and is not to be run
 * on an Arduino platform.
 */

#include "SensorTests.h"
#include "ArduinoSim.h"
#include "sensors.h"
#include "gtest/gtest.h"

#include <cmath>
#include <iostream>
#include <stdbool.h>
#include <stdio.h>
#include <string>

// Maximum allowable delta between calculated sensor readings and the input
// pressure waveform [kPa]
static const float COMPARISON_TOLERANCE = 0.005f;

//@TODO: Finish writing more specific unit tests for this module

/*
 * @brief This method models the pressure to voltage transfer function of the
 * MPXV5004 series sensors. Caller must ensure the output buffer has the
 * correct minimum length.
 *
 * @param *pressureIn pointer to input pressure waveform to process
 * @param *voltageOut pointer to the output voltage buffer
 * @param count is the length of the input buffer
 */
static void MPXV5004_TransferFn(float *pressureIn, float *voltageOut,
                                int count) {
  if (pressureIn == NULL || voltageOut == NULL) {
    throw "Pressure or voltage pointer is null";
  }
  for (int i = 0; i < count; i++) {
    voltageOut[i] = 5 * (0.2f * pressureIn[i] + 0.2f);
  }
}

/*
 * @brief This method models the pressure to voltage transfer function of the
 * MPXV7002 series sensors. Caller must ensure the output buffer has the
 * correct minimum length.
 *
 * @param *pressureIn pointer to input pressure waveform to process
 * @param *voltageOut pointer to the output voltage buffer
 * @param count is the length of the input buffer
 */
static void MPXV7002_TransferFn(float *pressureIn, float *voltageOut,
                                int count) {
  if (pressureIn == NULL || voltageOut == NULL) {
    throw "Pressure or voltage pointer is null";
  }
  for (int i = 0; i < count; i++) {
    voltageOut[i] = 5 * (0.2f * pressureIn[i] + 0.5f);
  }
}

// TODO(verityRF): Fix and enable this test (currently it fails).
TEST(SensorTests, DISABLED_FullScaleReading) {
  // These pressure waveforms start at 0 kPa to simulate the system being in the
  // proper calibration state then they go over the sensor full ranges The first
  // value is repeated four times, and each subsequent value twice, so that the
  // test neatly corresponds to the 4 and 2 default average sample counts that
  // the sensor module defaults to.
  //
  // Value sare in kPa.
  float differentialFlowPressures[] = {
      0.0f,  0.0f, 0.0f, 0.0f, -2.0f, -2.0f, -1.5f, -1.5f, -1.0f, -1.0f, -0.5f,
      -0.5f, 0,    0,    0.5f, 0.5f,  1.0f,  1.0f,  1.5f,  1.5f,  2.0f,  2.0f};
  float patientPressures[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.5f,  0.5f, 1.0f,
                              1.0f, 1.5f, 1.5f, 2.0f, 2.0f,  2.5f, 2.5f,
                              3.0f, 3.0f, 3.5f, 3.5f, 3.92f, 3.92f};
  // length of the differentialFlowPressures array
  const int NUM_DIFF_ELEMENTS = 22;
  // length of the patientPressures Array
  const int NUM_PATIENT_ELEMENTS = 20;
  float differentialFlowSensorVoltages[NUM_DIFF_ELEMENTS]; //[V]
  float patientSensorVoltages[NUM_PATIENT_ELEMENTS];       //[V]

  // Convert these pressure waveforms into the voltage equivalents using the
  // appropriate sensor transfer functions
  MPXV7002_TransferFn(differentialFlowPressures, differentialFlowSensorVoltages,
                      NUM_DIFF_ELEMENTS);
  MPXV5004_TransferFn(patientPressures, patientSensorVoltages,
                      NUM_PATIENT_ELEMENTS);

  // Will pad the rest of the simulated analog signals with ambient pressure
  // readings (0 kPa) voltage equivalents
  float ambientPressure = 0;                    //[kPa]
  float differentialFlowSensorVoltage_0kPa = 0; //[V]
  float patientFlowSensorVoltage_0kPa = 0;      //[V]

  MPXV7002_TransferFn(&ambientPressure, &differentialFlowSensorVoltage_0kPa, 1);
  MPXV5004_TransferFn(&ambientPressure, &patientFlowSensorVoltage_0kPa, 1);

  // First set the simulated analog signals to an ambient 0 kPa corresponding
  // voltage
  createStaticAnalogSignal((int)PressureSensors::INHALATION_PIN,
                           differentialFlowSensorVoltage_0kPa);
  createStaticAnalogSignal((int)PressureSensors::EXHALATION_PIN,
                           differentialFlowSensorVoltage_0kPa);
  createStaticAnalogSignal((int)PressureSensors::PATIENT_PIN,
                           patientFlowSensorVoltage_0kPa);

  // Overwrite the start of the simulated signal to the dynamic signal
  createDynamicAnalogSignal((int)PressureSensors::INHALATION_PIN,
                            differentialFlowSensorVoltages, NUM_DIFF_ELEMENTS);
  createDynamicAnalogSignal((int)PressureSensors::EXHALATION_PIN,
                            differentialFlowSensorVoltages, NUM_DIFF_ELEMENTS);
  createDynamicAnalogSignal((int)PressureSensors::PATIENT_PIN,
                            patientSensorVoltages, NUM_PATIENT_ELEMENTS);

  // Result is now that the dynamic signal is first, then followed by 0 kPa
  // readings

  sensors_init(); // the sensors are also calibrated

  // Now to compare the pressure readings the sensor module is calculating
  // versus what the original preassure waveform was
  for (int i = 0; i < 9; i++) {
    int index = 4 + 2 * i;
    float pressureInhalation =
        get_pressure_reading(PressureSensors::INHALATION_PIN);
    float pressureExhalation =
        get_pressure_reading(PressureSensors::EXHALATION_PIN);
    // Inhalation and exhalation should match because they are fed with the same
    // pressure waveform
    EXPECT_EQ(pressureInhalation, pressureExhalation)
        << "Differential Sensor Calculated Inhale/Exhale at index " << index;
    // Calculate deviance from expected. Using only inhalation because we know
    // it is equal to exhalation by now.
    EXPECT_NEAR(pressureInhalation, differentialFlowPressures[index],
                COMPARISON_TOLERANCE)
        << "Differential Sensor Calculated Value at index " << index;
  }

  for (int i = 0; i < 8; i++) {
    int index = 4 + 2 * i;
    float pressurePatient = get_pressure_reading(PressureSensors::PATIENT_PIN);
    EXPECT_NEAR(pressurePatient, patientPressures[index], COMPARISON_TOLERANCE)
        << "Patient Sensor at index" << index;
  }
}
