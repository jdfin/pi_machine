#include <Arduino.h>
#include <math.h>
#include <pidec.h>

static int64_t _m;
static double _invm;


static void InitializeModulo(int64_t m)
{
  _m = m;
  _invm = 1. / (double)m;
}


// Compute a*b modulo _m
inline int64_t MulMod(int64_t a, int64_t b)
{
  // classical trick to bypass the 64-bit limitation, when a*b does not fit into
  // the int64_t type. Works whenever a*b/_m is less than 2^52 (double type
  // maximal precision)
  int64_t q = (int64_t)(_invm * (double)a * (double)b);
  return a * b - q * _m;
}


// Compute a*b+c*d modulo _m
inline int64_t SumMulMod(int64_t a, int64_t b, int64_t c, int64_t d)
{
  int64_t q = (int64_t)(_invm * ((double)a * (double)b + (double)c * (double)d));
  return a * b + c * d - q * _m;
}


inline double easyround(double x)
{
  const double FullDouble = 1024. * 1024. * 1024. * 1024. * 1024. * 8.; // 2^53
  double y = x + FullDouble;
  y -= FullDouble;
  return y;
}


/* return g, A such that g=gcd(a,_m) and a*A=g mod _m  */
static int64_t ExtendedGcd(int64_t a, int64_t& A)
{
  int64_t A0 = 1, A1 = 0;
  int64_t r0 = a, r1 = _m;

  while (r1 > 0.) {
    int64_t q = r0 / r1;

    int64_t tmp = A0 - q * A1;
    A0 = A1;
    A1 = tmp;

    tmp = r0 - q * r1;
    r0 = r1;
    r1 = tmp;
  }
  A = A0;
  return r0;
}


static int64_t InvMod(int64_t a)
{
  int64_t A;
  a = a % _m;
  if (a < 0)
    a += _m;
  (void)ExtendedGcd(a, A);
  return A;
}


static int64_t PowMod(int64_t a, long b)
{
  int64_t r, aa;

  r = 1;
  aa = a;
  while (1) {
    if (b & 1)
      r = MulMod(r, aa);
    b >>= 1;
    if (b == 0)
      break;
    aa = MulMod(aa, aa);
  }
  return r;
}


