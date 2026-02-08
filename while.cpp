#include "stdafx.h"
#include "defs.h"

void
eval_while(void)
{
	// Extract condition and body from p1

	// Save condition expression
	p2 = cadr(p1);

	// Skip condition, p1 now points to loop body list
	p1 = cddr(p1);

	// Save loop body list copy for each iteration
	p3 = p1;

	for (;;) {
		// Evaluate condition
		push(p2);
		eval();
		p4 = pop();

		// Exit if condition is zero
		if (iszero(p4))
			break;

		// Execute each statement in loop body
		p1 = p3;
		while (iscons(p1)) {
			push(car(p1));
			eval();
			pop();
			p1 = cdr(p1);
		}
	}

	// while function returns nil
	push_symbol(NIL);
}

#if SELFTEST

static const char *s[] = {

	// Test 1: Basic while loop with counter
	"i=0",
	"",     // nil is not printed

	"while(i<3, i=i+1)",
	"",     // nil is not printed

	"i",
	"3",

	// Test 2: While loop that doesn't execute
	"j=0",
	"",     // nil is not printed

	"while(j>0, j=j+1)",
	"",     // nil is not printed

	"j",
	"0",

	// Test 3: Simple sum (use 'total' to avoid conflict with built-in sum function)
	"k=1",
	"",     // nil is not printed

	"total=0",
	"",     // nil is not printed

	"while(k<=3, total=total+k, k=k+1)",
	"",     // nil is not printed

	"total",
	"6",

	// Test 4: Fibonacci-like sequence (y should be 8 after 4 iterations)
	"x=1",
	"",     // nil is not printed

	"y=1",
	"",     // nil is not printed

	"while(x<4, z=x+y, x=y, y=z)",
	"",     // nil is not printed

	"y",
	"8",

	// Test 5: Fibonacci sequence stopping at y=5
	"a=1",
	"",     // nil is not printed

	"b=1",
	"",     // nil is not printed

	"n=0",
	"",     // nil is not printed

	"while(n<3, n=n+1, c=a+b, a=b, b=c)",
	"",     // nil is not printed

	"b",
	"5",
};

void
test_while(void)
{
	test(__FILE__, s, sizeof s / sizeof (char *));
}

#endif
