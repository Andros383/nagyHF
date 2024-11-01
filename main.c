#include <stdio.h>

#include "debugmalloc.h"

int asd(int b) {
    return b * 2;
}

int asd(int a, int b) {
    return a + b;
}

int main(void) {
    printf("%d", asd(5));
    return 0;
}