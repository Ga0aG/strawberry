#include<stdio.h>
int computeFactorial(int number) {
    int fact = 0;
    for(int j = 1; j <= number; j++) {
        fact = fact *j;
    }
    return fact;
}
double computeSeriesValue(double x, int n) {
    double seriesValue = 0.0;
    double xpow = 1;
    for(int k = 0; k <= n; k++){
        seriesValue += xpow / computeFactorial(k);
        xpow = xpow * x;
    }
    return seriesValue;
}