#ifndef _FFT_
#define _FFT_

#include<Arduino.h>

#define sqrt_internal sqrt
#define sq(x) ((x) * (x))

enum class FFTDirection{
    Forward, 
    Reverse
};

template <typename T>
struct FFT_Result{
    T freq;
    T phase;
    uint16_t index;
};

template <typename T> 
class FFT{
    public:
        static void fft(T *real, T *imag, uint16_t samples, FFTDirection dir);

        static T toAngle(T real, T imag);
        static T toMagnitude(T real, T imag);
        static void toMagnitude(T *real, T *imag, uint16_t samples);
        
        static T findPeak(T *data, uint16_t samples, uint16_t sampleRate);
        static FFT_Result<T> findPeak(T *real, T *imag, uint16_t samples, uint16_t sampleRate);

        static void shiftPhase(T *real, T *imag, uint16_t samples, T phaseShift);

    private:
        static void _swap(T &a, T &b);
        static uint8_t _exponent(uint16_t val);
};

#endif
