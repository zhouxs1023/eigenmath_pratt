// This scanner uses the method of V. Pratt.
//
// The char pointers token_str and scan_str are pointers to the input string as
// in the following example.
//
//	| g | a | m | m | a |   | a | l | p | h | a |
//	  ^                   ^
//	  token_str           scan_str
//
// The char pointer token_buf points to a malloc buffer.
//
//	| g | a | m | m | a | \0 |
//	  ^
//	  token_buf

 # include "stdafx.h"
 # include "defs.h"

 # define T_INTEGER 1001
 # define T_DOUBLE 1002
 # define T_SYMBOL 1003
 # define T_FUNCTION 1004
 # define T_NEWLINE 1006
 # define T_STRING 1007
 # define T_GTEQ 1008
 # define T_LTEQ 1009
 # define T_EQ 1010

static int pptoken, ptoken, token, ppnewline_flag, pnewline_flag, newline_flag, meta_mode;
static char * input_str,  * scan_str,  * token_str,  * ptoken_buf,  * token_buf;

# define MIN_BP 0
# define MUL_BP 40
# define MAX_BP 10000

typedef void (*denotation)(void); // used for nud and led in the Pratt parser

struct token_info { // stores information about a token
	int token;
	int lbp;
	int nbp;
	denotation nud;
	denotation led;
};

// lbp and nbp are not used for nuds!
# define MAX_SYM 22
struct token_info sym_table[MAX_SYM] = { // stores the nuds and leds
	// token   , lbp   , nbp       , nud      , led
	{T_INTEGER , MIN_BP, MIN_BP    , &nud_atom, NULL    }, // nilfix
	{T_DOUBLE  , MIN_BP, MIN_BP    , &nud_atom, NULL    }, // nilfix
	{T_STRING  , MIN_BP, MIN_BP    , &nud_atom, NULL    }, // nilfix
	{T_SYMBOL  , MIN_BP, MIN_BP    , &nud_atom, NULL    }, // nilfix
	{T_FUNCTION, MIN_BP, MIN_BP    , &nud_func, NULL    }, // nilfix
	{'('       , MIN_BP, MIN_BP    , &nud_tens, NULL    }, // nilfix
	{'='       , 10    , 10        , NULL     , &led_rel}, // infixN
	{T_EQ      , 20    , 20        , NULL     , &led_rel}, // infixN
	{T_GTEQ    , 20    , 20        , NULL     , &led_rel}, // infixN
	{T_LTEQ    , 20    , 20        , NULL     , &led_rel}, // infixN
	{'>'       , 20    , 20        , NULL     , &led_rel}, // infixN
	{'<'       , 20    , 20        , NULL     , &led_rel}, // infixN
	{'+'       , 30    , 31        , &nud_sign, &led_add}, // add, infixL rbp 30
	{'-'       , 30    , 31        , &nud_sign, &led_add}, // sub, infixL rbp 30
	{'*'       , MUL_BP, MUL_BP + 1, NULL     , &led_mul}, // mul, infixL rbp 40
	{'/'       , MUL_BP, MUL_BP + 1, NULL     , &led_mul}, // div, infixL rbp 40
	{'^'       , 50    , 51        , NULL     , &led_pow}, // pow, infixR rbp 49
	{'!'       , 60    , 61        , NULL     , &led_fac}, // suffix
	{'['       , 80    , 81        , NULL     , &led_arr}  // array access infixL
};

// Returns number of chars scanned and expr on stack.

// Returns zero when nothing left to scan.

int
scan(const char * s)
{
	meta_mode = 0;
	expanding++;
	input_str = (char * )s;
	scan_str = (char * )s;
	get_next_token();
	if (token == 0) {
		push(symbol(NIL));
		expanding--;
		return 0;
	}
	pratt(MIN_BP);
	expanding--;
	return (int)(token_str - input_str);
}

int
scan_meta(const char * s)
{
	meta_mode = 1;
	expanding++;
	input_str = (char * )s;
	scan_str = (char * )s;
	get_next_token();
	if (token == 0) {
		push(symbol(NIL));
		expanding--;
		return 0;
	}
	pratt(MIN_BP);
	expanding--;
	return (int)(token_str - input_str);
}

void
nud_error(void)
{
	scan_str = token_str; // better error display
	error("syntax error");
}

void
led_error(void)
{
	error("No led found!");
}

