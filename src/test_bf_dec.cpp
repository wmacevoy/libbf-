#include <iostream>
#include <sstream>
#include <cmath>
#include <climits>
#include <cstdint>
#include <string>
#include "bf.hpp"
#include "facts.h"

using namespace bellard;

// ── Constructors ────────────────────────────────────────────────────

FACTS(BFDec_Constructors) {
  BFContext context(256, 34);
  retain<BFContext> use(&context);

  // Default is zero
  FACT(BFDec().is_zero(), ==, true);

  // From int
  FACT(BFDec(0).is_zero(), ==, true);
  FACT(BFDec(42).to_double(), ==, 42.0);
  FACT(BFDec(-1).to_double(), ==, -1.0);

  // From int64_t
  FACT(BFDec(int64_t(1000000)).to_double(), ==, 1000000.0);

  // From uint64_t
  FACT(BFDec(uint64_t(0)).is_zero(), ==, true);
  FACT(BFDec(uint64_t(100)).to_double(), ==, 100.0);

  // From double
  FACT(BFDec(0.5).to_double(), ==, 0.5);
  FACT(BFDec(1.0).to_double(), ==, 1.0);

  // From string — exact decimal
  FACT(BFDec("42").to_double(), ==, 42.0);
  FACT(BFDec("0.1").to_string(), ==, std::string("0.1"));
  FACT(BFDec("0.5").to_double(), ==, 0.5);
  FACT(BFDec("-3.14").to_double(), ==, -3.14);

  // Copy
  BFDec a("123.456");
  BFDec b(a);
  FACT(b.to_string(), ==, a.to_string());

  // Move
  BFDec c(std::move(b));
  FACT(c.to_string(), ==, a.to_string());
}

// ── Decimal exactness ───────────────────────────────────────────────

FACTS(BFDec_Exactness) {
  BFContext context(256, 34);
  retain<BFContext> use(&context);

  // 0.1 is exact in decimal, not in binary
  BFDec d("0.1");
  FACT(d.to_string(), ==, std::string("0.1"));

  // 0.1 + 0.2 == 0.3 exactly in decimal
  BFDec sum = BFDec("0.1") + BFDec("0.2");
  FACT(sum == BFDec("0.3"), ==, true);

  // Financial: $19.99 + 7.25% tax
  BFDec price("19.99");
  BFDec tax("0.0725");
  BFDec total = price + price * tax;
  FACT(total == BFDec("21.439275"), ==, true);

  // Exact representation preserved through arithmetic
  BFDec a("1.23");
  BFDec b("4.56");
  FACT((a + b) == BFDec("5.79"), ==, true);
  FACT((b - a) == BFDec("3.33"), ==, true);
}

// ── Arithmetic ──────────────────────────────────────────────────────

FACTS(BFDec_Arithmetic) {
  BFContext context(256, 34);
  retain<BFContext> use(&context);

  BFDec a(10), b(3);

  FACT((a + b).to_double(), ==, 13.0);
  FACT((a - b).to_double(), ==, 7.0);
  FACT((a * b).to_double(), ==, 30.0);

  // Division
  BFDec q = a / b;
  FACT(q.is_finite(), ==, true);

  // Unary negation
  FACT((-a).to_double(), ==, -10.0);

  // Compound assignment
  BFDec x(5);
  x += BFDec(3); FACT(x.to_double(), ==, 8.0);
  x -= BFDec(2); FACT(x.to_double(), ==, 6.0);
  x *= BFDec(4); FACT(x.to_double(), ==, 24.0);
  x /= BFDec(6); FACT(x.to_double(), ==, 4.0);
}

// ── Comparison ──────────────────────────────────────────────────────

