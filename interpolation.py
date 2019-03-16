#!/usr/bin/env python3

import numpy as np
from numpy import linalg as la

def polynomial(coeff):
    A = np.matrix([[x**i for i in range(len(coeff))] for x, _ in coeff])
    b = np.matrix([[y for _, y in coeff]]).transpose()
    return la.solve(A, b).transpose().tolist()[0]

if __name__ == "__main__":
    print("Format: x0:y0,x1:y1,x2:y2")
    print("Insert values: ", end='')
    v = [tuple(map(float, p.split(":"))) for p in input().strip().split(",")]
    c = polynomial(v)
    print("Polynomial: ", end='')
    sol = ""
    for i in reversed(range(len(c))):
        if(c[i] == 0.0):
            continue
        sol += str(c[i])
        if i == 0:
            pass
        elif i == 1:
            sol += "x"
        else:
            sol += "x^" + str(i)
        if i != 0:
            sol += " + "
    if sol.endswith(" + "):
        sol = sol[:-3]
    if sol == "":
        sol = "0"
    print(sol)
