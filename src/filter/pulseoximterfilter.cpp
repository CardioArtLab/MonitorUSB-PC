#include "pulseoximterfilter.h"

PulseoximterFilter::PulseoximterFilter()
{
    memset(ydata, 0, sizeof(ydata[0][0]) * 2 * 5);
}

double PulseoximterFilter::calculate(double time, double value)
{
    double *t = tdata;
    double *x= xdata;
    double *y= ydata[0];
    /**
     * SPO2
     * low pass 7Hz, Fs 1000Hz
     * denominator = [1 -1.95 0.95287]
     * numerator = [1 2 1]
     * high pass 0.5Hz, Fs 1000Hz
     * denominator = [1 -1.99686796747 0.9968769]
     * numerator = [1 -2 1]
     */
    for(int i=0; i<3; i++) {
        t[i] = t[i+1];
        x[i] = x[i+1];
        y[i] = y[i+1];
    }
    x[3] = value;
    t[3] = time;
    y[3] = x[3] + 3*x[2] + 3*x[1] +x[0] + 2.8744*y[2] - 2.7565*y[1] + 0.8819*y[0];

    value = 0.000025 * y[3];

    // peak detection algorithm
    return value;
}
