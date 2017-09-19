#ifndef BASEFILTER_H
#define BASEFILTER_H

inline void ROLLUP(double*X, int N) {
    for(int i=0;i<N;i++) X[i] = X[i+1];
}

inline void ROLLDOWN(double*X, int N) {
    for(int i=N;i>0;i--) X[i] = X[i-1];
}

class BaseFilter
{
public:
    virtual double calculate(double time, double rawData) = 0;
};

#endif // BASEFILTER_H
