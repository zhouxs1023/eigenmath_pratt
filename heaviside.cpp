//-----------------------------------------------------------------------------
//
//	Author : pbillet.pb@gmail.com
//
//	heaviside unit function
//
//	A number of things are done to produce a canonical form:
//	heaviside(-x)=1-heaviside(x)
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "defs.h"
static void yheaviside(void);
static int distmode = 0;

void
eval_heaviside(void)
{
	push(cadr(p1));
	eval();
	heaviside();
}

void
heaviside(void)
{
	save();
	yheaviside();
	restore();
}

#define X p1

void
yheaviside(void)
{
	
	X = pop();

	
	if (isdouble(p1)) {
		if (p1->u.d > 0) 
			{push_integer(1);
			 return;}
		else 
			if (p1->u.d == 0) 
				{push_rational(1, 2);
				 return;}
			else
				{push_integer(0);
				 return;}
	}

	if (isrational(p1)) {
		if (MSIGN(mmul(p1->u.q.a,p1->u.q.b)) == -1) 
			{push_integer(0);
			 return;}
		else 
			if (MZERO(mmul(p1->u.q.a,p1->u.q.b))) 
				{push_rational(1, 2);
				 return;}
			else
				{push_integer(1);
				 return;}
		
	}
	
	if (distmode == 1) {
		push_rational(1, 2);
		push(p1);
		sgn();
		push_integer(1);
		add();
		multiply();
		return;
	}
	
	if (isnegativeterm(X)) {
		push_symbol(HEAVISIDE);
		push(X);
		negate();
		list(2);
		push_integer(-1);
		multiply();
		push_integer(1);
		add();
		return;
	}

	if (car(p1) == symbol(POWER)) {
		push_symbol(HEAVISIDE);
		push(cadr(p1));
		list(2);
		return;
	}

	if (car(p1) == symbol(ABS)) {
		push_integer(1);
		return;
	}

	if (car(p1) == symbol(SGN)) {
		if (isnegativeterm(cadr(p1)))
				push_integer(0);
			else
				push_integer(1);
		return;
	}

	push_symbol(HEAVISIDE);
	push(X);
	list(2);
}

#if SELFTEST

static const char *s[] = {

	"heaviside(-3)",
	"0",
	
	"heaviside(0)",
	"1/2",
	
	"heaviside(3)",
	"1",

};

void
test_heaviside(void)
{
	test(__FILE__, s, sizeof s / sizeof (char *));
}

#endif
