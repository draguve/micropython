/* Project: Q-Number (Q16.16, signed) library
 * Author:  Richard James Howe
 * License: The Unlicense
 * Email:   howe.r.j.89@gmail.com
 * Repo:    <https://github.com/q> 
 *
 *
 * A Q32.32 version would be useful. 
 *
 * The following should be changed/done for this library:
 *
 * - Moving towards a header-only model.
 * - Removal of dependencies such as 'isalpha', 'tolower'
 *   as they are locale dependent.
 * - Make components optional (filters, expression parser, ...)
 * - Make hyperbolic arc sin/cos/tan functions.
 * - Fix bugs / inaccuracies in CORDIC code.
 * - Improve accuracy of all the functions and quantify error and
 *   their limits. 
 *
 * BUG: Enter: 2.71791, get 2.0625, 2.7179 works fine. (Need to
 * limit decimal places).
 */

#include "q.h"
#include <assert.h>
// #include <ctype.h>
#include <inttypes.h>
#include <limits.h>
#include "m_string.h"
#include <string.h>
#include "py/runtime.h"

#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"

#define UNUSED(X)               ((void)(X))
#define BOOLIFY(X)              (!!(X))
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
#define MULTIPLIER              (INT16_MAX)
#define DMIN                    (INT32_MIN)
#define DMAX                    (INT32_MAX)
// #define MIN(X, Y)               ((X) < (Y) ? (X) : (Y))
// #define MAX(X, Y)               ((X) < (Y) ? (Y) : (X))

#ifndef CONFIG_Q_HIDE_FUNCS /* 1 = hide hidden (testing) functions, 0 = enable them */
#define CONFIG_Q_HIDE_FUNCS (0)
#endif

typedef  int16_t hd_t; /* half Q width,      signed */
typedef uint64_t lu_t; /* double Q width,  unsigned */

const qinfo_t qinfo = {
	.whole      = QBITS,
	.fractional = QBITS,
	.zero = (u_t)0uL << QBITS,
	.bit  = 1uL,
	.one  = (u_t)1uL << QBITS,
	.min  = (u_t)(QHIGH << QBITS),
	.max  = (u_t)((QHIGH << QBITS) - 1uL),

	.pi    = QPI, /* 3.243F6 A8885 A308D 31319 8A2E0... */
	.e     = QMK(0x2, 0xB7E1, 16), /* 2.B7E1 5162 8A... */
	.sqrt2 = QMK(0x1, 0x6A09, 16), /* 1.6A09 E667 F3... */
	.sqrt3 = QMK(0x1, 0xBB67, 16), /* 1.BB67 AE85 84... */
	.ln2   = QMK(0x0, 0xB172, 16), /* 0.B172 17F7 D1... */
	.ln10  = QMK(0x2, 0x4D76, 16), /* 2.4D76 3776 AA... */

	// .version = QVERSION,s
};


// I forced this to be a constant and commented out the qbase function
const qconf_t qconf = { /* Global Configuration Options */
	.bound = qbound_donothing,
	.dp    = 4,
	.base  = 10,
};

/********* Basic Library Routines ********************************************/


static inline void implies(const int x, const int y) {
	assert(!x || y);
}

static inline void mutual(const int x, const int y) { /* mutual implication */
	assert(BOOLIFY(x) == BOOLIFY(y));
}

static inline void exclusive(const int x, const int y) {
	assert(BOOLIFY(x) != BOOLIFY(y));
}

static inline void static_assertions(void) {
	BUILD_BUG_ON(CHAR_BIT != 8);
	BUILD_BUG_ON((sizeof(q_t)*CHAR_BIT) != (QBITS * 2));
	BUILD_BUG_ON( sizeof(q_t) !=  sizeof(u_t));
	BUILD_BUG_ON( sizeof(u_t) !=  sizeof(d_t));
	BUILD_BUG_ON(sizeof(lu_t) !=  sizeof(ld_t));
	BUILD_BUG_ON(sizeof(d_t)  != (sizeof(hd_t) * 2));
	BUILD_BUG_ON(sizeof(lu_t) != (sizeof(u_t)  * 2));
}

q_t qbound_saturate(const ld_t s) { /**< default saturation handler */
	assert(s > DMAX || s < DMIN);
	if (s > DMAX) return DMAX;
	return DMIN;
}

q_t qbound_wrap(const ld_t s) { /**< wrap numbers on overflow */
	assert(s > DMAX || s < DMIN);
	if (s > DMAX) return DMIN + (s % DMAX);
	return DMAX - ((-s) % DMAX);
}

q_t qbound_donothing(const ld_t s){
	return (q_t) s;
}

static inline q_t qsat(const ld_t s) {
	static_assertions();
	if (s > DMAX || s < DMIN) return qconf.bound(s);
	return s;
}

inline d_t arshift(const d_t v, const unsigned p) {
	u_t vn = v;
	if (v >= 0l)
		return vn >> p;
	const u_t leading = ((u_t)(-1l)) << ((sizeof(v) * CHAR_BIT) - p - 1);
	return leading | (vn >> p);
}

inline d_t divn(const d_t v, const unsigned p) {
	/* return v / (1l << p); */
	const u_t shifted = ((u_t)v) >> p;
	if (qispositive(v))
		return shifted;
	const u_t leading = ((u_t)(-1l)) << ((sizeof(v)*CHAR_BIT) - p - 1);
	return leading | shifted;
}

/* These really all should be moved the header for efficiency reasons */
static inline u_t qhigh(const q_t q) { return ((u_t)q) >> QBITS; }
static inline u_t qlow(const q_t q)  { return ((u_t)q) & QMASK; }
static inline q_t qcons(const u_t hi, const u_t lo) { return (hi << QBITS) | (lo & QMASK); }

int qtoi(const q_t toi)                 { return ((lu_t)((ld_t)toi)) >> QBITS; }
q_t qint(const int toq)                 { return ((u_t)((d_t)toq)) << QBITS; }
signed char qtoc(const q_t q)           { return qtoi(q); }
q_t qchar(signed char c)                { return qint(c); }
short qtoh(const q_t q)                 { return qtoi(q); }
q_t qshort(short s)                     { return qint(s); }
long qtol(const q_t q)                  { return qtoi(q); }
q_t qlong(long l)                       { return qint(l); }
long long qtoll(const q_t q)            { return qtoi(q); }
q_t qvlong(long long ll)                { return qint(ll); }

