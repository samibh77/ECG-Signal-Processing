 /*
 * LIB.cpp
 *
 *  Created on: May 10, 2025
 *      Author: wiki
 */

#include <APP.h>
#include "SDW.h"
#define CHIP_SELECT_PIN 10

SDW sdw1;

input buzzer_enable;


SDW_SensorSpec raw_ecg_spec = {1,0,AnIn_2};
SDW_ActuatorSpec beep = {7000, 0, 1, FreqOut_1};
probe ecg_measure;
probe ecg_filtered;
probe ecg_filtered1;
probe ecg_sqr;
probe ecg_bpm;
probe ecg_period;

float ecg_measure_val;
float ecg_filtered_val;
float ecg_filtered_val1;
float ecg_sqr_val;
float ecg_bpm_val;
float ecg_period_val;




void setup()
{
  /* Create SDW instance */
  sdw1 = SDW(CHIP_SELECT_PIN);

  /* Set Sampling Frequency and Discretization Algorithm */
  sdw1.startDesign(250, Tustin);

  /*Set Monitoring Interface over SPI */
  sdw1.setMonitoringInterface(MonitoringOverSpi);

  /* Create Blocks */
    sensor raw_ecg = sensor(&raw_ecg_spec);
    actuator bip = actuator(&beep);
    butter lowPass = butter(5, 15, passLow);    //  butter lowPass = butter(3, 15, passLow); // 10 10
    butter highPass = butter(5, 5, passHigh);   // butter highPass = butter(3, 5, passHigh); // 5 8
    tf derivative = tf("[10 0]", "[0.0001 1]"); // tf derivative = tf("[10 0]", "[0.0001 1]"); // 25 0
    node Abs = node("[1]", "[1]", Abs1);
    butter Average = butter(5, 5, passLow); // butter Average = butter(2, 5, passLow);
    schmidt ecgToSqr = schmidt();
    input Vmax = input("Vmax", 100.0);
    input Vmin = input("Vmin",0.0);
    input LTH = input("LTH",60);
    input HTH = input("HTH",80);
    period periodMeasure = period(100, 0, 0.001);
    maxObserve MAX = maxObserve(7);

    buzzer_enable = input("control",1);
    node Multiplicator = node ("[1 1]","[1]",Mul);

    ecg_measure = probe("ecg_measure");
    ecg_filtered = probe("ecg_filtered");
    ecg_filtered1 = probe("ecg_filtered1");
    ecg_sqr = probe("ecg_sqr");
    ecg_bpm = probe("ecg_bpm");
    ecg_period = probe("ecg_period");



  /* Connect Blocks */




  raw_ecg.out(0) > lowPass;
  lowPass > highPass;
  highPass > derivative;
  derivative > Abs.in(0);
  Abs.out(0) > Average;


  Average > ecgToSqr.in(0);
  Vmax > ecgToSqr.in(1);
  Vmin > ecgToSqr.in(2);
  LTH > ecgToSqr.in(3);
  HTH > ecgToSqr.in(4);
  ecgToSqr.out(0) > periodMeasure;

  ecgToSqr.out(0) > Multiplicator.in(0);
  buzzer_enable > Multiplicator.in(1);
  Multiplicator.out(0) > bip.in(0);
  //ecgToSqr.out(0) > bip.in(0);
  periodMeasure > MAX;



  raw_ecg.out(0) > ecg_measure;
  Average > ecg_filtered;
  ecgToSqr.out(0) > ecg_sqr;
  periodMeasure > ecg_period;
  MAX > ecg_bpm;

  buzzer_enable.enableRemote();


  /* Finalyze and check Design Integrity */
  sdw1.stopDesign();

  /* Start Sampling */
  sdw1.startSampling();
}

void loop()
{
  sdw1.getProbes();

  ecg_measure_val = ecg_measure.getVal();
  ecg_filtered_val = ecg_filtered.getVal();
  ecg_bpm_val = ecg_bpm.getVal();

  //ecg_sqr_val = ecg_sqr.getVal();
  //ecg_period_val = ecg_period.getVal();
}


