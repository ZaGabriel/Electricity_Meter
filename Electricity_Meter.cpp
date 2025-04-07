#include <Wire.h>
#include <Tiny4kOLED.h>

#include "Sensor.h"

#define T double

#define SAMPLE_RATE 5000
#define BUFFER_SIZE 256
#define FIR_ORDER 32

using namespace std;

Sensor<T> voltage(sensor::ZMPT1018, A0, SAMPLE_RATE, BUFFER_SIZE, FIR_ORDER);
Sensor<T> current(sensor::CWCS7600, A1, SAMPLE_RATE, BUFFER_SIZE, FIR_ORDER);

void setup(){
    Serial.begin(115200);

    //Serial.println(voltage.calibrate(110));
    //Serial.println(current.calibrate(12));
    
    oled.begin(0, 0);
    oled.enableChargePump();
    oled.setRotation(1);
    oled.enableZoomIn();
    oled.setFont(FONT6X8);
    updateDisplay();
    oled.switchRenderFrame();
    updateDisplay();
    oled.switchFrame();
    oled.on();
}

void loop(){
    updateDisplay();
    oled.switchFrame();
    //delay(50);
}

void updateDisplay(){
  //  VOLTAGE
    voltage.readToBuffer();
    T vrms = voltage.toVal(voltage.rms());
    FFT_Result<T> vResult = voltage.fft();

    //  CURRENT
    current.readToBuffer();
    T irms = current.toVal(current.rms());
    FFT_Result<T> iResult = current.fft();
    
    //  POWER
    T phi = abs(vResult.phase - iResult.phase);

    T apparent_pow = vrms * irms;
    T pow_factor = cos(phi);
    T real_pow = apparent_pow * pow_factor;
    T reactive_pow = apparent_pow * sin(phi);
    

    Serial.print("Vrms:"); Serial.println(vrms);
    Serial.print("VrmsPeakFreq:"); Serial.println(vResult.freq);
    Serial.print("VrmsPhase: "); Serial.println(vResult.phase);
    Serial.print("Irms:"); Serial.println(irms);
    Serial.print("IrmsPeakFreq:"); Serial.println(iResult.freq);
    Serial.print("IrmsPhase:"); Serial.println(iResult.phase);
    Serial.print("ApparentPower:"); Serial.println(apparent_pow);
    Serial.print("RealPower:"); Serial.println(real_pow);
    Serial.print("ReactivePower:"); Serial.println(reactive_pow);
    Serial.print("PowerFactor:"); Serial.println(pow_factor);
    
    oled.clear();
    oled.setCursor(0, 0);
    oled.print(F("V: "));
    oled.print(vrms);
    oled.print(F("V  "));
    oled.print(vResult.freq);
    oled.print(F("Hz"));
    
    oled.setCursor(0, 1);
    oled.print(F("I: "));
    oled.print(irms);
    oled.print(F("A  "));
    oled.print(iResult.freq);
    oled.print(F("Hz"));

    oled.setCursor(0, 3);
    oled.print(F("P: "));
    oled.print(apparent_pow);
    oled.print("W  ");
    oled.print(real_pow);
    oled.print("W");
    
}