q_t qisnegative(const q_t a)            { return QINT(BOOLIFY(qhigh(a) & QHIGH)); }
q_t qispositive(const q_t a)            { return QINT(!(qhigh(a) & QHIGH)); }
q_t qisinteger(const q_t a)             { return QINT(!qlow(a)); }
q_t qisodd(const q_t a)                 { return QINT(qisinteger(a) &&  (qhigh(a) & 1)); }
q_t qiseven(const q_t a)                { return QINT(qisinteger(a) && !(qhigh(a) & 1)); }
q_t qless(const q_t a, const q_t b)     { return QINT(a < b); }
q_t qeqless(const q_t a, const q_t b)   { return QINT(a <= b); }
q_t qmore(const q_t a, const q_t b)     { return QINT(a > b); }
q_t qeqmore(const q_t a, const q_t b)   { return QINT(a >= b); }
q_t qequal(const q_t a, const q_t b)    { return QINT(a == b); }
q_t qunequal(const q_t a, const q_t b)  { return QINT(a != b); }

q_t qnegate(const q_t a)                { return (~(u_t)a) + 1ULL; }
q_t qmin(const q_t a, const q_t b)      { return qless(a, b) ? a : b; }
q_t qmax(const q_t a, const q_t b)      { return qmore(a, b) ? a : b; }
q_t qabs(const q_t a)                   { return qisnegative(a) ? qnegate(a) : a; }
q_t qadd(const q_t a, const q_t b)      { return qsat((ld_t)a + (ld_t)b); }
q_t qsub(const q_t a, const q_t b)      { return qsat((ld_t)a - (ld_t)b); }
q_t qcopysign(const q_t a, const q_t b) { return qisnegative(b) ? qnegate(qabs(a)) : qabs(a); }
q_t qand(const q_t a, const q_t b)      { return a & b; }
q_t qxor(const q_t a, const q_t b)      { return a ^ b; }
q_t qor(const q_t a, const q_t b)       { return a | b; }
q_t qinvert(const q_t a)                { return ~a; }
q_t qnot(const q_t a)                   { return QINT(!a); }
q_t qlogical(const q_t a)               { return QINT(BOOLIFY(a)); }

q_t qlrs(const q_t a, const q_t b)      { /* assert low bits == 0? */ return (u_t)a >> (u_t)qtoi(b); }
q_t qlls(const q_t a, const q_t b)      { return (u_t)a << b; }
q_t qars(const q_t a, const q_t b)      { return arshift(a, qtoi(b)); }
q_t qals(const q_t a, const q_t b)      { return qsat((lu_t)a << b); }
q_t qsign(const q_t a)                  { return qisnegative(a) ? -QINT(1) : QINT(1); }
q_t qsignum(const q_t a)                { return a ? qsign(a) : QINT(0); }

q_t qrotr(const q_t x, const q_t b){
    uint n = (u_t)qtoi(b);
    return (x >> n % 32) | (x << (32-n) % 32);
}

q_t qrotl(const q_t x, const q_t b){
    uint n = (u_t)qtoi(b);
    return (x << n % 32) | (x >> (32-n) % 32);
}

q_t qapproxequal(const q_t a, const q_t b, const q_t epsilon) {
	assert(qeqmore(epsilon, qint(0))); 
	return QINT(qless(qabs(qsub(a, b)), epsilon)); 
}

q_t qapproxunequal(const q_t a, const q_t b, const q_t epsilon) { 
	return QINT(!qapproxequal(a, b, epsilon));
}

q_t qwithin(q_t v, q_t b1, q_t b2) {
	const q_t hi = qmax(b1, b2);
	const q_t lo = qmin(b1, b2);
	if (qequal(v, b1) || qequal(v, b2))
		return 1;
	return qless(v, hi) && qmore(v, lo) ? QINT(1) : QINT(0);
}

q_t qwithin_interval(q_t v, q_t expected, q_t allowance) {
	const q_t b1 = qadd(expected, allowance);
	const q_t b2 = qsub(expected, allowance);
	return qwithin(v, b1, b2);
}

q_t qfloor(const q_t q) {
	return q & ~QMASK;
}

q_t qceil(q_t q) {
	const q_t adj = qisinteger(q) ? QINT(0) : QINT(1);
	q = qadd(q, adj);
	return ((u_t)q) & (QMASK << QBITS);
}

q_t qtrunc(q_t q) {
	const q_t adj = qisnegative(q) && qlow(q) ? QINT(1) : QINT(0);
	q = qadd(q, adj);
	return ((u_t)q) & (QMASK << QBITS);
}

q_t qround(q_t q) {
	const int negative = qisnegative(q);
	q = qabs(q);
	const q_t adj = (qlow(q) & QHIGH) ? QINT(1) : QINT(0);
	q = qadd(q, adj);
	q = ((u_t)q) & (QMASK << QBITS);
	return negative ? qnegate(q) : q;
}

int qpack(const q_t *q, char *buffer, const size_t length) {
	assert(buffer);
	if (length < sizeof(*q))
		return -1;
	q_t qn = *q;
	uint8_t *b = (uint8_t*)buffer;
	for (size_t i = 0; i < sizeof(qn); i++) {
		b[i] = qn;
		qn = (u_t)qn >> CHAR_BIT;
	}
	return sizeof(qn);
}

int qunpack(q_t *q, const char *buffer, const size_t length) {
	assert(q);
	assert(buffer);
	if (length < sizeof(*q))
		return -1;
	uint8_t *b = (uint8_t*)buffer;
	u_t nq = 0;
	for (size_t i = 0; i < sizeof(*q); i++) {
		nq <<= CHAR_BIT;
		nq |= b[sizeof(*q)-i-1];
	}
	*q = nq;
	return sizeof(*q);
}

static inline ld_t multiply(const q_t a, const q_t b) {
	const ld_t dd = ((ld_t)a * (ld_t)b) + (lu_t)QHIGH;
	/* N.B. portable version of "dd >> QBITS", for double width signed values */
	return dd < 0 ? (-1ull << (2 * QBITS)) | ((lu_t)dd >> QBITS) : ((lu_t)dd) >> QBITS;
}

q_t qmul(const q_t a, const q_t b) {
	return qsat(multiply(a, b));
}

q_t qfma(const q_t a, const q_t b, const q_t c) {
	return qsat(multiply(a, b) + (ld_t)c);
}

