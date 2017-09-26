#include <QDebug>
#include "spo2filter.h"

SpO2Filter::SpO2Filter():PulseoximterFilter()
{
    for(int i=0; i<99; i++) {
        x.append(0);
    }
    for(int i=0; i<600;i++) {
        y.append(0);
        t.append(0);
    }
}

double SpO2Filter::calculate(double time, double value)
{
    // store y data in linklist
    value = PulseoximterFilter::calculate(time, value);
    x.append(value);
    x.removeFirst();
    // CIC filter
    double newY = 0.01 * (x.last() - x.front()) + y.last();
    // add new value to vector
    value = newY;
    y.append(newY);
    y.removeFirst();
    t.append(time);
    t.removeFirst();
    // calculate local maximum, minimum
    int mid = y.length()/2, last = y.length();
    double Y = y.at(mid);
    double T = t.at(mid);
    double prevYMin = y.first(), nextYMin = y.last();
    double prevYMax = y.first(), nextYMax = y.last();

    for (int i=1; i<mid; i++) {
        if (prevYMin > y[i]) prevYMin = y[i];
        if (prevYMax < y[i]) prevYMax = y[i];
    }
    for (int i=mid+1; i<last; i++) {
        if (nextYMin > y[i]) nextYMin = y[i];
    }
    for (int i=mid+1; i<mid+100 && i<last; i++) {
        if (nextYMax < y[i]) nextYMax = y[i];
    }

    // peak detection algorithm
    // minimum peak detection
    if (prevYMin >= Y && nextYMin > Y) {
        //qDebug("min noise? %f", t[2] - tmpTmin);
        if (T - tmpTmin > SEC_THRESHOLD_DURATION) {
            isNoise = false;
        } else {
            isNoise = true;
        }
        if (!isNoise && !maxFound) {
            //qDebug("found min");
            Tmin[LENGTH_ARRAY_RATIO - 1] = T;
            Vmin[LENGTH_ARRAY_RATIO - 1] = Y;
            minFound = true;
        }
        tmpTmin = T;
    }
    // maximum peak detection
    if (Y >= prevYMax && Y > nextYMax) {
        //qDebug("max noise? %f", t[2] - tmpTmax);
        if (T - tmpTmax > SEC_THRESHOLD_DURATION) {
            isNoise = false;
        } else {
            isNoise = true;
        }
        if (!isNoise && minFound) {
            Tmax[LENGTH_ARRAY_RATIO - 1] = T;
            Vmax[LENGTH_ARRAY_RATIO - 1] = Y;
            maxFound = true;
        }
        tmpTmax = T;
    }

    if (maxFound && minFound) {
        double Vrange = Vmax[LENGTH_ARRAY_RATIO - 1] - Vmin[LENGTH_ARRAY_RATIO - 1];
        if (Vrange > 0.01) {
            for (int i=0; i<LENGTH_ARRAY_RATIO - 1; i++) {
                Tmax[i] = Tmax[i+1];
                Vmax[i] = Vmax[i+1];
                Vmin[i] = Vmin[i+1];
                Tmin[i] = Tmin[i+1];
            }
        }
        lastTmpTmin = tmpTmin;
        minFound = false;
        maxFound = false;
    }
    if (minFound && !maxFound && tmpTmin - lastTmpTmin > 100 * SEC_THRESHOLD_DURATION) {
        lastTmpTmin = tmpTmin;
        minFound = false;
    }
    return value;
}

bool SpO2Filter::isReadyToRead()
{
    if (tmpTmin - Tmin[LENGTH_ARRAY_RATIO - 2] > 5) return false;
    for (int i=0; i<(LENGTH_ARRAY_RATIO - 2); i++) {
        if (Tmax[i] == 0) return false;
        if (Tmin[i] == 0) return false;
        if (Tmax[i+1] - Tmax[i] > 2) return false;
        if (Tmin[i+1] - Tmin[i] > 2) return false;
    }
    return true;
}

int SpO2Filter::getSaturationType()
{
    if (isReadyToRead()) {
        const int index = LENGTH_ARRAY_RATIO - 2;
        if (Vmax[index-2] - Vmax[index-1] >= 0.05 && Vmax[index-1] - Vmax[index] >= 0.05)
            return SATURATION_DECREASE;
        if (Vmin[index-1] - Vmin[index-2] >= 0.05 && Vmin[index] - Vmin[index-1] >= 0.05)
            return SATURATION_INCREASE;
    }
    return SATURATION_STEADY_STATE;
}

double SpO2Filter::getRatio(int saturationType)
{
    if (!isReadyToRead()) return 0;
    const int index = LENGTH_ARRAY_RATIO - 2;
    if (saturationType == SATURATION_INCREASE) {
        double correctedVmin = Vmax[index-2] + (Vmax[index-1] - Vmax[index-2])*(Tmin[index-1]-Tmax[index-2])/(Tmax[index-1]-Tmax[index-2]);
        return Vmin[index-1] / correctedVmin;
    }

    if (saturationType == SATURATION_DECREASE) {
        double correctedVmax = Vmin[index-1] + (Vmin[index-1] - Vmin[index])*(Tmin[index-1]-Tmax[index-1])/(Tmin[index]-Tmin[index-1]);
        return correctedVmax / Vmax[index-1];
    }

    // otherwise stready state case
    return Vmin[index-1] / Vmax[index-1];
}

double SpO2Filter::getHearthRate() {
    if (!isReadyToRead()) return 0;
    double secDuration = 0;
    for (int i=LENGTH_ARRAY_RATIO - 2; i>0; i--) {
        secDuration += (Tmax[i] - Tmax[i-1]);
    }
    return 60 * secDuration/(LENGTH_ARRAY_RATIO-2);
}
