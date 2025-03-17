#include "Voltage.h"

// ********************************************* INIT ********************************************************** //

template <typename T>
Voltage<T>::Voltage(uint8_t pin, uint16_t sampleRate, uint16_t bufferSize, uint8_t firOrder){
    this->_pin = pin;
    this->_sampleRate = sampleRate;     // Hz
    this->_samplePeriod = round(1000000 / sampleRate);      // us
    this->_bufferSize = bufferSize;
    this->_firOrder = firOrder;
    this->_firBufferSize = bufferSize + firOrder;

    this->_firBuffer = new T[this->_firBufferSize];
    this->_firCoef = new T[this->_firOrder + 1];
    this->_vReal = new T[this->_bufferSize];
    this->_vImag = new T[this->_bufferSize];
    _firwin();

    this->_sensorFactor = 2.5;  // At 110V
}
template <typename T>
Voltage<T>::~Voltage(){
    delete[] this->_firBuffer;
    delete[] this->_firCoef;
    delete[] this->_vReal;
    delete[] this->_vImag;
}

// ********************************************* READ ********************************************************** //

template <typename T>
T Voltage<T>::adcValue(){
    unsigned long ms = micros();
    T val = analogRead(this->_pin);
    while(micros() - ms < this->_samplePeriod);
    return val;
}

template <typename T>
void Voltage<T>::readToBuffer(){
    memset(this->_vImag, 0, this->_bufferSize * sizeof(T));
    readToBuffer(this->_firBuffer, this->_firBufferSize, this->_vReal);
    return;
}
template <typename T>
void Voltage<T>::readToBuffer(T *buffer){
    readToBuffer(this->_firBuffer, this->_firBufferSize, buffer);
    return;
}
template <typename T>
void Voltage<T>::readToBuffer(T *firBuffer, uint16_t firBufferSize, T *buffer){
    double sum = 0;
    for(int i=0; i<firBufferSize; i++){
        firBuffer[i] = adcValue();
        sum += firBuffer[i];
    }
    T mean = sum / firBufferSize;

    //for(int i=0; i<firBufferSize; i++) buffer[i] = firBuffer[i] - mean;
    
    // apply low-pass filter
    _fir(firBuffer, buffer, mean);

    return;
}

// ********************************************* FIR ********************************************************** //

template <typename T>
inline T Voltage<T>::_sinc(T x){
    T M_PI_X = M_PI * x;
    return (x == 0) ? static_cast<T> (1) : sin(M_PI_X) / M_PI_X;
}

template <typename T>
void Voltage<T>::_firwin(){
    _firwin(this->_firCoef, this->_sampleRate, this->_firOrder + 1);
    return;
}
template <typename T>
void Voltage<T>::_firwin(T *coef, uint16_t sampleRate, int numSteps){
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
void Voltage<T>::_fir(T *in, T *out, T mean){
    _fir(in, out, mean, this->_firCoef, this->_bufferSize, this->_firOrder);
    return;
}
template <typename T>
void Voltage<T>::_fir(T *in, T*out, T mean, T*coef, uint16_t bufferSize, uint8_t firOrder){
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
FFT_Result<T> Voltage<T>::fft(){
    return this->fft(this->_vReal, this->_vImag, this->_bufferSize, this->_sampleRate);
}
template <typename T>
FFT_Result<T> Voltage<T>::fft(T *vReal, T *vImag, uint16_t bufferSize, uint16_t sampleRate){
    // Perform Fast Foreier Transform
    FFT<T>::fft(vReal, vImag, bufferSize, FFTDirection::Forward);

    //Find peak freqency and phase
    FFT_Result<T> res;
    res = FFT<T>::findPeak(vReal, vImag, bufferSize, sampleRate);

    if(this->DEBUG){
        Serial.print("Peak Freq : ");
        Serial.print(res.freq);
        Serial.print("\tPhase : ");
        Serial.println(res.phase);
    }

    return res;
}

template <typename T>
void Voltage<T>::ifft(){
    return this->ifft(this->_vReal, this->_vImag, this->_bufferSize);
}
template <typename T>
void Voltage<T>::ifft(T *vReal, T *vImag, uint16_t bufferSize){
    // Perform Inverse Fast Foreier Transform
    FFT<T>::fft(vReal, vImag, bufferSize, FFTDirection::Reverse);
    return;
}

// ********************************************* Measure ********************************************************** //

template <typename T>
T Voltage<T>::toV(T sensorVal){
    return sensorVal * this->_sensorFactor;
}
template <typename T>
void Voltage<T>::toV(T *buffer, uint16_t bufferSize){
    for(uint16_t i=0; i<bufferSize; i++)
        buffer[i] *= this->_sensorFactor;
    return;
}

template <typename T>
T Voltage<T>::vrms(){
    return vrms(this->_vReal, this->_bufferSize);
}
template <typename T>
T Voltage<T>::vrms(T *buffer, uint16_t bufferSize){
    double sum = 0;
    for(uint16_t i=0; i<bufferSize; i++)
        sum += sq(buffer[i]);
    
    T res = toV(sqrt(sum / bufferSize));

    if(this->DEBUG){
        Serial.print("Vrms : ");
        Serial.println(res);
    }
    return res;
}

template <typename T>
void Voltage<T>::calibrate(T vrms){
    readToBuffer();
    double sum = 0;
    for(uint16_t i=0; i<this->bufferSize; i++)
        sum += sq(buffer[i]);
    T rms = sqrt(sum / this->bufferSize);

    this->_sensorFactor = vrms / rms;

    if(this->DEBUG){
        Serial.print("Voltage Sensor Factor : ");
        Serial.println(this->_sensorFactor);
    }
    return;
}

template class Voltage<double>;
template class Voltage<float>;