q_t qdiv(const q_t a, const q_t b) {
	assert(b);
	const ld_t dd = ((ld_t)a) << QBITS;
	ld_t bd2 = divn(b, 1);
	if (!((dd >= 0 && b > 0) || (dd < 0 && b < 0)))
		bd2 = -bd2;
	/* Overflow not checked! */
	/*return (dd/b) + (bd2/b);*/
	return (dd + bd2) / b;
}

q_t qrem(const q_t a, const q_t b) {
	return qsub(a, qmul(qtrunc(qdiv(a, b)), b));
}

q_t qmod(q_t a, q_t b) {
	return qsub(a, qmul(qfloor(qdiv(a, b)), b));
}

static char itoch(const unsigned ch) {
	assert(ch < 36);
	if (ch <= 9)
		return ch + '0';
	return ch + 'A' - 10;
}

static inline void swap(char *a, char *b) {
	assert(a);
	assert(b);
	const int c = *a;
	*a = *b;
	*b = c;
}

static void reverse(char *s, const size_t length) {
	assert(s);
	for (size_t i = 0; i < length/2; i++)
		swap(&s[i], &s[length - i - 1]);
}

static int uprint(u_t p, char *s, const size_t length, const d_t base) {
	assert(s);
	assert(base >= 2 && base <= 36);
	if (length < 2)
		return -1;
	size_t i = 0;
	do {
		unsigned ch = p % base;
		p /= base;
		s[i++] = itoch(ch);
	} while (p && i < length);
	if (p && i >= length)
		return -1;
	reverse(s, i);
	return i;
}

/* <https://codereview.stackexchange.com/questions/109212> */
int qsprintbdp(q_t p, char *s, size_t length, const u_t base, const d_t idp) {
	assert(s);
	const int negative = BOOLIFY(qisnegative(p));
	if (negative)
		p = qnegate(p);
	const d_t hi = qhigh(p);
	char frac[QBITS + 2] = { '.', };
	memset(s, 0, length);
	assert(base >= 2 && base <= 36);
	u_t lo = qlow(p);
	size_t i = 1;
	for (i = 1; lo; i++) {
		if (idp >= 0 && (int)i > idp)
			break;
		lo *= base;
		assert(i < (QBITS + 2));
		frac[i] = itoch(lo >> QBITS);
		lo &= QMASK;
	}
	if (negative)
		s[0] = '-';
	const int hisz = uprint(hi, s + negative, length - (1 + negative), base);
	if (hisz < 0 || (hisz + i + negative + 1) > length)
		return -1;
	memcpy(s + hisz + negative, frac, i);
	return i + hisz;
}

int qsprintb(q_t p, char *s, size_t length, const u_t base) {
	return qsprintbdp(p, s, length, base, qconf.dp);
}

int qsprint(const q_t p, char *s, const size_t length) {
	return qsprintb(p, s, length, qconf.base); 
}


static inline int extract(unsigned char c, const int radix) {
	c = toLower(c);
	if (c >= '0' && c <= '9')
		c -= '0';
	else if (c >= 'a' && c <= 'z')
		c -= ('a' - 10);
	else
		return -1;
	if (c < radix)
		return c;
	return -1;
}

static inline q_t qmk(d_t integer, u_t fractional) {
	const int negative = integer < 0;
	integer = negative ? -integer : integer;
	const q_t r = qcons((d_t)integer, fractional);
	return negative ? qnegate(r) : r;
}

static inline u_t integer_logarithm(u_t num, const u_t base) {
	assert(num > 0 && base >= 2 && base <= 36);
	u_t r = -1;
	do r++; while (num /= base);
	return r;
}

int qnconvbdp(q_t *q, const char *s, size_t length, const d_t base, const u_t idp) {
	assert(q);
	assert(s);
	assert(base >= 2 && base <= 36);
	*q = QINT(0);
	if (length < 1)
		return -1;
	d_t hi = 0, lo = 0, places = 1, negative = 0, overflow = 0;
	size_t sidx = 0;

	if (s[sidx] == '-') {
		if (length < 2)
			return -1;
		negative = 1;
		sidx++;
	}

	for (; sidx < length && s[sidx]; sidx++) {
		const d_t e = extract(s[sidx], base);
		if (e < 0)
			break;
		if (hi > MULTIPLIER) { /* continue on with conversion, do not accumulate */
			overflow = 1;
		} else { 
			hi = (hi * base) + e;
		}
	}
	if (sidx >= length || !s[sidx])
		goto done;
	if (s[sidx] != '.')
		return -2;
	sidx++;
	
	const u_t ilog = integer_logarithm(0x10000, base);
	const u_t max = MIN(idp, ilog); /* Calculate maximum decimal places given base */

	for (u_t dp = 0; sidx < length && s[sidx]; sidx++, dp++) {
		const int ch = extract(s[sidx], base);
		if (ch < 0)
			return -3;
		if (dp < max) { /* continue on with conversion , do not accumulate */
			/* We could get more accuracy by looking at one digit
			 * passed the maximum digits allowed and rounding if
			 * that digit exists in the input. */
			lo = (lo * base) + ch;
			if (places >= (DMAX / base))
				return -4;
			places *= base;
		}
		assert((dp + 1) > dp);
	}
	if (!places)
		return -5;
	lo = ((d_t)((u_t)lo << QBITS) / places);
done:
	if (overflow) {
		*q = negative ? qinfo.min : qinfo.max;
		return -6;
	} else {
		const q_t nq = qmk(hi, lo);
		*q = negative ? qnegate(nq) : nq;

	}
	return 0;
}

int qnconvb(q_t *q, const char *s, size_t length, const d_t base) {
	return qnconvbdp(q, s, length, base, qconf.dp);
}

int qnconv(q_t *q, const char *s, size_t length) {
	return qnconvb(q, s, length, qconf.base);
}

int qconv(q_t *q, const char * const s) {
	assert(s);
	return qnconv(q, s, strlen(s));
}

int qconvb(q_t *q, const char * const s, const d_t base) {
	assert(s);
	return qnconvb(q, s, strlen(s), base);
}

typedef enum {
	CORDIC_MODE_VECTOR_E/* = 'VECT'*/,
	CORDIC_MODE_ROTATE_E/* = 'ROT'*/,
} cordic_mode_e;

typedef enum {
	CORDIC_COORD_HYPERBOLIC_E = -1,
	CORDIC_COORD_LINEAR_E     =  0,
	CORDIC_COORD_CIRCULAR_E   =  1,
} cordic_coordinates_e;

