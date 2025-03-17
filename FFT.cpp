#include "FFT.h"

template <typename T>
void FFT<T>::fft(T *real, T *imag, uint16_t samples, FFTDirection dir){
    // bit reversal
    uint16_t j = 0;
    for(int i=0; i<samples-1; i++){
        if(i < j){
            _swap(real[i], real[j]);
            if(dir == FFTDirection::Reverse)
                _swap(imag[i], imag[j]);
        }
        uint16_t k = samples >> 1;
        while(k <= j){
            j -= k;
            k >>= 1;
        }
        j += k;
    }

    // compute fft
    T c1 = -1.0;
    T c2 = 0.0;
    uint16_t l2 = 1;
    static uint8_t power = _exponent(samples);
    for(uint8_t l=0; l<power; l++){
        uint16_t l1 = l2;
        l2 <<= 1;
        T u1 = 1.0;
        T u2 = 0.0;
        for(j=0; j<l1; j++){
            for(uint16_t i=j; i<samples; i+=l2){
                uint16_t i1 = i + l1;
                T t1 = u1 * real[i1] - u2 * imag[i1];
                T t2 = u1 * imag[i1] + u2 * real[i1];
                real[i1] = real[i] - t1;
                imag[i1] = imag[i] - t2;
                real[i] += t1;
                imag[i] += t2;
            }
            T z = (u1 * c1) - (u2 * c2);
            u2 = (u1 * c2) + (u2 * c1);
            u1 = z;
        }
        T cTemp = 0.5 * c1;
        c1 = sqrt_internal(0.5 + cTemp);
        c2 = sqrt_internal(0.5 - cTemp);

        if(dir == FFTDirection::Forward) c2 = -c2;
    }
    if(dir == FFTDirection::Reverse){
        for(uint16_t i=0; i<samples; i++){
            real[i] /= samples;
            imag[i] /= samples;
        }
    }
}

template <typename T>
T FFT<T>::toMagnitude(T real, T imag){
    return sqrt_internal(sq(real) + sq(imag));
}
template <typename T>
void FFT<T>::toMagnitude(T *real, T *imag, uint16_t samples){
    uint16_t n = (samples >> 1) + 1;
    for(uint16_t i=0; i<n; i++){
        real[i] = sqrt_internal(sq(real[i]) + sq(imag[i]));
    }
    return;
}

template <typename T>
T FFT<T>::toAngle(T real, T imag){
    return atan2(imag , real);
}

template <typename T>
T FFT<T>::findPeak(T *data, uint16_t samples, uint16_t sampleRate){
    uint16_t N = (samples >> 1);

    // Find max index
    uint16_t maxIndex = 1;
    for(uint16_t i=1; i<N+1; i++){
        T curr = data[i];
        if(curr > data[i-1] && curr > data[i+1] && curr > data[maxIndex])
            maxIndex = i;
    }

    // Quadratic Interpolation
    T delta = 0.5 * ((data[maxIndex-1] - data[maxIndex+1]) / (data[maxIndex-1] - (2.0 * data[maxIndex]) + data[maxIndex+1]));
    T freq;
    // Improve calculation on edge value
    if(maxIndex == N){
        freq = (maxIndex + delta) * sampleRate / samples;
    }else{
        freq = (maxIndex + delta) * sampleRate / (samples - 1);
    }
    return freq;
}
template <typename T>
FFT_Result<T> FFT<T>::findPeak(T *real, T *imag, uint16_t samples, uint16_t sampleRate){
    // Find max magnitude index
    uint16_t maxIndex = 1;
    T maxPrev = 0;
    T maxCurr = 0;
    T maxNext = 0;
    T prev = 0;
    T curr = sqrt_internal(sq(real[0]) + sq(imag[0]));
    T next = sqrt_internal(sq(real[1]) + sq(imag[1]));
    uint16_t N = (samples >> 1);
    for(uint16_t i=1; i<N+1; i++){
        prev = curr;
        curr = next;
        next = sqrt_internal(sq(real[i+1]) + sq(imag[i+1]));
        if(curr > prev && curr > next && curr > maxCurr){
            maxIndex = i;
            maxPrev = prev;
            maxCurr = curr;
            maxNext = next;
        }
    }

    // Quadratic Interpolation
    T delta = 0.5 * ((maxPrev - maxNext) / (maxPrev - (2.0 * maxCurr) + maxNext));
    T freq;
    if(maxIndex == N){      // Improve calculation on edge value
        freq = (maxIndex + delta) * sampleRate / samples;
    }else{
        freq = (maxIndex + delta) * sampleRate / (samples - 1);
    }

    T phase = toAngle(real[maxIndex], imag[maxIndex]);

    return {freq, phase, maxIndex};
}

template <typename T>
void FFT<T>::shiftPhase(T *real, T *imag, uint16_t samples, T phaseShift){
    uint16_t half_N = samples >> 1;

    // First half of data
    // X = X * e^(-1j * theta)
    // X = (Real + jImag) * (cos(-theta) + jsin(-theta))
    // X = (Real * cos(-theta) - Imag * sin(-theta)) + j(Real * sin(-theta) + Imag * cos(-theta))
    T costheta = cos(-phaseShift);
    T sintheta = sin(-phaseShift);
    for(uint16_t i=0; i<half_N; i++){
        T tempReal = real[i] * costheta - imag[i] * sintheta;
        T tempImag = real[i] * sintheta + imag[i] * costheta;
        real[i] = tempReal;
        imag[i] = tempImag;
    }

    // Last half of data
    // X = X * e^(1j * theta)
    // X = (Real + jImag) * (cos(theta) + jsin(theta))
    // X = (Real * cos(theta) - Imag * sin(theta)) + j(Real * sin(theta) + Imag * cos(theta))
    sintheta = -sintheta;
    for(uint16_t i=half_N; i<samples; i++){
        T tempReal = real[i] * costheta - imag[i] * sintheta;
        T tempImag = real[i] * sintheta + imag[i] * costheta;
        real[i] = tempReal;
        imag[i] = tempImag;
    }

    return;
}

template <typename T>
void FFT<T>::_swap(T &a, T &b){
    T temp = a;
    a = b;
    b = temp;
    return;
}
template <typename T>
uint8_t FFT<T>::_exponent(uint16_t val){
    uint8_t res = 0;
    while(val >>= 1) res++;
    return res;
}

template class FFT<double>;
template class FFT<float>;