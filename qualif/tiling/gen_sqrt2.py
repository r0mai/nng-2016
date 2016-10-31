#!/usr/bin/env python
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


def F(n):
    return int(n * (sqrt2 - 1))


def gen():
    n = 10**10000 + 1
    count = 0
    with open('x_sqrt_seq.txt', 'wb') as f:
        while n > 0:
            count += 1
            print(count)
            print(n, file=f)
            n = F(n)


if __name__ == '__main__':
    pass
