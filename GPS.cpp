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
    this->latitude = tmp.substring(0, 2).toFloat() + tmp.substring(2).toFloat() / 60;

    // North or South
    tmp = "";
    i++;
    while(data[i] != ',') tmp += data[i++];
    this->N_S = (tmp[0]) ? tmp[0] : '*';
    this->latitude = (this->N_S == 'N') ? this->latitude : -this->latitude;
    

    // Longitude
    tmp = "";
    i++;
    while(data[i] != ',') tmp += data[i++];
    this->longitude = tmp.substring(0, 3).toFloat() + tmp.substring(3).toFloat() / 60;

    // East or West
    tmp = "";
    i++;
    while(data[i] != ',') tmp += data[i++];
    this->E_W = (tmp[0]) ? tmp[0] : '*';
    this->longitude = (this->E_W == 'E') ? this->longitude : -this->longitude;

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


    // Time Zone correction
    int time_zone = this->longitude / 15;
    int t = time.substring(0, 2).toInt() + time_zone;
    if(t > 24){
        t = t - 24;
        this->date = this->date.substring(0, 8) + String(this->date.substring(8).toInt() + 1);
    }
    String corrected_time = String(t);
    this->time = corrected_time + this->time.substring(2);
    

}
