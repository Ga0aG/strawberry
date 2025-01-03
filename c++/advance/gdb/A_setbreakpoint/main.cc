// Reference: https://kuafu1994.github.io/GDB/GDB/SETBP.html

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
extern double computeSeriesValue(double x, int n);

int main(int argc, char * argv[]){
    double x;
    int n;
    x = atof(argv[1]);
    n = atoi(argv[2]);
    printf("This program is used to compute the value of the following series:\n");
    printf("(x^0)/0! + (x^1)/1! + (x^2)/2! + (x^3)/3! + (x^4)/4! + ...... + (x^n)/n!\n");
    printf("The value of the series for x=%f, n=%d is %f\n", x, n, computeSeriesValue(x, n));
    return 0;
}