FACTS(BFDec_Comparison) {
  BFContext context(256, 34);
  retain<BFContext> use(&context);

  BFDec a("1.1"), b("2.2"), c("1.1");

  FACT(a == c, ==, true);
  FACT(a != b, ==, true);
  FACT(a < b, ==, true);
  FACT(b > a, ==, true);
  FACT(a <= c, ==, true);
  FACT(a <= b, ==, true);
  FACT(b >= a, ==, true);

  FACT(a == b, ==, false);
  FACT(a != c, ==, false);
  FACT(b < a, ==, false);
}

// ── State queries ───────────────────────────────────────────────────

FACTS(BFDec_StateQueries) {
  BFContext context(256, 34);
  retain<BFContext> use(&context);

  FACT(BFDec(0).is_zero(), ==, true);
  FACT(BFDec(1).is_zero(), ==, false);
  FACT(BFDec(1).is_finite(), ==, true);
  FACT(BFDec(1).is_nan(), ==, false);
  FACT(BFDec(1).is_inf(), ==, false);
}

// ── Math ────────────────────────────────────────────────────────────

FACTS(BFDec_Math) {
  BFContext context(256, 34);
  retain<BFContext> use(&context);

  // sqrt
  BFDec s = sqrt(BFDec(4));
  FACT(s == BFDec(2), ==, true);

  // sqrt(2) close to known value
  BFDec s2 = sqrt(BFDec(2));
  FACT(s2.to_double() > 1.414, ==, true);
  FACT(s2.to_double() < 1.415, ==, true);

  // abs
  FACT(abs(BFDec(-5)).to_double(), ==, 5.0);
  FACT(abs(BFDec(5)).to_double(), ==, 5.0);

  // pow (integer exponent)
  FACT(pow(BFDec(2), 10).to_double(), ==, 1024.0);
  FACT(pow(BFDec(10), 3).to_double(), ==, 1000.0);
}

// ── Conversions ─────────────────────────────────────────────────────

FACTS(BFDec_Conversions) {
  BFContext context(256, 34);
  retain<BFContext> use(&context);

  // to_double with rounding modes
  BFDec tenth("0.1");
  double dn = tenth.to_double(BF_RNDN);
  double dd = tenth.to_double(BF_RNDD);
  double du = tenth.to_double(BF_RNDU);
  FACT(dd < du, ==, true);     // 0.1 not exact in double
  FACT(dd <= dn, ==, true);
  FACT(dn <= du, ==, true);

  // to_double exact cases
  FACT(BFDec("0.5").to_double(), ==, 0.5);
  FACT(BFDec("0.25").to_double(), ==, 0.25);
  FACT(BFDec("1024").to_double(), ==, 1024.0);

  // to_int64
  FACT(BFDec(42).to_int64(), ==, 42);

  // to_string
  FACT(BFDec("0.1").to_string(), ==, std::string("0.1"));
  FACT(BFDec(42).to_string(), ==, std::string("42"));

  // BFDec -> BF -> BFDec for exact values
  BFDec half("0.5");
  BF bf_half(half);
  BFDec back(bf_half);
  FACT(back == half, ==, true);
}

// ── Precision ───────────────────────────────────────────────────────

FACTS(BFDec_Precision) {
  BFContext context(256, 34);
  retain<BFContext> use(&context);

  // 1/3 at 34 digits
  BFDec third = BFDec(1) / BFDec(3);
  std::string s34 = third.to_string();

  // 1/3 at 50 digits
  bf_dprec(50);
  BFDec third50 = BFDec(1) / BFDec(3);
  std::string s50 = third50.to_string();
  FACT(s50.size() > s34.size(), ==, true);

  // Restore and verify
  bf_dprec(34);
  BFDec again = BFDec(1) / BFDec(3);
  FACT(again.to_string(), ==, s34);
}

// ── Ref-counted allocator ───────────────────────────────────────────

FACTS(BFDec_RefCounting) {
  BFContext outer(64, 10);
  retain<BFContext> use_outer(&outer);

  BFDec escaped;
  {
    BFContext inner(256, 34);
    retain<BFContext> use(&inner);
    escaped = BFDec("3.14159265358979323846");
  }
  // escaped's allocator is still alive via ref-count
  FACT(escaped.to_double() > 3.14, ==, true);
  FACT(escaped.to_double() < 3.15, ==, true);
  FACT(escaped.is_finite(), ==, true);
}

