#!/usr/bin/env pypy3

import sys
from cyk import Terminal, NonTerminal, Grammar, identity
from abc import ABC, abstractmethod

class Expression(ABC):
    @abstractmethod
    def eval(self):
        pass

    @abstractmethod
    def __repr__(self):
        pass

class Number(Expression):
    def __init__(self, x):
        self.value = int(x)

    def eval(self):
        return self.value

    def __repr__(self):
        return "Number({})".format(self.value)

class Neg(Expression):
    def __init__(self, x):
        self.a = x

    def eval(self):
        return -self.a.eval()

    def __repr__(self):
        return "Neg({})".format(self.a)

class BinaryOperator(Expression):
    def __init__(self, a, b):
        self.a = a
        self.b = b

    @staticmethod
    @abstractmethod
    def op(a, b):
        pass

    def eval(self):
        return self.op(self.a.eval(), self.b.eval())

    def __repr__(self):
        return "{}({}, {})".format(self.__class__.__name__, self.a, self.b)

class Add(BinaryOperator):
    @staticmethod
    def op(a, b):
        return a + b

class Sub(BinaryOperator):
    @staticmethod
    def op(a, b):
        return a - b

class Mul(BinaryOperator):
    @staticmethod
    def op(a, b):
        return a * b

class Div(BinaryOperator):
    @staticmethod
    def op(a, b):
        return a // b

class Mod(BinaryOperator):
    @staticmethod
    def op(a, b):
        return a % b

class Pow(BinaryOperator):
    @staticmethod
    def op(a, b):
        return int(a ** b)

if __name__ == "__main__":
    assert len(sys.argv) == 2
    DIGIT = NonTerminal("DIGIT")
    for i in range(10):
        DIGIT.add(Terminal(str(i)), f = identity)
    WHITESPACE = NonTerminal("WHITESPACE")
    for i in (" ", "\t", "\n", "\r"):
        WHITESPACE.add(Terminal(i))
    BREAK = NonTerminal("BREAK")
    BREAK.add(Terminal.epsilon)
    BREAK.add(WHITESPACE, BREAK)
    INTEGER = NonTerminal("INTEGER")
    INTEGER.add(DIGIT, f = identity)
    INTEGER.add(DIGIT, INTEGER, f = lambda x, y: x + y)
    NUMBER = NonTerminal("NUMBER")
    NUMBER.add(INTEGER, BREAK, f = lambda x, y: Number(x))
    E = NonTerminal("E")
    E0 = NonTerminal("E0")
    E1 = NonTerminal("E1")
    E2 = NonTerminal("E2")
    E3 = NonTerminal("E3")
    E4 = NonTerminal("E4")
    E5 = NonTerminal("E5")
    E.add(BREAK, E0, f = lambda a, b: b)
    E0.add(E1, f = identity)
    E1.add(E1, Terminal("+"), BREAK, E2, f = lambda a, b, c, d: Add(a, d))
    E1.add(E1, Terminal("-"), BREAK, E2, f = lambda a, b, c, d: Sub(a, d))
    E1.add(E2, f = identity)
    E2.add(E2, Terminal("*"), BREAK, E3, f = lambda a, b, c, d: Mul(a, d))
    E2.add(E2, Terminal("/"), BREAK, E3, f = lambda a, b, c, d: Div(a, d))
    E2.add(E2, Terminal("%"), BREAK, E3, f = lambda a, b, c, d: Mod(a, d))
    E2.add(E3, f = identity)
    E3.add(E4, Terminal("*"), Terminal("*"), BREAK, E3, f = lambda a, b, c, d, e: Pow(a, e))
    E3.add(E4, f = identity)
    E4.add(Terminal("("), BREAK, E0, Terminal(")"), BREAK, f = lambda a, b, c, d, e: c)
    E4.add(Terminal("-"), BREAK, E4, f = lambda a, b, c: Neg(c))
    E4.add(E5, f = identity)
    E5.add(NUMBER, f = lambda x: x)
    #print(E.rules())
    G = Grammar(E)
    #print()
    #print(G.rules())
    #print()
    p = G.parse(sys.argv[1])
    print("Number of possible parsings: {}".format(p.count_parses()))
    ast = p.parse_tree()
    print("Parse: {}".format(ast))
    print("Eval: {}".format(ast.eval()))
