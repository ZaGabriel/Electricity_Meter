#include "GPS.h"

void GPS::decode(String s){
    if(s.substring(0, 6) != "$GPRMC") return;
    String data = s.substring(7);

    // Time
    String tmp = "";
    int i = 0;
    while(data[i] != ',') tmp += data[i++];
    this->time = tmp.substring(0, 2) + ':' + tmp.substring(2, 4) + ':' + tmp.substring(4, 6);

    // Valid
    tmp = "";
    i++;
    while(data[i] != ',') tmp += data[i++];
    this->valid = (tmp[0]) ? tmp[0] : '*';

    // Latitude
    tmp = "";
    i++;
    while(data[i] != ',') tmp += data[i++];
    this->latitude = tmp.toFloat();

    // North or South
    tmp = "";
    i++;
    while(data[i] != ',') tmp += data[i++];
    this->N_S = (tmp[0]) ? tmp[0] : '*';

    // Longitude
    tmp = "";
    i++;
    while(data[i] != ',') tmp += data[i++];
    this->longitude = tmp.toFloat();

    // East or West
    tmp = "";
    i++;
    while(data[i] != ',') tmp += data[i++];
    this->E_W = (tmp[0]) ? tmp[0] : '*';

    // Speed
    tmp = "";
    i++;
    while(data[i] != ',') tmp += data[i++];
    this->speed = tmp.toFloat();

    // Direction
    tmp = "";
    i++;
    while(data[i] != ',') tmp += data[i++];
    this->direction = tmp.toFloat();

    // Date
    tmp = "";
    i++;
    while(data[i] != ',') tmp += data[i++];
    this->date = "20" + tmp.substring(4, 6) + '/' + tmp.substring(2, 4) + '/' + tmp.substring(0, 2);

}
