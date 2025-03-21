#ifndef _GPS_
#define _GPS_

#include <Arduino.h>

class GPS{
    public:
        char valid, N_S, E_W;
        String time, date;
        float latitude, longitude, speed, direction;

        void decode(String s);

};

#endif