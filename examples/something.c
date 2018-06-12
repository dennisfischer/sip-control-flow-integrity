#include <stdio.h>
#include "cfi.h"


CONTROL_FLOW_INTEGRITY
void foobar() {
	printf("%d + %d = %d\n", 1, 1, 2);
}

CONTROL_FLOW_INTEGRITY
void bar() {
	printf("%d + %d = %d\n", 7, 4, 11);
	foobar();
}

CONTROL_FLOW_INTEGRITY
void foo(char a[]) {
	printf("%s\n", a);
}

CONTROL_FLOW_INTEGRITY
int main() {
	printf("something\n");
	bar();
	foo("7");
	return 0;
}