static const d_t cordic_circular_inverse_scaling   = 0x9B74; /* 1/scaling-factor */
static const d_t cordic_hyperbolic_inverse_scaling = 0x13520; /* 1/scaling-factor */

static inline int mulsign(d_t a, d_t b) { /* sign(a*b) */
	const int aneg = a < 0;
	const int bneg = b < 0;
	return aneg ^ bneg ? -QINT(1) : QINT(1);
}

/* Universal CORDIC <https://en.wikibooks.org/wiki/Digital_Circuits/CORDIC>
 *
 *	x(i+1) = x(i) - u.d(i).y(i).pow(2, -i)
 * 	y(i+1) = y(i) +   d(i).x(i).pow(2, -i)
 * 	z(i+1) = z(i) -   d(i).a(i)
 *
 *  d(i) =  sgn(z(i))      (rotation)
 *  d(i) = -sgn(x(i).y(i)) (vectoring)
 *
 *             hyperbolic      linear          circular
 *  u =                -1           0                 1
 *  a = atanh(pow(2, -i))  pow(2, -i)  atan(pow(2, -i))
 *
 *  linear shift sequence:      i = 0, 1, 2, 3, ...
 *  circular shift sequence:    i = 1, 2, 3, 4, ...
 *  hyperbolic shift sequence:  i = 1, 2, 3, 4, 4, 5, ... */
static int cordic(const cordic_coordinates_e coord, const cordic_mode_e mode, int iterations, d_t *x0, d_t *y0, d_t *z0) {
	assert(x0);
	assert(y0);
	assert(z0);
	if (mode != CORDIC_MODE_VECTOR_E && mode != CORDIC_MODE_ROTATE_E)
		return -1;

	BUILD_BUG_ON(sizeof(d_t) != sizeof(uint32_t));
	BUILD_BUG_ON(sizeof(u_t) != sizeof(uint32_t));

	static const u_t arctans[] = { /* atan(2^0), atan(2^-1), atan(2^-2), ... */
		0xC90FuL, 0x76B1uL, 0x3EB6uL, 0x1FD5uL,
		0x0FFAuL, 0x07FFuL, 0x03FFuL, 0x01FFuL,
		0x00FFuL, 0x007FuL, 0x003FuL, 0x001FuL,
		0x000FuL, 0x0007uL, 0x0003uL, 0x0001uL,
		0x0000uL, // 0x0000uL,
	};
	static const size_t arctans_length = sizeof arctans / sizeof arctans[0];

	static const u_t arctanhs[] = { /* atanh(2^-1), atanh(2^-2), ... */
		0x8c9fuL, 0x4162uL, 0x202buL, 0x1005uL,
		0x0800uL, 0x0400uL, 0x0200uL, 0x0100uL,
		0x0080uL, 0x0040uL, 0x0020uL, 0x0010uL,
		0x0008uL, 0x0004uL, 0x0002uL, 0x0001uL,
		0x0000uL, // 0x0000uL,
	};
	static const size_t arctanhs_length = sizeof arctanhs / sizeof arctanhs[0];

	static const u_t halfs[] = { /* 2^0, 2^-1, 2^-2, ..*/
		0x10000uL,
		0x8000uL, 0x4000uL, 0x2000uL, 0x1000uL,
		0x0800uL, 0x0400uL, 0x0200uL, 0x0100uL,
		0x0080uL, 0x0040uL, 0x0020uL, 0x0010uL,
		0x0008uL, 0x0004uL, 0x0002uL, 0x0001uL,
		//0x0000uL, // 0x0000uL,
	};
	static const size_t halfs_length = sizeof halfs / sizeof halfs[0];

	const u_t *lookup = NULL;
	size_t i = 0, j = 0, k = 0, length = 0;
	const size_t *shiftx = NULL, *shifty = NULL;
	int hyperbolic = 0;

	switch (coord) {
	case CORDIC_COORD_CIRCULAR_E:
		lookup = arctans;
		length = arctans_length;
		i = 0;
		shifty = &i;
		shiftx = &i;
		break;
	case CORDIC_COORD_HYPERBOLIC_E:
		lookup = arctanhs;
		length = arctanhs_length;
		hyperbolic = 1;
		i = 1;
		shifty = &i;
		shiftx = &i;
		break;
	case CORDIC_COORD_LINEAR_E:
		lookup = halfs;
		length = halfs_length;
		shifty = &j;
		shiftx = NULL;
		i = 1;
		break;
	default: /* not implemented */
		return -2;
	}

	iterations = iterations > (int)length ? (int)length : iterations;
	iterations = iterations < 0           ? (int)length : iterations;

	d_t x = *x0, y = *y0, z = *z0;

	/* rotation mode: z determines direction,
	 * vector mode:   y determines direction */
	for (; j < (unsigned)iterations; i++, j++) {
		again:
		{
			const d_t  m = mode == CORDIC_MODE_ROTATE_E ? z : -y /*-mulsign(x, y)*/;
			const d_t  d =   -!!(m < 0);
			const d_t xs = ((((shiftx ? divn(y, *shiftx) : 0)) ^ d) - d);
			const d_t ys =             (divn(x, *shifty)       ^ d) - d;
			const d_t xn = x - (hyperbolic ? -xs : xs);
			const d_t yn = y + ys;
			const d_t zn = z - ((lookup[j] ^ d) - d);
			x = xn; /* cosine, in circular, rotation mode */
			y = yn; /*   sine, in circular, rotation mode   */
			z = zn;
		}
		if (hyperbolic) { /* Experimental/Needs bug fixing */
			switch (1) { // TODO: Correct hyperbolic redo of iteration
			case 0: break;
			case 1: if (k++ >= 3) { k = 0; goto again; } break;
			case 2: {
				assert(j <= 120);
				size_t cmp = j + 1;
				if (cmp == 4 || cmp == 13 /*|| cmp == 40 || cmp == 121 || cmp == floor(pow(3,i-1)/2) */) {
					if (k) {
						k = 0;
					} else {
						k = 1;
						goto again;
					}
				}
				break;
			}
			}
		}
	}
	*x0 = x;
	*y0 = y;
	*z0 = z;

	return iterations;
}

/* See: - <https://dspguru.com/dsp/faqs/cordic/>
 *      - <https://en.wikipedia.org/wiki/CORDIC> */