void
nud_atom(void)
{
	switch (ptoken) {
		case T_INTEGER :
			bignum_scan_integer(ptoken_buf);
			break;
		case T_DOUBLE :
			bignum_scan_float(ptoken_buf);
			break;
		case T_STRING :
			new_string(ptoken_buf);	
			break;
		case T_SYMBOL :
			if (meta_mode && strlen(ptoken_buf) == 1)
				switch (ptoken_buf[0]) {
				case 'a':
					push(symbol(METAA));
					break;
				case 'b':
					push(symbol(METAB));
					break;
				case 'x':
					push(symbol(METAX));
					break;
				default:
					push(usr_symbol(ptoken_buf));
					break;
				}
			else
				push(usr_symbol(ptoken_buf));
			break;			
	}
}

void
nud_func(void)
{
	int n = 1;
	U * p;
	p = usr_symbol(token_buf);
	push(p);
	get_next_token(); // left paren
	if (token != ')') {
		pratt(MIN_BP);
		n++;
		while (token == ',') {
			get_next_token(); // comma
			pratt(MIN_BP);
			n++;
		}
	}
	if (token != ')')
		error(") expected");
        get_next_token(); // right paren
	list(n);
}

void
nud_tens(void)
{
	int n;
	if (ptoken != '(')
		error("( expected");
	pratt(MIN_BP);
	if (token == ',') {
		n = 1;
		while (token == ',') {
			get_next_token();
			pratt(MIN_BP);
			n++;
		}
		build_tensor(n);
	}
	if (token != ')')
		error(") expected");
	get_next_token();
}

void
nud_sign(void)
{
	if (ppnewline_flag == 0 && (pptoken == '+' || pptoken == '-'))
		error("syntax error");
	if (ptoken == '+') {
		pratt(30); // get right on the stack
	} else if (ptoken == '-') {
		pratt(30);
		negate();
	}
}

void
led_rel(void)
{
    // left is on stack
    switch (ptoken) {
	case T_EQ:
		push_symbol(TESTEQ);
		swap();
		pratt(20);
		list(3);
		break;
	case T_GTEQ:
		push_symbol(TESTGE);
		swap();
		pratt(20);
		list(3);
		break;
	case T_LTEQ:
		push_symbol(TESTLE);
		swap();
		pratt(20);
		list(3);
		break;
	case '>':
		push_symbol(TESTGT);
		swap();
		pratt(20);
		list(3);
		break;
	case '<':
		push_symbol(TESTLT);
		swap();
		pratt(20);
		list(3);
		break;
	case '=':
        push_symbol(SETQ);
        swap();
        pratt(10);
		list(3);
		break;
	default:
		break;
	}
}

void
led_add(void)
{
	// rbp 30
	// left is on stack
	push_symbol(ADD);
	swap();
	if (ptoken == '+') {
		pratt(30);
	} else if (ptoken == '-') {
		pratt(30);
		negate();
	}
	list(3);
}

void
led_mul(void)
{
	int h;
	U *p1;
	
	p1 = pop(); // left
	
	h = tos;

	push(p1); // left is now on stack

	// discard integer 1
	if (tos > h && isrational(stack[tos - 1]) && equaln(stack[tos - 1], 1))
		pop();

	// put right on stack
	if (ptoken != '/') {
		pratt(MUL_BP);
	} else if (ptoken == '/') {
		pratt(MUL_BP);
		inverse();
	}
	
	// fold constants
	if (tos > h + 1 && isnum(stack[tos - 2]) && isnum(stack[tos - 1]))
		multiply();

	// discard integer 1
	if (tos > h && isrational(stack[tos - 1]) && equaln(stack[tos - 1], 1))
		pop();

	if (h == tos)
		push_integer(1);
	else if (tos - h > 1) {
		list(tos - h);
		push_symbol(MULTIPLY);
		swap();
		cons();
	}
}

void
led_pow(void)
{
	// left on stack
	if (ptoken == '^') {
		push_symbol(POWER);
		swap();
		pratt(49);
		list(3);
	}
}

void
led_fac(void)
{
	// left is on stack
	push_symbol(FACTORIAL);
	swap();
	list(2);
}

void
led_arr(void)
{
	int h = tos;
	// left on stack
	if (ptoken == '[') {
		push_symbol(INDEX);
		swap();
		pratt(MIN_BP);
		while (token == ',') {
			get_next_token();
			pratt(MIN_BP);
		}
		if (token != ']')
			error("] expected");
		get_next_token();
		list(tos - h + 1);
	}
}

