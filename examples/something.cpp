#include <stdio.h>
#include "cfi.h"


namespace other {
    void foobar() {
        printf("other foobar: %d + %d = %d\n", 1, 1, 2);
    }
};


CONTROL_FLOW_INTEGRITY
void foobar() {
    printf("%d + %d = %d\n", 1, 1, 2);
}

void bar() {
    printf("%d + %d = %d\n", 7, 4, 11);
    foobar();
    other::foobar();
}

void foo(char a[]) {
    printf("%s\n", a);
}

int main() {
    printf("something\n");
    bar();
    foo("7");
    return 0;
}
