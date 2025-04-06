#include <Wire.h>
#include <Tiny4kOLED.h>

#include "Sensor.h"
#include "GPS.h"

#define T double

#define SAMPLE_RATE 5000
#define BUFFER_SIZE 256
#define FIR_ORDER 32

using namespace std;

Sensor<T> voltage(sensor::ZMPT1018, A0, SAMPLE_RATE, BUFFER_SIZE, FIR_ORDER);
Sensor<T> current(sensor::CWCS7600, A1, SAMPLE_RATE, BUFFER_SIZE, FIR_ORDER);
GPS gps;

void setup(){
    Serial.begin(115200);
    
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
    delay(50);
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
    T real_pow = vrms * vrms * cos(phi);
    T reactive_pow = vrms * vrms * sin(phi);
    T pow_factor = cos(phi);

    
    // GPS
    String s = "";
    while(Serial.available()){
        char c = Serial.read();
        if(c == '\n'){
            if(s.substring(0, 6) == "$GPRMC"){
                gps.decode(s);
                /*
                Serial.println("Valid : " + gps.valid);
                Serial.print("Time : ");
                Serial.println(gps.date + " " + gps.time);
                Serial.print("Latitude : ");
                Serial.print(gps.latitude);
                Serial.println(' ' + gps.N_S);
                Serial.print("Longitude : ");
                Serial.print(gps.longitude);
                Serial.println(' ' + gps.E_W);
                */
                break;
            }
            s = "";
        }else{
            s += c;
        }
    }
  
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

    oled.setCursor(0, 2);
    oled.print(gps.date + " " + gps.time);
    oled.setCursor(0, 3);
    oled.print(gps.latitude);
    oled.print("  ");
    oled.print(gps.longitude);

    
}