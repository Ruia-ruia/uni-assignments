#include <stdio.h>
#include <stdlib.h>

int main() {
    double z[100];
    printf("%ld\n", &z[12] - &z[9]);
    return 0;
}
