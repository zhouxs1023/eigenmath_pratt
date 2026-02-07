#include "stdafx.h"
#include "defs.h"

void
eval_newton(void)
{
    double y,x1,x2,d;
    int n;         
	p2 = cadr(p1);
	push(p2);
	eval();
	p2=pop();
	p3=caddr(p1);
	push(p3);
	eval();
	p3=pop();
	p4=cadddr(p1);
	push(p4);
	eval();
	x2=pop_double();
	push(p2);
	push(p3);
    derivative();
    p5=pop();
    push_symbol(EVAL);
    push(p2);
    push(p3);
    push_double(x2);
    list(4);
    p6=pop();
    p8=cadddr(p6);
    push_symbol(EVAL);
    push(p5);
    push(p3);
    push_double(x2);;
    list(4);
    p7=pop();
    p9=cadddr(p7);
    for (n=1;n<=30;n++) {
      x1=x2;
      push(p6);
      p8->u.d=x1;
      eval();
      y=pop_double();
      push(p7);
      p9->u.d=x1;
      eval();
      d=pop_double();
      x2=x1-y/d;
      if (((x1-x2)<0.000001)&&((x1-x2)>-0.000001)) break;
}
    push_double(x2);
}

#if SELFTEST

static const char *s[] = {

	// Test Newton's method on simple quadratic equations

	"newton(x^2-4,x,3)",
	"2",

	"newton(x^2-4,x,-3)",
	"-2",

	"newton(x^2-9,x,5)",
	"3",

	"newton(x^2-9,x,-5)",
	"-3",

	// Test Newton's method on cubic equation

	"newton(x^3-8,x,3)",
	"2",

	"newton(x^3-1,x,2)",
	"1",

	// Test Newton's method on exponential equation

	"newton(exp(x)-2,x,1)",
	"0.693147",

	// Test Newton's method on trigonometric equation

	"newton(sin(x),x,0.5)",
	"0",

	"newton(cos(x)-0.5,x,1)",
	"1.0472",

	// Test Newton's method on polynomial with known root

	"newton(x^3-2*x^2+x-2,x,2)",
	"2",

	// Test Newton's method convergence with different initial guesses

	"newton(x^2-2,x,1)",
	"1.41421",

	"newton(x^2-2,x,-1)",
	"-1.41421",

	// Test Newton's method on rational function root

	"newton(1/x-2,x,1)",
	"0.5",

	// Test Newton's method on linear equation (should converge in one step)

	"newton(2*x-4,x,1)",
	"2",

	// Test Newton's method with function that has derivative zero at root

	"newton(x^3,x,0.1)",
	"0",
};

void
test_newton(void)
{
	test(__FILE__, s, sizeof s / sizeof (char *));
}

#endif
