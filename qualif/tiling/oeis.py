#!/usr/bin/env python
#
# Fast computing A022775
#
#
# A0(n) := sqrt(2) * n
# A1(n) := (sqrt(2) + 1) * n
#
# S0(n) := sum(floor(A0(1..n)))
# S1(n) := sum(floor(A1(1..n)))
#
# S(n) := n * (n + 1) / 2
# F(n) := floor(n * (sqrt(2) - 1))
#
# Recursive formula:
#  S0(n) = S(n) + F(n) * n - S1(F(n))
#  S1(n) = 2 * S(n) + F(n) * n - S1(F(n))
#
#
# Errors for sqrt:
#
#  a: series interpolating sqrt(n)
#  e: error
#  e(i+1) = -e(i)^2 / 2 * (lim(a) - e(i))
#

from __future__ import print_function
from fractions import Fraction
import math
import sys


def Sqrt(k):
    n = Fraction(k)
    x = Fraction(1)
    # error of p iterations is less than 10^-(2^p)
    s = [x]
    for i in xrange(14):
        x = (x + n / x) / Fraction(2)
        s.append(x)
    return x


def LoadF():
    m = {}
    n = 0
    with open('sqrt_seq.txt') as f:
        n = int(f.readline())
        for fn in f:
            fn = int(fn)
            m[n] = fn
            n = fn
    print('Loaded', file=sys.stderr)
    return m


sqrt2 = Sqrt(2)
memo = LoadF()


def S(n):
    return n * (n + 1) / 2


def F(n):
    if n in memo:
        return memo[n]
    return int(n * (sqrt2 - 1))


def S1(n):
    if n == 0:
        return 0

    # print('> {}'.format(len(str(n.numerator))), file=sys.stderr)
    fn = F(n)
    return 2 * S(n) + fn * n - S1(fn)


def S1u(n):
    s = 0
    mul = 1
    while n > 0:
        print('> {}'.format(len(str(n.numerator))), file=sys.stderr)
        fn = F(n)
        s += mul * (2 * S(n) + fn * n)
        mul = -mul
        n = fn
    return s


def S0(n):
    fn = F(n)
    return S(n) + fn * n - S1u(fn)


def S0a(n):
    # slow computing for S0()
    return int(sum([math.floor(math.sqrt(2) * i) for i in xrange(1, n+1)]))


def oeis(n):
    if n == 0:
        return 1
    return n + S0(n-1)


def _oeis(n):
    # slow A022775
    return 1 + int(sum([math.ceil(i * math.sqrt(2)) for i in xrange(n)]))


def solve(p):
    with open('solution_{}.txt'.format(p), 'wb') as f:
        print(oeis(10**p + 1), file=f)


if __name__ == '__main__':
    solve(10000)