// ── Stream output ───────────────────────────────────────────────────

FACTS(BFDec_Stream) {
  BFContext context(256, 34);
  retain<BFContext> use(&context);

  std::ostringstream oss;
  oss << BFDec(42);
  FACT(oss.str(), ==, std::string("42"));

  oss.str("");
  oss << BFDec("0.1");
  FACT(oss.str(), ==, std::string("0.1"));
}

// ── Integer boundary edge cases ─────────────────────────────────────

FACTS(BFDec_IntBoundaries) {
  BFContext context(256, 34);
  retain<BFContext> use(&context);

  // INT64_MAX / MIN
  int64_t i64max = INT64_MAX;
  int64_t i64min = INT64_MIN;
  BFDec bmax(i64max);
  BFDec bmin(i64min);
  FACT(bmax > BFDec(0), ==, true);
  FACT(bmin < BFDec(0), ==, true);
  FACT(bmax > bmin, ==, true);
  FACT((bmax + BFDec(1)) - bmax == BFDec(1), ==, true);

  // String round-trip
  std::string smax = bmax.to_string();
  FACT(BFDec(smax).to_string(), ==, smax);
  std::string smin = bmin.to_string();
  FACT(BFDec(smin).to_string(), ==, smin);

  // UINT64_MAX
  uint64_t u64max = UINT64_MAX;
  BFDec bumax(u64max);
  FACT(bumax > bmax, ==, true);
  std::string sumax = bumax.to_string();
  FACT(BFDec(sumax).to_string(), ==, sumax);

  // 2^53 boundary
  int64_t pow2_53 = int64_t(1) << 53;
  BFDec b53(pow2_53);
  BFDec b53p1(pow2_53 + 1);
  BFDec b53m1(pow2_53 - 1);

  FACT(b53m1 < b53, ==, true);
  FACT(b53 < b53p1, ==, true);
  FACT(b53p1 - b53 == BFDec(1), ==, true);

  // double can't distinguish 2^53+1, but BFDec can
  FACT(b53p1 == b53, ==, false);
}

// ── Very large / very small decimal values ──────────────────────────

FACTS(BFDec_ExtremeValues) {
  BFContext context(256, 34);
  retain<BFContext> use(&context);

  // 10^34 (at the edge of 34-digit precision)
  BFDec big = pow(BFDec(10), 34);
  FACT(big > BFDec(0), ==, true);
  FACT(big.is_finite(), ==, true);
  FACT(big / big == BFDec(1), ==, true);

  // 10^100 from string
  BFDec googol("1" + std::string(100, '0'));
  FACT(googol.is_finite(), ==, true);
  std::string sgoogol = googol.to_string();
  FACT(BFDec(sgoogol) == googol, ==, true);

  // Very small: 10^-34
  BFDec tiny = BFDec(1) / pow(BFDec(10), 34);
  FACT(tiny > BFDec(0), ==, true);
  FACT(tiny.is_finite(), ==, true);

  // Very small from string
  BFDec point_small("0." + std::string(33, '0') + "1");
  FACT(point_small > BFDec(0), ==, true);
  FACT(point_small.is_finite(), ==, true);

  // Near-overflow double: value beyond double max is still finite in BFDec
  BFDec beyond_dbl("1" + std::string(309, '0'));
  FACT(beyond_dbl.is_finite(), ==, true);
  FACT(std::isinf(beyond_dbl.to_double()), ==, true);

  // Near-underflow double: very small value
  BFDec near_zero("0." + std::string(323, '0') + "1");
  FACT(near_zero > BFDec(0), ==, true);
  FACT(near_zero.is_finite(), ==, true);
}

// ── Powers of 2 in decimal ──────────────────────────────────────────