static int qcordic(q_t theta, const int iterations, q_t *sine, q_t *cosine) {
	assert(sine);
	assert(cosine);

	static const q_t   pi =   QPI,    npi =  -QPI;
	static const q_t  hpi =   QPI/2, hnpi = -(QPI/2);
	static const q_t  qpi =   QPI/4, qnpi = -(QPI/4);
	static const q_t  dpi =   QPI*2, dnpi = -(QPI*2);

	/* Convert to range -pi   to pi, we could use qmod,
	 * however that uses multiplication and division, and
	 * if we can use those operators freely then there are
	 * other, better algorithms we can use instead of CORDIC
	 * for sine/cosine calculation. */
	while (qless(theta, npi)) theta = qadd(theta,  dpi);
	while (qmore(theta,  pi)) theta = qadd(theta, dnpi);

	int negate = 0, shift = 0;

	/* convert to range -pi/2 to pi/2 */
	if (qless(theta, hnpi)) {
		theta = qadd(theta,  pi);
		negate = 1;
	} else if (qmore(theta, hpi)) {
		theta = qadd(theta, npi);
		negate = 1;
	}

	/* convert to range -pi/4 to pi/4 */
	if (qless(theta, qnpi)) {
		theta = qadd(theta,  hpi);
		shift = -1;
	} else if (qmore(theta, qpi)) {
		theta = qadd(theta, hnpi);
		shift =  1;
	}

	d_t x = cordic_circular_inverse_scaling, y = 0, z = theta /* no theta scaling needed */;

	/* CORDIC in Q2.16 format */
	if (cordic(CORDIC_COORD_CIRCULAR_E, CORDIC_MODE_ROTATE_E, iterations, &x, &y, &z) < 0)
		return -1;

	/* undo shifting and quadrant changes */
	if (shift > 0) {
		const d_t yt = y;
		y =  x;
		x = -yt;
	} else if (shift < 0) {
		const d_t yt = y;
		y = -x;
		x =  yt;
	}

	if (negate) {
		x = -x;
		y = -y;
	}
	/* set output; no scaling needed */
	*cosine = x;
	  *sine = y;
	return 0;
}

q_t qatan(const q_t t) {
	q_t x = qint(1), y = t, z = QINT(0);
	cordic(CORDIC_COORD_CIRCULAR_E, CORDIC_MODE_VECTOR_E, -1, &x, &y, &z);
	return z;
}

q_t qatan2(const q_t a, const q_t b) {
	q_t x = b, y = a, z = QINT(0);
	if (qequal(b, QINT(0))) {
		assert(qunequal(a, QINT(0)));
		if (qmore(a, QINT(0)))
			return QPI/2;
		return -(QPI/2);
	} else if (qless(b, QINT(0))) {
		if (qeqmore(a, QINT(0)))
			return qadd(qatan(qdiv(a, b)), QPI);
		return qsub(qatan(qdiv(a, b)), QPI);
	}
	cordic(CORDIC_COORD_CIRCULAR_E, CORDIC_MODE_VECTOR_E, -1, &x, &y, &z);
	return z;
}

void qsincos(q_t theta, q_t *sine, q_t *cosine) {
	assert(sine);
	assert(cosine);
	const int r = qcordic(theta, -1, sine, cosine);
	assert(r >= 0);
}

q_t qsin(const q_t theta) {
	q_t sine = QINT(0), cosine = QINT(0);
	qsincos(theta, &sine, &cosine);
	return sine;
}

q_t qcos(const q_t theta) {
	q_t sine = QINT(0), cosine = QINT(0);
	qsincos(theta, &sine, &cosine);
	return cosine;
}

q_t qtan(const q_t theta) {
	q_t sine = QINT(0), cosine = QINT(0);
	qsincos(theta, &sine, &cosine);
	return qdiv(sine, cosine); /* can use qcordic_div, with range limits it imposes */
}

q_t qcot(const q_t theta) {
	q_t sine = QINT(0), cosine = QINT(0);
	qsincos(theta, &sine, &cosine);
	return qdiv(cosine, sine); /* can use qcordic_div, with range limits it imposes */
}

q_t qcordic_mul(const q_t a, const q_t b) { /* works for small values; result < 4 */
	q_t x = a, y = QINT(0), z = b;
	const int r = cordic(CORDIC_COORD_LINEAR_E, CORDIC_MODE_ROTATE_E, -1, &x, &y, &z);
	assert(r >= 0);
	return y;
}

q_t qcordic_div(const q_t a, const q_t b) {
	q_t x = b, y = a, z = QINT(0);
	const int r = cordic(CORDIC_COORD_LINEAR_E, CORDIC_MODE_VECTOR_E, -1, &x, &y, &z);
	assert(r >= 0);
	return z;
}

void qsincosh(const q_t a, q_t *sinh, q_t *cosh) {
	assert(sinh);
	assert(cosh);
	q_t x = cordic_hyperbolic_inverse_scaling, y = QINT(0), z = a; /* (e^2x - 1) / (e^2x + 1) */
	const int r = cordic(CORDIC_COORD_HYPERBOLIC_E, CORDIC_MODE_ROTATE_E, -1, &x, &y, &z);
	assert(r >= 0);
	*sinh = y;
	*cosh = x;
}

q_t qtanh(const q_t a) {
	q_t sinh = QINT(0), cosh = QINT(0);
	qsincosh(a, &sinh, &cosh);
	return qdiv(sinh, cosh);
}

q_t qcosh(const q_t a) {
	q_t sinh = QINT(0), cosh = QINT(0);
	qsincosh(a, &sinh, &cosh);
	return cosh;
}

q_t qsinh(const q_t a) {
	q_t sinh = QINT(0), cosh = QINT(0);
	qsincosh(a, &sinh, &cosh);
	return sinh;
}

q_t qcordic_exp(const q_t e) {
	q_t s = QINT(0), h = QINT(0);
	qsincosh(e, &s, &h);
	return qadd(s, h);
}

q_t qcordic_ln(const q_t d) {
	q_t x = qadd(d, QINT(1)), y = qsub(d, QINT(1)), z = QINT(0);
	const int r = cordic(CORDIC_COORD_HYPERBOLIC_E, CORDIC_MODE_VECTOR_E, -1, &x, &y, &z);
	assert(r >= 0);
	return qadd(z, z);
}

