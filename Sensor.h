#ifndef _Sensor_
#define _Sensor_

#include <Arduino.h>
#include "FFT.h"

enum sensor{
    ZMPT1018,
    CWCS7600,
    NONE
};

template <typename T> class Sensor{
    public:
        Sensor(sensor type=sensor::NONE, uint8_t pin=A0, uint16_t sampleRate=5000, uint16_t bufferSize=256, uint8_t firOrder=32);
        ~Sensor();

        T adcValue();
        void readToBuffer();
        void readToBuffer(T *buffer);
        void readToBuffer(T *firBuffer, uint16_t firBufferSize, T *buffer);

        FFT_Result<T> fft();
        FFT_Result<T> fft(T *Real, T *Imag, uint16_t bufferSize, uint16_t sampleRate);
        void ifft();
        void ifft(T *Real, T *Imag, uint16_t bufferSize);

        T toVal(T sensorVal);
        void toVal(T *buffer, uint16_t bufferSize);

        T rms();
        T rms(T *buffer, uint16_t bufferSize);

        T calibrate(T rms);


        // variables
        bool DEBUG;

    private:
        inline T _sinc(T x);
        void _firwin();
        void _firwin(T *coef, uint16_t sampleRate, int numSteps);

        void _fir(T *in, T *out, T mean);
        void _fir(T *in, T*out, T mean, T*coef, uint16_t bufferSize, uint8_t firOrder);


        
        // variables
        uint8_t _pin, _firOrder;
        uint16_t _sampleRate, _samplePeriod, _bufferSize, _firBufferSize;
        T _sensorFactor;

        T *_FirCoef;
        T *_FirBuffer;
        T *_Real;
        T *_Imag;
};

#endif