FACTS(BFDec_PowersOf2) {
  BFContext context(256, 34);
  retain<BFContext> use(&context);

  // 0.5 = 1/2 is exact
  BFDec half("0.5");
  FACT(half + half == BFDec(1), ==, true);

  // 0.25 = 1/4 is exact
  BFDec quarter("0.25");
  FACT(quarter * BFDec(4) == BFDec(1), ==, true);

  // 0.125 = 1/8 is exact
  BFDec eighth("0.125");
  FACT(eighth * BFDec(8) == BFDec(1), ==, true);

  // 0.0625 = 1/16
  BFDec sixteenth("0.0625");
  FACT(sixteenth * BFDec(16) == BFDec(1), ==, true);

  // 2^10 = 1024
  FACT(pow(BFDec(2), 10) == BFDec(1024), ==, true);

  // 2^20 = 1048576
  FACT(pow(BFDec(2), 20) == BFDec(1048576), ==, true);

  // Negative powers: 2^-1 * 2 == 1
  BFDec inv2 = BFDec(1) / BFDec(2);
  FACT(inv2 == BFDec("0.5"), ==, true);

  // 2^-10 = 1/1024
  BFDec inv1024 = BFDec(1) / BFDec(1024);
  FACT(inv1024 * BFDec(1024) == BFDec(1), ==, true);

  // 2^53 is exact in decimal
  BFDec p53 = pow(BFDec(2), 53);
  FACT(p53 == BFDec("9007199254740992"), ==, true);

  // 2^64
  BFDec p64 = pow(BFDec(2), 64);
  FACT(p64 == BFDec("18446744073709551616"), ==, true);
}

// ── Decimal exactness edge cases ────────────────────────────────────

FACTS(BFDec_DecimalEdgeCases) {
  BFContext context(256, 34);
  retain<BFContext> use(&context);

  // Trailing zeros don't affect value
  FACT(BFDec("0.10") == BFDec("0.1"), ==, true);
  FACT(BFDec("42.00") == BFDec("42"), ==, true);
  FACT(BFDec("100") == BFDec("1e2"), ==, true);

  // Many-digit exact arithmetic
  BFDec a("99999999999999999999999999999999.99");
  BFDec b("0.01");
  FACT(a + b == BFDec("100000000000000000000000000000000"), ==, true);

  // Subtraction of close values
  BFDec x("1.000000000000000000000000000000001");
  BFDec y("1.000000000000000000000000000000000");
  BFDec diff = x - y;
  FACT(diff > BFDec(0), ==, true);
  FACT(diff == BFDec("0.000000000000000000000000000000001"), ==, true);

  // Negative zero
  BFDec nz = BFDec(0) - BFDec(0);
  FACT(nz.is_zero(), ==, true);

  // Multiplication by powers of 10 is exact
  BFDec v("1.23456789");
  FACT(v * BFDec(10) == BFDec("12.3456789"), ==, true);
  FACT(v * BFDec(100) == BFDec("123.456789"), ==, true);
  FACT(v * BFDec(1000) == BFDec("1234.56789"), ==, true);
}

FACTS_REGISTER_ALL() {
  FACTS_REGISTER(BFDec_Constructors);
  FACTS_REGISTER(BFDec_Exactness);
  FACTS_REGISTER(BFDec_Arithmetic);
  FACTS_REGISTER(BFDec_Comparison);
  FACTS_REGISTER(BFDec_StateQueries);
  FACTS_REGISTER(BFDec_Math);
  FACTS_REGISTER(BFDec_Conversions);
  FACTS_REGISTER(BFDec_Precision);
  FACTS_REGISTER(BFDec_RefCounting);
  FACTS_REGISTER(BFDec_Stream);
  FACTS_REGISTER(BFDec_IntBoundaries);
  FACTS_REGISTER(BFDec_ExtremeValues);
  FACTS_REGISTER(BFDec_PowersOf2);
  FACTS_REGISTER(BFDec_DecimalEdgeCases);
}

FACTS_MAIN