/* Compute sum_{j=0}^k binomial(n,j) mod m */
static int64_t SumBinomialMod(long n, long k)
{
  // Optimisation : when k>n/2 we use the relation
  // sum_{j=0}^k binomial(n,j) =  2^n - sum_{j=0}^{n-k-1} binomial(n,j)
  //
  // Note : additionnal optimization, not afforded here, could be done when k is
  // near n/2 using the identity sum_{j=0}^{n/2} = 2^(n-1) + 1/2
  // binomial(n,n/2). A global saving of 20% or 25% could be obtained.
  if (k > n / 2) {
    int64_t s = PowMod(2, n) - SumBinomialMod(n, n - k - 1);
    if (s < 0)
      s += _m;
    return s;
  }
  //
  // Compute prime factors of _m which are smaller than k
  //
  const long NbMaxFactors = 20; // no more than 20 different prime factors for numbers <2^64
  long PrimeFactor[NbMaxFactors];
  long NbPrimeFactors = 0;
  int64_t mm = _m;
  // _m is odd, thus has only odd prime factors
  for (int64_t p = 3; p * p <= mm; p += 2) {
    if (mm % p == 0) {
      mm = mm / p;
      if (p <= k) // only prime factors <=k are needed
        PrimeFactor[NbPrimeFactors++] = p;
      while (mm % p == 0)
        mm = mm / p; // remove all powers of p in mm
    }
  }
  // last factor : if mm is not 1, mm is necessarily prime
  if (mm > 1 && mm <= k) {
    PrimeFactor[NbPrimeFactors++] = mm;
  }

  // BinomialExponent[i] will contain the power of PrimeFactor[i] in binomial
  long BinomialPower[NbMaxFactors];
  // NextDenom[i] and NextNum[i] will contain next multiples of PrimeFactor[i]
  long NextDenom[NbMaxFactors], NextNum[NbMaxFactors];
  for (long i = 0; i < NbPrimeFactors; i++) {
    BinomialPower[i] = 1;
    NextDenom[i] = PrimeFactor[i];
    NextNum[i] = PrimeFactor[i] * (n / PrimeFactor[i]);
  }

  int64_t BinomialNum0 = 1, BinomialDenom = 1;
  int64_t SumNum = 1;
  int64_t BinomialSecondary = 1;

  for (long j = 1; j <= k; j++) {
    // new binomial : b(n,j) = b(n,j-1) * (n-j+1) / j
    int64_t num = n - j + 1;
    int64_t denom = j;
    int BinomialSecondaryUpdate = 0;

    for (long i = 0; i < NbPrimeFactors; i++) {
      long p = PrimeFactor[i];
      // Test if p is a prime factor of num0
      if (NextNum[i] == n - j + 1) {
        BinomialSecondaryUpdate = 1;
        NextNum[i] -= p;
        BinomialPower[i] *= p;
        num /= p;
        while (num % p == 0) {
          BinomialPower[i] *= p;
          num /= p;
        }
      }
      // Test if p is a prime factor of denom0
      if (NextDenom[i] == j) {
        BinomialSecondaryUpdate = 1;
        NextDenom[i] += p;
        BinomialPower[i] /= p;
        denom /= p;
        while (denom % p == 0) {
          BinomialPower[i] /= p;
          denom /= p;
        }
      }
    }

    if (BinomialSecondaryUpdate) {
      BinomialSecondary = BinomialPower[0];
      for (long i = 1; i < NbPrimeFactors; i++)
        BinomialSecondary = MulMod(BinomialSecondary, BinomialPower[i]);
    }

    BinomialNum0 = MulMod(BinomialNum0, num);
    BinomialDenom = MulMod(BinomialDenom, denom);

    if (BinomialSecondary != 1) {
      SumNum = SumMulMod(SumNum, denom, BinomialNum0, BinomialSecondary);
    } else {
      SumNum = MulMod(SumNum, denom) + BinomialNum0;
    }
  }
  SumNum = MulMod(SumNum, InvMod(BinomialDenom));
  return SumNum;
}


/* return fractionnal part of 10^n*(a/b) */
static double DigitsOfFraction(long n, int64_t a, int64_t b)
{
  InitializeModulo(b);
  int64_t pow = PowMod(10, n);
  int64_t c = MulMod(pow, a);
  return (double)c / (double)b;
}


/* return fractionnal part of 10^n*S, where S=4*sum_{k=0}^{m-1} (-1)^k/(2*k+1).
 * m is even */
static double DigitsOfSeries(long n, int64_t m)
{
  double x = 0.;
  for (int64_t k = 0; k < m; k += 2) {
    x += DigitsOfFraction(n, 4, 2 * k + 1) - DigitsOfFraction(n, 4, 2 * k + 3);
    x = x - easyround(x);
  }
  return x;
}


double DigitsOfPi(long n)
{
  double logn = log((double)n);
  long M = 2 * (long)(3. * n / logn / logn / logn); // M is even
  long N = 1 + (long)((n + 15.) * log(10.) / (1. + log(2. * M))); // n >= N
  N += N % 2; // N should be even
  int64_t mmax = (int64_t)M * (int64_t)N + (int64_t)N;
  double x = DigitsOfSeries(n, mmax);
  for (long k = 0.; k < N; k++) {
    int64_t m = (int64_t)2 * (int64_t)M * (int64_t)N + (int64_t)2 * (int64_t)k + 1;
    InitializeModulo(m);
    int64_t s = SumBinomialMod(N, k);
    s = MulMod(s, PowMod(5, N));
    s = MulMod(s, PowMod(10, n - N)); // n-N is always positive
    s = MulMod(s, 4);
    x += (2 * (k % 2) - 1) * (double)s / (double)m; // 2*(k%2)-1 = (-1)^(k-1)
    x = x - floor(x);
  }
  return x;
}
