#!/usr/bin/env python3

#####################################################
#                                                   #
# License: Apache License 2.0                       #
# Author: Dario Ostuni <another.code.996@gmail.com> #
#                                                   #
#####################################################

import sys

def fatt(n):
    tmp = 1
    for i in range(1, n + 1):
        tmp *= i
    return tmp

def term(n):
    num = ((-1)**n)*fatt(4*n)*(21460*n+1123)
    den = (fatt(n)**4)*(14112**(2*n))
    return num, den
def mcd(a, b):
    if a < b:
        a, b = b, a
    while b != 0:
        a, b = b, a % b
    return a

if len(sys.argv) == 2:
    cfr = int(sys.argv[1])
else:
    cfr = int(input("Number of digits: "))
prec = cfr // 5 + 10
num = 0
den = 1
for i in range(prec):
    tmp_n, tmp_d = term(i)
    num = num * tmp_d + den * tmp_n
    den = den * tmp_d
    gdc = mcd(num, den)
    num //= gdc
    den //= gdc

num, den = den * 3528, num

num -= 3 * den
print("3.", end='')
for i in range(cfr):
    num *= 10
    print(num // den, end='')
    num %= den
print(flush=True)
