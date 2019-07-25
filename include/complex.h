#ifndef __DB_COMPLEX_HP__
#define __DB_COMPLEX_HP__

/* Arbitrary precision header. Here, GMP is used. Linker required: -lgmpxx -lgmp */
#include <gmpxx.h>

/* Classes for double precision complex and high precision complex numbers */
class complexDP {
    public:
        double re,im;
        int bitsPrec;
        
        constexpr complexDP(): re(0.0), im(0.0), bitsPrec(64) {}
        constexpr complexDP(double _re, double _im, int _bitsPrec = 64): re(_re), im(_im), bitsPrec(_bitsPrec) {}
};

class complexHP {
    public:
        mpf_class re,im;
};

/* Operator "+" for double precision complex numbers */
constexpr complexDP operator+(const complexDP &a, const complexDP &b) {
    return complexDP( a.re + b.re, a.im + b.im );
}

/* Operator "*" for double precision complex numbers */
constexpr complexDP operator*(const complexDP &a, const complexDP &b) {
    return complexDP( a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re );
}

/* Convert arbitrary precision number to double precision number */
inline complexDP complexDescend(const complexHP &a) {
    return complexDP( a.re.get_d(), a.im.get_d() );
}

/* Squaring of double precision number */
constexpr complexDP sqare(const complexDP &a) {
    return complexDP( a.re * a.re - a.im * a.im, 2.0 * a.re * a.im );
}

/* Squared magnitude of double precision number */
constexpr double sqrtSquared(const complexDP &a) {
    return ( a.re * a.re + a.im * a.im );
}

#endif