q_t qcordic_sqrt(const q_t n) {  /* testing only; works for 0 < x < 2 */
	const q_t quarter = 1uLL << (QBITS - 2); /* 0.25 */
	q_t x = qadd(n, quarter),
	    y = qsub(n, quarter),
	    z = 0;
	const int r = cordic(CORDIC_COORD_HYPERBOLIC_E, CORDIC_MODE_VECTOR_E, -1, &x, &y, &z);
	assert(r >= 0);
	return qmul(x, cordic_hyperbolic_inverse_scaling);
}

q_t qhypot(const q_t a, const q_t b) {
	q_t x = qabs(a), y = qabs(b), z = QINT(0); /* abs() should not be needed? */
	const int r = cordic(CORDIC_COORD_CIRCULAR_E, CORDIC_MODE_VECTOR_E, -1, &x, &y, &z);
	assert(r >= 0);
	return qmul(x, cordic_circular_inverse_scaling);
}

q_t qatanh(q_t x) {
	assert(qabs(qless(x, QINT(1))));
	return qmul(qlog(qdiv(qadd(QINT(1), x), qsub(QINT(1), x))), QMK(0, 0x8000, 16));
}

q_t qasinh(q_t x) {
	return qlog(qadd(x, qsqrt(qadd(qmul(x, x), QINT(1)))));
}

q_t qacosh(q_t x) {
	assert(qeqmore(x, QINT(1)));
	return qlog(qadd(x, qsqrt(qsub(qmul(x, x), QINT(1)))));
}

void qpol2rec(const q_t magnitude, const q_t theta, q_t *i, q_t *j) {
	assert(i);
	assert(j);
	q_t sin = QINT(0), cos = QINT(0);
	qsincos(theta, &sin, &cos);
	*i = qmul(sin, magnitude);
	*j = qmul(cos, magnitude);
}

void qrec2pol(const q_t i, const q_t j, q_t *magnitude, q_t *theta) {
	assert(magnitude);
	assert(theta);
	const int is = qisnegative(i), js = qisnegative(j);
	q_t x = qabs(i), y = qabs(j), z = QINT(0);
	const int r = cordic(CORDIC_COORD_CIRCULAR_E, CORDIC_MODE_VECTOR_E, -1, &x, &y, &z);
	assert(r >= 0);
	*magnitude = qmul(x, cordic_circular_inverse_scaling);
	if (is && js)
		z = qadd(z, QPI);
	else if (js)
		z = qadd(z, QPI/2l);
	else if (is)
		z = qadd(z, (3l*QPI)/2l);
	*theta = z;
}

q_t qcordic_hyperbolic_gain(const int n) {
	q_t x = QINT(1), y = QINT(0), z = QINT(0);
	const int r = cordic(CORDIC_COORD_HYPERBOLIC_E, CORDIC_MODE_ROTATE_E, n, &x, &y, &z);
	assert(r >= 0);
	return x;
}

q_t qcordic_circular_gain(const int n) {
	q_t x = QINT(1), y = QINT(0), z = QINT(0);
	const int r = cordic(CORDIC_COORD_CIRCULAR_E, CORDIC_MODE_ROTATE_E, n, &x, &y, &z);
	assert(r >= 0);
	return x;
}

static inline int isodd(const unsigned n) {
	return n & 1;
}

d_t dpower(d_t b, unsigned e) { /* https://stackoverflow.com/questions/101439 */
    d_t result = 1;
    for (;;) {
        if (isodd(e))
            result *= b;
        e >>= 1;
        if (!e)
            break;
        b *= b;
    }
    return result;
}

d_t dlog(d_t x, const unsigned base) { /* rounds up, look at remainder to round down */
	d_t b = 0;
	assert(x && base > 1);
	while ((x /= (d_t)base)) /* can use >> for base that are powers of two */
		b++;
	return b;
}

q_t qlog(q_t x) {
	q_t logs = 0;
	assert(qmore(x, 0));
	static const q_t lmax = QMK(9, 0x8000, 16); /* 9.5, lower limit needs checking */
	for (; qmore(x, lmax); x = divn(x, 1))
		logs = qadd(logs, qinfo.ln2);
	return qadd(logs, qcordic_ln(x));
}

q_t qsqr(const q_t x) {
	return qmul(x, x);
}

q_t qexp(const q_t e) { /* exp(e) = exp(e/2)*exp(e/2) */
	if (qless(e, QINT(1))) /* 1.1268 is approximately the limit for qcordic_exp */
		return qcordic_exp(e);
	return qsqr(qexp(divn(e, 1)));
}

q_t qpow(q_t n, q_t exp) {
	implies(qisnegative(n), qisinteger(exp));
	implies(qequal(n, QINT(0)), qunequal(exp, QINT(0)));
	if (qequal(QINT(0), n))
		return QINT(1);
	if (qisnegative(n)) {
		const q_t abspow = qpow(qabs(n), exp);
		return qisodd(exp) ? qnegate(abspow) : abspow;
	}
	if (qisnegative(exp))
		return qdiv(QINT(1), qpow(n, qabs(exp)));
	return qexp(multiply(qlog(n), exp));
}

q_t qsqrt(const q_t x) { /* Newton-Rhaphson method */
	assert(qeqmore(x, 0));
	const q_t difference = qmore(x, QINT(100)) ? 0x0100 : 0x0010;
	if (qequal(QINT(0), x))
		return QINT(0);
	q_t guess = qmore(x, qinfo.sqrt2) ? divn(x, 1) : QINT(1);
	while (qmore(qabs(qsub(qmul(guess, guess), x)), difference))
		guess = divn(qadd(qdiv(x, guess), guess), 1);
	return qabs(guess); /* correct for overflow int very large numbers */
}

q_t qasin(const q_t t) {
	assert(qless(qabs(t), QINT(1)));
	/* can also use: return qatan(qdiv(t, qsqrt(qsub(QINT(1), qmul(t, t))))); */
	return qatan2(t, qsqrt(qsub(QINT(1), qmul(t, t))));
}

q_t qacos(const q_t t) {
	assert(qeqless(qabs(t), QINT(1)));
	/* can also use: return qatan(qdiv(qsqrt(qsub(QINT(1), qmul(t, t))), t)); */
	return qatan2(qsqrt(qsub(QINT(1), qmul(t, t))), t);
}

q_t qdeg2rad(const q_t deg) {
	return qdiv(qmul(QPI, deg), QINT(180));
}

q_t qrad2deg(const q_t rad) {
	return qdiv(qmul(QINT(180), rad), QPI);
}

