#include "flower.h"
#include <cmath>

double Flower::computeCircleArea(double radius) {
    const double PI = 3.14159265358979323846;
    return PI * radius * radius;
}

double Flower::computeSphereVolume(double radius) {
    const double PI = 3.14159265358979323846;
    return (4.0 / 3.0) * PI * radius * radius * radius;
}

long long Flower::computeFibonacci(int n) {
    if (n <= 0) {
        return 0;
    }
    if (n == 1) {
        return 1;
    }

    long long prev = 0;
    long long curr = 1;

    for (int i = 2; i <= n; ++i) {
        long long next = prev + curr;
        prev           = curr;
        curr           = next;
    }

    return curr;
}