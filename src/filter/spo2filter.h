#ifndef SPO2FILTER_H
#define SPO2FILTER_H

#include <QLinkedList>
#include "pulseoximterfilter.h"
#include "cstring"
#define SATURATION_STEADY_STATE 0
#define SATURATION_INCREASE 1
#define SATURATION_DECREASE 2
#define LENGTH_ARRAY_RATIO 11
class SpO2Filter : public PulseoximterFilter
{
public:
    SpO2Filter();
    double calculate(double time, double value);
    int getSaturationType();
    double getRatio(int saturationType);
    double getHearthRate();
    bool isReadyToRead();

protected:
    bool minFound = false;
    bool maxFound = false;
    bool isNoise = true;
    double SEC_THRESHOLD_DURATION = 0.1;
    double tmpTmin = 0, tmpTmax = 0, lastTmpTmin = 0;
    double ratio = 0;

    QLinkedList<double> x;
    QVector<double> y;
    QVector<double> t;
public:
    double Tmin[LENGTH_ARRAY_RATIO] = {0}, Tmax[LENGTH_ARRAY_RATIO] = {0};
    double Vmin[LENGTH_ARRAY_RATIO] = {0}, Vmax[LENGTH_ARRAY_RATIO] = {0};
};

#endif // SPO2FILTER_H
