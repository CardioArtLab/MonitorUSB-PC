#include <QDebug>
#include "spo2filter.h"

SpO2Filter::SpO2Filter():PulseoximterFilter()
{
    for(int i=0; i<999; i++) {
        dc.append(0);
        y.append(0);
    }
}

double SpO2Filter::calculate(double time, double value)
{
    double *t = tdata;
    // store y data in linklist
    value = PulseoximterFilter::calculate(time, value);
    y.append(value);
    y.removeFirst();
    // CIC filter
    double newDc = 0.001 * (y.last() - y.front()) + dc.last();
    double Dc = dc.last();
    dc.append(newDc);
    QLinkedList<double>::const_iterator iterY = y.end();
    double Yprev = *(iterY-3), Y = *(iterY-2), Ynext = *(iterY-1);
    //qDebug("%f %f %f %f", Yprev, Y, Ynext, Dc);
    // peak detection algorithm
    // minimum peak detection
    if (Yprev >= Y && Ynext > Y) {
        //qDebug("min noise? %f", t[3] - tmpTmin);
        if (t[3] - tmpTmin > SEC_THRESHOLD_DURATION) {
            isNoise = false;
        } else {
            isNoise = true;
        }
        if (Dc >= Y && !isNoise && !maxFound) {
            //qDebug("found min");
            ROLLUP(Tmin, 3);
            ROLLUP(Vmin, 3);
            Tmin[3] = t[3];
            Vmin[3] = Y;
            minFound = true;
        }
        tmpTmin = t[3];
    }
    // maximum peak detection
    if (Y >= Yprev && Y > Ynext) {
        //qDebug("max noise? %f", t[3] - tmpTmin);
        if (t[3] - tmpTmax > SEC_THRESHOLD_DURATION) {
            isNoise = false;
        } else {
            isNoise = true;
        }
        if (Y >= Dc && !isNoise && minFound) {
            //qDebug("found max");
            ROLLUP(Tmax, 3);
            ROLLUP(Vmax, 3);
            Tmax[3] = t[3];
            Vmax[3] = Y;
            maxFound = true;
        }
        tmpTmax = t[3];
    }
    // R cal
    if (maxFound && minFound) {
        //qDebug("min (%f %f) max (%f %f)", Tmin[3], Vmin[3], Tmax[3], Vmax[3]);
        if (Vmax[3] - Vmin[3] > 0.1*rr) {
            // calculate for ratio of ratio
            rr = 0.3*rr + 0.7*(Vmax[3] - Vmin[3]);
            // corrected the Vmax Vmin value
        }
        minFound = false;
        maxFound = false;
    }
    return value;
}