void
error(const char * errmsg)
{
	printchar('\n');

	// try not to put question mark on orphan line
	while (input_str != scan_str) {
		if (( * input_str == '\n' ||  * input_str == '\r') && input_str + 1 == scan_str)
			break;
		printchar( * input_str++);
	}

	printstr(" ? ");
	while ( * input_str && ( * input_str != '\n' &&  * input_str != '\r'))
		printchar( * input_str++);

	printchar('\n');

	stop(errmsg);
}

// There are n expressions on the stack, possibly tensors.
//
// This function assembles the stack expressions into a single tensor.
//
// For example, at the top level of the expression ((a,b),(c,d)), the vectors
// (a,b) and (c,d) would be on the stack.

void
build_tensor(int n)
{
	// int i, j, k, ndim, nelem;

	int i;

	U ** s;

	save();

	s = stack + tos - n;

	p2 = alloc_tensor(n);
	p2->u.tensor->ndim = 1;
	p2->u.tensor->dim[0] = n;
	for (i = 0; i < n; i++)
		p2->u.tensor->elem[i] = s[i];

	tos -= n;

	push(p2);

	restore();
}

int
find_token(int token) {
	int i;
	for (i=0; i < MAX_SYM; i++)
		if (token == sym_table[i].token)
			break;
	return i;
}

int
has_nud(int token) {
	int i = find_token(token);	
	return (i < MAX_SYM && sym_table[i].nud != NULL);
}

int
has_led(int token) {
	int i = find_token(token);	
	return (i < MAX_SYM && sym_table[i].led != NULL);
}

int
get_led_lbp(int token) {
	int i = find_token(token);	
	if (i < MAX_SYM)
		return sym_table[i].lbp;
	else
		return MIN_BP;
}

int
get_led_nbp(int token) {
	int i = find_token(token);	
	if (i < MAX_SYM)
		return sym_table[i].nbp;
	else
		return MIN_BP;
}

denotation
get_nud(int token) {
	int i = find_token(token);	
	if (i < MAX_SYM && sym_table[i].nud != NULL)
		return sym_table[i].nud;
	else
		return &nud_error;
}

denotation
get_led(int token) {
	int i = find_token(token);	
	if (i < MAX_SYM && sym_table[i].led != NULL)
		return sym_table[i].led;
	else
		return &led_error;
}

void
pratt(int rbp)
{
	int nbp, lbp, newline_at_top_level, token_has_nud, token_has_led;
	denotation nud, led;
	
	nbp = MAX_BP;
	nud = get_nud(token);
	get_next_token();
	nud();
	while (true) {
		token_has_nud = has_nud(token);
		token_has_led = has_led(token);
		if (token_has_led) {
			lbp = get_led_lbp(token);
		} else if (token_has_nud) {
			lbp = MUL_BP; // implicit multiplication
		} else {
			lbp = MIN_BP;
		}
		if (!(rbp < lbp && lbp < nbp))
			break;
		newline_at_top_level = (newline_flag == 1); // Eigenmath lacks && (paren_depth == 0) && (brack_depth == 0);
		if ((!newline_at_top_level || !token_has_nud) && token_has_led) { // Mathematica lacks || !token_has_nud
			nbp = get_led_nbp(token);
			led = get_led(token);
			get_next_token();
			led();
		} else if (!newline_at_top_level && token_has_nud && !token_has_led) {
			nbp = MAX_BP;
			led = &led_mul; // implicit multiplication
			led();
		} else
			break;
	}
}

void
get_next_token()
{
	if (token_buf) {
		if (ptoken_buf)
			free(ptoken_buf);
		ptoken_buf = strdup(token_buf);
	}
	pptoken = ptoken;
	ptoken = token;
	ppnewline_flag = pnewline_flag;
	pnewline_flag = newline_flag;
	newline_flag = 0;
	while (1) {
		get_token();
		if (token != T_NEWLINE) {
			break;
		}
		newline_flag = 1;
	}
}

