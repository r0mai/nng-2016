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
import math

def S(n):
    return n * (n + 1) / 2


def F(n):
    return int(n * (math.sqrt(2) - 1))


def S1(n):
    if n == 0:
        return 0

    return 2 * S(n) + F(n) * n - S1(F(n))


def S0(n):
    return S(n) + F(n) * n - S1(F(n))


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
    for i in xrange(2, 10):
        print('10^{}:'.format(i), oeis(10 ** i))
