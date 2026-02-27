//-----------------------------------------------------------------------------
//
//	Author : pbillet.pb@gmail.com
//
//	tchebychevU polynomial U(x,n+1)=2*x*U(n,x)-U(x,n-1) U(x,0)=1,U(x,1)=2*x
//
//	Input:		tos-2		x	(can be a symbol or expr)
//
//				tos-1		n
//
//	Output:		Result on stack
//
//-----------------------------------------------------------------------------

#include "defs.h"
static void ytchebychevU(void);
static void ytchebychevU2(int);

void
eval_tchebychevU(void)
{
	push(cadr(p1));
	eval();
	push(caddr(p1));
	eval();
	tchebychevU();
}

void
tchebychevU(void)
{
	save();
	ytchebychevU();
	restore();
}

#define X p1
#define N p2
#define Y p3
#define Y1 p4
#define Y0 p5

static void
ytchebychevU(void)
{
	int n;

	N = pop();
	X = pop();

	push(N);
	n = pop_integer();

	if (n < 0) {
		push_symbol(TCHEBYCHEVU);
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
		ytchebychevU2(n-1);
	else {
		Y = X;			// do this when X is an expr
		X = symbol(SECRETX);
		ytchebychevU2(n-1);
		X = Y;
		push(symbol(SECRETX));
		push(X);
		subst();
		eval();
	}
}

static void
ytchebychevU2(int n)
{
	int i;

	push(X);
	push_integer(2);
	multiply();
	
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

	"tchebychevU(x,n)",
	"tchebychevU(x,n)",

	"tchebychevU(x,0)-1",
	"0",

	"tchebychevU(x,1)-2*x",
	"0",

	"tchebychevU(x,2)-(4*x^2-1)",
	"0",

	"tchebychevU(x,3)-(8*x^3-4*x)",
	"0",

	"tchebychevU(x,4)-(16*x^4-12*x^2+1)",
	"0",

	"tchebychevU(x,5)-(32*x^5-32*x^3+6*x)",
	"0",

	"tchebychevU(x,6)-(64*x^6-80*x^4+24*x^2-1)",
	"0",

	// Test with trigonometric identity: U_n(cos(theta)) = sin((n+1)*theta)/sin(theta)
	"tchebychevU(cos(theta),2)-sin(3*theta)/sin(theta)",
	"0",

	"tchebychevU(cos(theta),3)-sin(4*theta)/sin(theta)",
	"0",

	"tchebychevU(cos(theta),4)-sin(5*theta)/sin(theta)",
	"0",

	// Test with numeric values
	"tchebychevU(1,5)-6",
	"0",

	"tchebychevU(-1,4)+1",
	"0",

	"tchebychevU(-1,5)-6",
	"0",

	// Test with expressions
	"tchebychevU(x+y,2)-(4*(x+y)^2-1)",
	"0",

	// Test boundary conditions
	"tchebychevU(x,-1)",
	"tchebychevU(x,-1)",
};

void
test_tchebychevU(void)
{
	test(__FILE__, s, sizeof s / sizeof (char *));
}

#endif
