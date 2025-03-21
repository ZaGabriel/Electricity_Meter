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

    voltage.DEBUG = false;
    Serial.print("Voltage Factor : ");
    Serial.println(voltage.calibrate(110));

    current.DEBUG = false;
    Serial.print("Current Factor : ");
    Serial.println(current.calibrate(10));
}

void loop(){
    //  VOLTAGE
    voltage.readToBuffer();
    T vrms = voltage.toVal(voltage.rms());
    FFT_Result<T> vResult = voltage.fft();
    
    Serial.print("Vrms : ");
    Serial.print(vrms);
    Serial.print("\t");
    Serial.print("Peak Freq: ");
    Serial.print(vResult.freq);
    Serial.print("\t");
    Serial.print("Phase : ");
    Serial.println(vResult.phase);


    //  CURRENT
    current.readToBuffer();
    T irms = current.toVal(current.rms());
    FFT_Result<T> iResult = current.fft();
    
    Serial.print("Irms : ");
    Serial.print(irms);
    Serial.print("\t");
    Serial.print("Peak Freq: ");
    Serial.print(iResult.freq);
    Serial.print("\t");
    Serial.print("Phase : ");
    Serial.println(iResult.phase);
    
    //  POWER
    T phi = abs(vResult.phase - iResult.phase);

    Serial.print("Apparent Power : ");
    Serial.print(vrms * irms);
    Serial.print("\t");
    Serial.print("Real Power : ");
    Serial.print(vrms * vrms * cos(phi));
    Serial.print("\t");
    Serial.print("Reactive Power : ");
    Serial.print(vrms * vrms * sin(phi));
    Serial.print("\t");
    Serial.print("Power Factor : ");
    Serial.println(cos(phi));

    /*
    // GPS
    // Need another UART channel to read
    
    String s = "";
    while(true){
        if(!Serial1.available()) continue;
        char c = Serial1.read();
        if(c == '\n'){
            if(s.substring(0, 6) == "$GPRMC"){
                Serial.println(s);
                gps.decode(s);
                Serial.print("Valid : "); Serial.println(gps.valid);
                Serial.println("Time : " + gps.date + " " + gps.time);
                Serial.print("Latitude : "); Serial.print(gps.latitude); Serial.println(gps.N_S);
                Serial.print("Longitude : "); Serial.print(gps.longitude); Serial.println(gps.E_W);
                break;
            }
            s = "";
        }else{
            s += c;
        }
    }
    */
   
    Serial.print("\n\n");

    delay(1000);
}