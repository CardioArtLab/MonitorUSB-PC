#ifndef SPO2FILTER_H
#define SPO2FILTER_H

#include <QLinkedList>
#include "pulseoximterfilter.h"
#include "cstring"

class SpO2Filter : public PulseoximterFilter
{
public:
    SpO2Filter();
    double calculate(double time, double value);

protected:
    bool minFound = false;
    bool maxFound = false;
    bool isNoise = true;
    double SEC_THRESHOLD_DURATION = 0.01;
    double tmpTmin = 0, tmpTmax = 0;
    double Tmin[4] = {0}, Tmax[4] = {0};
    double Vmin[4] = {0}, Vmax[4] = {0};
    double rr = 0;

    QLinkedList<double> dc;
    QLinkedList<double> y;

    PulseoximterFilter redLED;
    PulseoximterFilter irLED;
};

#endif // SPO2FILTER_H