void
get_token(void)
{
	// skip spaces
	while (isspace( * scan_str)) {
		if ( * scan_str == '\n' ||  * scan_str == '\r') {
			token = T_NEWLINE;
			scan_str++;
			return;
		}
		scan_str++;
	}

	token_str = scan_str;

	// end of string?

	if ( * scan_str == 0) {
		token = 0;
		return;
	}

	// number?

	if (isdigit( * scan_str) ||  * scan_str == '.') {
		while (isdigit( * scan_str))
			scan_str++;
		if ( * scan_str == '.') {
			scan_str++;
			while (isdigit( * scan_str))
				scan_str++;
			if ( * scan_str == 'e' && (scan_str[1] == '+' || scan_str[1] == '-' || isdigit(scan_str[1]))) {
				scan_str += 2;
				while (isdigit( * scan_str))
					scan_str++;
			}
			token = T_DOUBLE;
		} else
			token = T_INTEGER;
		update_token_buf(token_str, scan_str);
		return;
	}

	// symbol?

	if (isalpha( * scan_str)) {
		while (isalnum( * scan_str))
			scan_str++;
		if ( * scan_str == '(')
			token = T_FUNCTION;
		else
			token = T_SYMBOL;
		update_token_buf(token_str, scan_str);
		return;
	}

	// string ?

	if ( * scan_str == '"') {
		scan_str++;
		while ( * scan_str != '"') {
			if ( * scan_str == 0 ||  * scan_str == '\n' ||  * scan_str == '\r')
				error("runaway string");
			scan_str++;
		}
		scan_str++;
		token = T_STRING;
		update_token_buf(token_str + 1, scan_str - 1);
		return;
	}

	// comment?

	if ( * scan_str == '#' || ( * scan_str == '-' && scan_str[1] == '-')) {
		while ( * scan_str &&  * scan_str != '\n' &&  * scan_str != '\r')
			scan_str++;
		if ( * scan_str)
			scan_str++;
		token = T_NEWLINE;
		return;
	}

	// relational operator?

	if ( * scan_str == '=' && scan_str[1] == '=') {
		scan_str += 2;
		token = T_EQ;
		return;
	}

	if ( * scan_str == '<' && scan_str[1] == '=') {
		scan_str += 2;
		token = T_LTEQ;
		return;
	}

	if ( * scan_str == '>' && scan_str[1] == '=') {
		scan_str += 2;
		token = T_GTEQ;
		return;
	}

	// single char token

	token =  * scan_str++;
}

void
update_token_buf(char * a, char * b)
{
	int n;

	if (token_buf) {
		free(token_buf);
	}

	n = (int)(b - a);

	token_buf = (char * )malloc(n + 1);

	if (token_buf == 0)
		stop("malloc failure");

	strncpy(token_buf, a, n);

	token_buf[n] = 0;
	
}

 # if SELFTEST

static const char * s[] = {


	"a\nb",
	"a\nb",
	
	"a+\n-b",
	"a+\n-b ? \nStop: syntax error",

	"a^^b",
	"a^^ ? b\nStop: syntax error",

	"(a+b",
	"(a+b ? \nStop: ) expected",

	"quote(1/(x*log(a*x)))", // test case A
	"1/(x*log(a*x))",

	"\"hello",
	"\"hello ? \nStop: runaway string",

	 # if 0 // does not work anymore because of "tensor expected" error

	// implicit mul cannot cross newline

	"a[b\nc]",
	"a[b\nc ? ]\nStop: ] expected",

	"a[b*\nc]",
	"a[b*c]",

	"a[b\n*c]",
	"a[b*c]",

	 # endif

	// make sure question mark can appear after newlines

	"a+\nb+\nc+",
	"a+\nb+\nc+ ? \nStop: syntax error",

	// this bug fixed in version 30 (used to give one result, 14)

	"2+2\n(3+3)",
	"4\n6",

	// plus and minus cannot cross newline

	"1\n-1",
	"1\n-1",

	"1\n+1",
	"1\n1",
};

void
test_scan(void)
{
	test(__FILE__, s, sizeof s / sizeof(char * ));
}

 # endif

//	Notes:
//
//	Formerly add() and multiply() were used to construct expressions but
//	this preevaluation caused problems.
//
//	For example, suppose A has the floating point value inf.
//
//	Before, the expression A/A resulted in 1 because the scanner would
//	divide the symbols.
//
//	After removing add() and multiply(), A/A results in nan which is the
//	correct result.
//
//	The functions negate() and inverse() are used but they do not cause
//	problems with preevaluation of symbols.
