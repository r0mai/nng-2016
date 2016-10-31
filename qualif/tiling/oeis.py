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

from __future__ import print_function
from fractions import Fraction
import math
import sys


def Sqrt(k):
    n = Fraction(k)
    x = Fraction(1)
    for i in xrange(14):
        x = (x + n / x) / Fraction(2)
    return x


sqrt2 = Sqrt(2)


def S(n):
    return n * (n + 1) / 2


def F(n):
    return int(n * (sqrt2 - 1))


def S1(n):
    if n == 0:
        return 0

    print('> {}'.format(len(str(n.numerator))), file=sys.stderr)
    fn = F(n)
    return 2 * S(n) + fn * n - S1(fn)


def S0(n):
    fn = F(n)
    return S(n) + fn * n - S1(fn)


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


if __name__ == '__main__':
    sys.setrecursionlimit(10000)
    # print(len(str(sqrt2.numerator)))
    print(oeis(10**100 + 1))
