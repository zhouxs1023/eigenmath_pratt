//-----------------------------------------------------------------------------
//
//	Author : pbillet.pb@gmail.com
//
//	tchebychevT polynomial T(x,n+1)=2*x*T(n,x)-T(x,n-1) T(x,0)=1,T(x,1)=x
//
//	Input:		tos-2		x	(can be a symbol or expr)
//
//				tos-1		n
//
//	Output:		Result on stack
//
//-----------------------------------------------------------------------------

#include "defs.h"
static void ytchebychevT(void);
static void ytchebychevT2(int);

void
eval_tchebychevT(void)
{
	push(cadr(p1));
	eval();
	push(caddr(p1));
	eval();
	tchebychevT();
}

void
tchebychevT(void)
{
	save();
	ytchebychevT();
	restore();
}


#define X p1
#define N p2
#define Y p3
#define Y1 p4
#define Y0 p5

void
ytchebychevT(void)
{
	int n;

	N = pop();
	X = pop();

	push(N);
	n = pop_integer();

	if (n < 0) {
		push_symbol(TCHEBYCHEVT);
		push(X);
		push(N);
		list(3);
		return;
	}
	if (n == 0) {
		push_integer(1);
		return;
	}
	if (issymbol(X))
		ytchebychevT2(n-1);
	else {
		Y = X;			// do this when X is an expr
		X = symbol(SECRETX);
		ytchebychevT2(n-1);
		X = Y;
		push(symbol(SECRETX));
		push(X);
		subst();
		eval();
	}
}

static void
ytchebychevT2(int n)
{
	int i;

	push(X);
	push_integer(1);
	Y1 = pop();
	for (i = 0; i < n; i++) {
		Y0 = Y1;

		Y1 = pop();

		push(X);
		push(Y1);
		multiply();
		
		push_integer(2);
		multiply();

		push(Y0);

		subtract();

	}
}

#if SELFTEST

static const char *s[] = {

	"tchebychevT(x,n)",
	"tchebychevT(x,n)",

	"tchebychevT(x,0)-1",
	"0",

	"tchebychevT(x,1)-x",
	"0",

	"tchebychevT(x,2)-(2*x^2-1)",
	"0",

	"tchebychevT(x,3)-(4*x^3-3*x)",
	"0",

	"tchebychevT(x,4)-(8*x^4-8*x^2+1)",
	"0",

	"tchebychevT(x,5)-(16*x^5-20*x^3+5*x)",
	"0",

	"tchebychevT(x,6)-(32*x^6-48*x^4+18*x^2-1)",
	"0",

	// Test with trigonometric identity: T_n(cos(theta)) = cos(n*theta)
	"tchebychevT(cos(theta),3)-cos(3*theta)",
	"0",

	"tchebychevT(cos(theta),4)-cos(4*theta)",
	"0",

	"tchebychevT(cos(theta),5)-cos(5*theta)",
	"0",

	// Test with numeric values
	"tchebychevT(0,5)-5*cos(5*pi/2)",
	"0",

	"tchebychevT(1,7)-1",
	"0",

	"tchebychevT(-1,6)-1",
	"0",

	"tchebychevT(-1,7)+1",
	"0",

	// Test with expressions
	"tchebychevT(x+y,2)-(2*(x+y)^2-1)",
	"0",

	// Test boundary conditions
	"tchebychevT(x,-1)",
	"tchebychevT(x,-1)",
};

void
test_tchebychevT(void)
{
	test(__FILE__, s, sizeof s / sizeof (char *));
}

#endif
