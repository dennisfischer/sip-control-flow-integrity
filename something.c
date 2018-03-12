#include <stdio.h>

#ifndef CONTROL_FLOW_INTEGRITY
#define CONTROL_FLOW_INTEGRITY __attribute__((annotate("control_flow_integrity")))
#endif

CONTROL_FLOW_INTEGRITY
void foobar() {
    printf("%d + %d = %d\n", 1, 1, 2);
    return;
}

void bar() {
    printf("%d + %d = %d\n", 7, 4, 11);
    foobar();
    return;
}

void foo(char a[]) {
    printf("%s\n", a);
    return;
}

int main() {
    printf("something\n");
    bar();
    foo("7");
    return 0;
}
