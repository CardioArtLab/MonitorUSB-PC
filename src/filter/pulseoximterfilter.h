#ifndef PULSEOXIMTERFILTER_H
#define PULSEOXIMTERFILTER_H
#include "basefilter.h"
#include "cstring"

class PulseoximterFilter : public BaseFilter
{
public:
    PulseoximterFilter();
    double calculate(double time, double value);
protected:
    double tdata[10] = {0}, xdata[10] = {0}, ydata[2][5];
};

#endif // PULSEOXIMTERFILTER_H
