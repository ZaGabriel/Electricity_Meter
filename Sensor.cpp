#include "Sensor.h"

// ********************************************* INIT ********************************************************** //

template <typename T>
Sensor<T>::Sensor(sensor type, uint8_t pin, uint16_t sampleRate, uint16_t bufferSize, uint8_t firOrder){
    this->_pin = pin;
    this->_sampleRate = sampleRate;     // Hz
    this->_samplePeriod = round(1000000 / sampleRate);      // us
    this->_bufferSize = bufferSize;
    this->_firOrder = firOrder;
    this->_firBufferSize = bufferSize + firOrder;

    this->_FirBuffer = new T[this->_firBufferSize];
    this->_FirCoef = new T[this->_firOrder + 1];
    this->_Real = new T[this->_bufferSize];
    this->_Imag = new T[this->_bufferSize];
    _firwin();

    if(type == sensor::ZMPT1018)
        this->_sensorFactor = 2.5;  // At 110 V
    else if(type == sensor::CWCS7600)
        this->_sensorFactor = 0.28;    // At 10 A
    else if(type == sensor::NONE)
        this->_sensorFactor = 1;
}
template <typename T>
Sensor<T>::~Sensor(){
    delete[] this->_FirBuffer;
    delete[] this->_FirCoef;
    delete[] this->_Real;
    delete[] this->_Imag;
}

// ********************************************* READ ********************************************************** //

template <typename T>
T Sensor<T>::adcValue(){
    unsigned long ms = micros();
    T val = analogRead(this->_pin);
    while(micros() - ms < this->_samplePeriod);
    return val;
}

template <typename T>
void Sensor<T>::readToBuffer(){
    readToBuffer(this->_FirBuffer, this->_firBufferSize, this->_Real);
    memset(this->_Imag, 0, this->_bufferSize * sizeof(T));
    return;
}
template <typename T>
void Sensor<T>::readToBuffer(T *buffer){
    readToBuffer(this->_FirBuffer, this->_firBufferSize, buffer);
    return;
}
template <typename T>
void Sensor<T>::readToBuffer(T *firBuffer, uint16_t firBufferSize, T *buffer){
    double sum = 0;
    for(int i=0; i<firBufferSize; i++){
        firBuffer[i] = adcValue();
        sum += firBuffer[i];
    }
    T mean = sum / firBufferSize;   // to remove DC bias
    
    // apply low-pass filter
    _fir(firBuffer, buffer, mean);

    return;
}

// ********************************************* FIR ********************************************************** //

template <typename T>
inline T Sensor<T>::_sinc(T x){
    T M_PI_X = M_PI * x;
    return (x == 0) ? static_cast<T> (1) : sin(M_PI_X) / M_PI_X;
}

template <typename T>
void Sensor<T>::_firwin(){
    _firwin(this->_FirCoef, this->_sampleRate, this->_firOrder + 1);
    return;
}
template <typename T>
void Sensor<T>::_firwin(T *coef, uint16_t sampleRate, int numSteps){
    // Function to find fir filter coefficients

    // Normalized cutoff freq=100Hz
    double f1 = 100.0 / sampleRate;
    // Half of order
    int M = (numSteps - 1) / 2;

    for(int i=0; i<numSteps; i++){
        double ideal = _sinc(2 * f1 * (i - M));
        // Haming Window
        double window = 0.54 - 0.46 * cos(2 * M_PI * i / numSteps);
        coef[i] = ideal * window;
    }

    double sum = 0;
    for(int i=0; i<numSteps; i++) sum += coef[i];
    for(int i=0; i<numSteps; i++) coef[i] /= sum;

    return;
}

template <typename T>
void Sensor<T>::_fir(T *in, T *out, T mean){
    _fir(in, out, mean, this->_FirCoef, this->_bufferSize, this->_firOrder);
    return;
}
template <typename T>
void Sensor<T>::_fir(T *in, T*out, T mean, T*coef, uint16_t bufferSize, uint8_t firOrder){
    // FIR Filter : y(n) = coef(n)*x(n) + coef(n-1)*x(n-1) + ... + coef(n-order)*x(n-order)
    // deprecate first "FILTER_ORDER" elements
    // "in" size: BUFFER_SIZE + FILTER_ORDER
    // "out" size: BUFFER_SIZE

    for(int i=0; i<bufferSize; i++){
        double temp = 0;
        for(int j=0; j<firOrder+1; j++){
            temp += coef[j] * (in[i - j + firOrder] - mean);
        }
        out[i] = temp;
    }

    return;
}

// ********************************************* FFT ********************************************************** //

template <typename T>
FFT_Result<T> Sensor<T>::fft(){
    return this->fft(this->_Real, this->_Imag, this->_bufferSize, this->_sampleRate);
}
template <typename T>
FFT_Result<T> Sensor<T>::fft(T *real, T *imag, uint16_t bufferSize, uint16_t sampleRate){
    // Perform Fast Foreier Transform
    FFT<T>::fft(real, imag, bufferSize, FFTDirection::Forward);

    //Find peak freqency and phase
    FFT_Result<T> res;
    res = FFT<T>::findPeak(real, imag, bufferSize, sampleRate);

    if(this->DEBUG){
        Serial.print("Peak Freq : ");
        Serial.print(res.freq);
        Serial.print("\tPhase : ");
        Serial.println(res.phase);
    }

    return res;
}

template <typename T>
void Sensor<T>::ifft(){
    return this->ifft(this->_Real, this->_Imag, this->_bufferSize);
}
template <typename T>
void Sensor<T>::ifft(T *real, T *imag, uint16_t bufferSize){
    // Perform Inverse Fast Foreier Transform
    FFT<T>::fft(real, imag, bufferSize, FFTDirection::Reverse);
    return;
}

// ********************************************* Measure ********************************************************** //

template <typename T>
T Sensor<T>::toVal(T sensorVal){
    return sensorVal * this->_sensorFactor;
}
template <typename T>
void Sensor<T>::toVal(T *buffer, uint16_t bufferSize){
    for(uint16_t i=0; i<bufferSize; i++)
        buffer[i] *= this->_sensorFactor;
    return;
}

template <typename T>
T Sensor<T>::rms(){
    return rms(this->_Real, this->_bufferSize);
}
template <typename T>
T Sensor<T>::rms(T *buffer, uint16_t bufferSize){
    double sum = 0;
    for(uint16_t i=0; i<bufferSize; i++)
        sum += sq(buffer[i]);
    
    T rms = sqrt(sum / bufferSize);

    if(this->DEBUG){
        Serial.print("rms : ");
        Serial.println(rms);
    }
    return rms;
}

template <typename T>
T Sensor<T>::calibrate(T vrms){
    readToBuffer();
    T rms = this->rms();

    this->_sensorFactor = vrms / rms;

    if(this->DEBUG){
        Serial.print("Sensor Factor : ");
        Serial.println(this->_sensorFactor);
    }
    return this->_sensorFactor;
}

template class Sensor<double>;
template class Sensor<float>;