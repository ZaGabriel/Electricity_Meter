#ifndef _Voltage_
#define _Voltage_

#include <Arduino.h>
#include "FFT.h"

template <typename T> class Voltage{
    public:
        Voltage(uint8_t pin=A0, uint16_t sampleRate=5000, uint16_t bufferSize=256, uint8_t firOrder=32);
        ~Voltage();

        T adcValue();
        void readToBuffer();
        void readToBuffer(T *buffer);
        void readToBuffer(T *firBuffer, uint16_t firBufferSize, T *buffer);

        FFT_Result<T> fft();
        FFT_Result<T> fft(T *vReal, T *vImag, uint16_t bufferSize, uint16_t sampleRate);
        void ifft();
        void ifft(T *vReal, T *vImag, uint16_t bufferSize);

        T toV(T sensorVal);
        void toV(T *buffer, uint16_t bufferSize);

        T vrms();
        T vrms(T *buffer, uint16_t bufferSize);

        void calibrate(T vrms);


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

        T *_firCoef;
        T *_firBuffer;
        T *_vReal;
        T *_vImag;
};

#endif