/* Simpsons method for numerical integration, from "Math Toolkit for 
 * Real-Time Programming" by Jack Crenshaw */
q_t qsimpson(q_t (*f)(q_t), const q_t x1, const q_t x2, const unsigned n) {
	assert(f);
	assert((n & 1) == 0);
	const q_t h = qdiv(qsub(x2, x1), QINT(n));
	q_t sum = 0, x = x1;
	for (unsigned i = 0; i < (n / 2u); i++){
		sum = qadd(sum, qadd(f(x), qmul(QINT(2), f(qadd(x,h)))));
		x   = qadd(x, qmul(QINT(2), h));
	}
	sum = qsub(qmul(QINT(2), sum), qadd(f(x1), f(x2)));
	return qdiv(qmul(h, sum), QINT(3));
}

static q_t qfz(q_t a) { UNUSED(a); return QINT(0); }
static q_t qf1(q_t a) { UNUSED(a); return QINT(1); }

static int addchar(char **str, size_t *length, const int ch) {
	assert(str && *str);
	assert(length);
	if (!length)
		return -1;
	char *s = *str;
	*s++ = ch;
	*str = s;
	*length -= 1;
	return 0;
}

static int addstr(char **str, size_t *length, char *addme) {
	assert(str && *str);
	assert(length);
	assert(addme);
	const size_t sz = strlen(addme);
	for (size_t i = 0; i < sz; i++)
		if (addchar(str, length, addme[i]) < 0)
			return -1;
	return 0;
}

/* See <https://github.com/jamesbowman/sincos> 
 * and "Math Toolkit for Real-Time Programming" by Jack Crenshaw 
 *
 * The naming of these functions ('furman_') is incorrect, they do their
 * computation on numbers represented in Furmans but they do not use a 'Furman
 * algorithm'. As I do not have a better name, the name shall stick. */
static int16_t _sine(const int16_t y) {
	const int16_t s1 = 0x6487, s3 = -0x2953, s5 = 0x04f8;
	const int16_t z = arshift((int32_t)y * y, 12);
	int16_t prod = arshift((int32_t)z * s5, 16);
	int16_t sum = s3 + prod;
	prod = arshift((int32_t)z * sum, 16);
	sum = s1 + prod;
	return arshift((int32_t)y * sum,  13);
}

static int16_t _cosine(int16_t y) {
	const int16_t c0 = 0x7fff, c2 = -0x4ee9, c4 = 0x0fbd;
	const int16_t z = arshift((int32_t)y * y, 12);
	int16_t prod = arshift((int32_t)z * c4,  16);
	const int16_t sum = c2 + prod;
	prod = arshift((int32_t)z * sum, 15);
	return c0 + prod;
}

int16_t furman_sin(int16_t x) {
	const int16_t n = 3 & arshift(x + 0x2000, 14);
	x -= n << 14;
	const int16_t r = (n & 1) ? _cosine(x) : _sine(x);
	return (n & 2) ? -r : r;
}

int16_t furman_cos(int16_t x) {
	return furman_sin(x + 0x4000);
}

/* expression evaluator */

static q_t numberify(const char *s) {
	assert(s);
	q_t q = 0;
	(void) qconv(&q, s);
	return q;
}

// static q_t qbase(q_t b) {
// 	int nb = qtoi(b);
// 	if (nb < 2 || nb > 36)
// 		return -QINT(1);
// 	qconf.base = nb;
// 	return b;
// }

void check_div0(q_t a, q_t b) {
	UNUSED(a);
	if (!b)
		mp_raise_ValueError(MP_ERROR_TEXT("division by zero"));
}

void check_nlz(q_t a) { // Not Less Zero
	if (qless(a, QINT(0)))
		mp_raise_ValueError(MP_ERROR_TEXT("negative argument"));
}

void check_nlez(q_t a) { // Not Less Equal Zero
	if (qeqless(a, QINT(0)))
		mp_raise_ValueError(MP_ERROR_TEXT("negative or zero argument"));
}

void check_nlo(q_t a) { // Not less than one
	if (qless(a, QINT(1)))
		mp_raise_ValueError(MP_ERROR_TEXT("out of range [1, INF]"));
}

void check_alo(q_t a) {
	if (qmore(qabs(a), QINT(1)))
		mp_raise_ValueError(MP_ERROR_TEXT("out of range [-1, 1]"));
}

// const qoperations_t *qop(const char *op) {
// 	assert(op);
// 	static const qoperations_t ops[] = {
// 		/* Binary Search Table: Use 'LC_ALL="C" sort -k 2 < table' to sort this */
// 		/* name         function                       check function        precedence arity left/right-assoc hidden */     
// 		{  "!",         .eval.unary   =  qnot,         .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "!=",        .eval.binary  =  qunequal,     .check.binary  =  NULL,        2,  2,  ASSOCIATE_LEFT,   0,  },
// 		{  "%",         .eval.binary  =  qrem,/*!*/    .check.binary  =  check_div0,  3,  2,  ASSOCIATE_LEFT,   0,  },
// 		{  "&",         .eval.binary  =  qand,         .check.binary  =  NULL,        2,  2,  ASSOCIATE_LEFT,   0,  },
// 		{  "(",         .eval.unary   =  NULL,         .check.unary   =  NULL,        0,  0,  ASSOCIATE_NONE,   0,  },
// 		{  ")",         .eval.unary   =  NULL,         .check.unary   =  NULL,        0,  0,  ASSOCIATE_NONE,   0,  },
// 		{  "*",         .eval.binary  =  qmul,         .check.binary  =  NULL,        3,  2,  ASSOCIATE_LEFT,   0,  },
// 		{  "+",         .eval.binary  =  qadd,         .check.binary  =  NULL,        2,  2,  ASSOCIATE_LEFT,   0,  },
// 		{  "-",         .eval.binary  =  qsub,         .check.binary  =  NULL,        2,  2,  ASSOCIATE_LEFT,   0,  },
// 		{  "/",         .eval.binary  =  qdiv,         .check.binary  =  check_div0,  3,  2,  ASSOCIATE_LEFT,   0,  },
// 		{  "<",         .eval.binary  =  qless,        .check.binary  =  NULL,        2,  2,  ASSOCIATE_LEFT,   0,  },
// 		{  "<<",        .eval.binary  =  qlls,         .check.binary  =  NULL,        4,  2,  ASSOCIATE_RIGHT,  0,  },
// 		{  "<=",        .eval.binary  =  qeqless,      .check.binary  =  NULL,        2,  2,  ASSOCIATE_LEFT,   0,  },
// 		{  "==",        .eval.binary  =  qequal,       .check.binary  =  NULL,        2,  2,  ASSOCIATE_LEFT,   0,  },
// 		{  ">",         .eval.binary  =  qmore,        .check.binary  =  NULL,        2,  2,  ASSOCIATE_LEFT,   0,  },
// 		{  ">=",        .eval.binary  =  qeqmore,      .check.binary  =  NULL,        2,  2,  ASSOCIATE_LEFT,   0,  },
// 		{  ">>",        .eval.binary  =  qlrs,         .check.binary  =  NULL,        4,  2,  ASSOCIATE_RIGHT,  0,  },
// 		{  "^",         .eval.binary  =  qxor,         .check.binary  =  NULL,        2,  2,  ASSOCIATE_LEFT,   0,  },
// 		{  "_div",      .eval.binary  =  qcordic_div,  .check.binary  =  NULL,        5,  2,  ASSOCIATE_RIGHT,  1,  },
// 		{  "_exp",      .eval.unary   =  qcordic_exp,  .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  1,  },
// 		{  "_ln",       .eval.unary   =  qcordic_ln,   .check.unary   =  check_nlez,  5,  1,  ASSOCIATE_RIGHT,  1,  },
// 		{  "_mul",      .eval.binary  =  qcordic_mul,  .check.binary  =  NULL,        5,  2,  ASSOCIATE_RIGHT,  1,  },
// 		{  "_sqrt",     .eval.unary   =  qcordic_sqrt, .check.unary   =  check_nlz,   5,  1,  ASSOCIATE_RIGHT,  1,  },
// 		{  "abs",       .eval.unary   =  qabs,         .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "acos",      .eval.unary   =  qacos,        .check.unary   =  check_alo,   5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "acosh",     .eval.unary   =  qacosh,       .check.unary   =  check_nlo,   5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "arshift",   .eval.binary  =  qars,         .check.binary  =  NULL,        4,  2,  ASSOCIATE_RIGHT,  1,  },
// 		{  "asin",      .eval.unary   =  qasin,        .check.unary   =  check_alo,   5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "asinh",     .eval.unary   =  qasinh,       .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "atan",      .eval.unary   =  qatan,        .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "atan2",     .eval.binary  =  qatan2,       .check.binary  =  NULL,        5,  2,  ASSOCIATE_RIGHT,  1,  },
// 		{  "atanh",     .eval.unary   =  qatanh,       .check.unary   =  check_alo,   5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "base",      .eval.unary   =  qbase,        .check.unary   =  NULL,        2,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "ceil",      .eval.unary   =  qceil,        .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "copysign",  .eval.binary  =  qcopysign,    .check.binary  =  NULL,        4,  2,  ASSOCIATE_RIGHT,  1,  },
// 		{  "cos",       .eval.unary   =  qcos,         .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "cosh",      .eval.unary   =  qcosh,        .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "cot",       .eval.unary   =  qcot,         .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "deg2rad",   .eval.unary   =  qdeg2rad,     .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "even?",     .eval.unary   =  qiseven,      .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "exp",       .eval.unary   =  qexp,         .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "floor",     .eval.unary   =  qfloor,       .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "hypot",     .eval.binary  =  qhypot,       .check.binary  =  NULL,        5,  2,  ASSOCIATE_RIGHT,  0,  },
// 		{  "int?",      .eval.unary   =  qisinteger,   .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "log",       .eval.unary   =  qlog,         .check.unary   =  check_nlez,  5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "lshift",    .eval.binary  =  qlls,         .check.binary  =  NULL,        4,  2,  ASSOCIATE_RIGHT,  1,  },
// 		{  "max",       .eval.binary  =  qmax,         .check.binary  =  NULL,        5,  2,  ASSOCIATE_RIGHT,  1,  },
// 		{  "min",       .eval.binary  =  qmin,         .check.binary  =  NULL,        5,  2,  ASSOCIATE_RIGHT,  1,  },
// 		{  "mod",       .eval.binary  =  qmod,         .check.binary  =  check_div0,  3,  2,  ASSOCIATE_LEFT,   0,  },
// 		{  "neg?",      .eval.unary   =  qisnegative,  .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "negate",    .eval.unary   =  qnegate,      .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "odd?",      .eval.unary   =  qisodd,       .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "places",    .eval.unary   =  qplaces,      .check.unary   =  NULL,        2,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "pos?",      .eval.unary   =  qispositive,  .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "pow",       .eval.binary  =  qpow,         .check.binary  =  NULL,        5,  2,  ASSOCIATE_RIGHT,  0,  },
// 		{  "rad2deg",   .eval.unary   =  qrad2deg,     .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "rem",       .eval.binary  =  qrem,         .check.binary  =  check_div0,  3,  2,  ASSOCIATE_LEFT,   0,  },
// 		{  "round",     .eval.unary   =  qround,       .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "rshift",    .eval.binary  =  qlrs,         .check.binary  =  NULL,        4,  2,  ASSOCIATE_RIGHT,  1,  },
// 		{  "sign",      .eval.unary   =  qsign,        .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "signum",    .eval.unary   =  qsignum,      .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "sin",       .eval.unary   =  qsin,         .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "sinh",      .eval.unary   =  qsinh,        .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "sqrt",      .eval.unary   =  qsqrt,        .check.unary   =  check_nlz,   5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "tan",       .eval.unary   =  qtan,         .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "tanh",      .eval.unary   =  qtanh,        .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "trunc",     .eval.unary   =  qtrunc,       .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 		{  "|",         .eval.binary  =  qor,          .check.binary  =  NULL,        2,  2,  ASSOCIATE_LEFT,   0,  },
// 		{  "~",         .eval.unary   =  qinvert,      .check.unary   =  NULL,        5,  1,  ASSOCIATE_RIGHT,  0,  },
// 	};
// 	const size_t length = (sizeof ops / sizeof ops[0]);
// 	size_t l = 0, r = length - 1;
// 	while (l <= r) { // Iterative Binary Search
// 		size_t m = l + ((r - l)/2u);
// 		assert (m < length);
// 		const int comp = strcmp(ops[m].name, op);
// 		if (comp == 0)
// 			return &ops[m];
// 		if (comp < 0)
// 			l = m + 1;
// 		else
// 			r = m - 1;
// 	}
// 	return NULL;
// }
