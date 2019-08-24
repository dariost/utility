#!/usr/bin/env pypy3

from functools import lru_cache
import sys

def tuplify(*args):
    return tuple(args)

def identity(x):
    return x

class Terminal:
    epsilon_symbol = object()

    def __init__(self, x):
        self.symbol = x

    def __repr__(self):
        return str(self.symbol) if self.symbol != Terminal.epsilon_symbol else "ε"

    def __eq__(self, other):
        return isinstance(other, Terminal) and self.symbol == other.symbol

    def __hash__(self):
        return hash(self.symbol)

Terminal.epsilon = Terminal(Terminal.epsilon_symbol)

class NonTerminal:
    def __init__(self, x):
        self.symbol = x
        self.production = []
        self.reduction = []

    def __repr__(self):
        return self.symbol

    def add(self, *args, f = tuplify):
        assert all(isinstance(x, Terminal) or isinstance(x, NonTerminal) for x in args)
        self.production.append(list(args))
        self.reduction.append(f)

    def rules(self):
        return self._rules(set())

    def _rules(self, visited):
        if self in visited:
            return
        visited.add(self)
        if len(self.production) == 0:
            return
        to_visit = set()
        s = str(self) + " -> "
        for p in self.production:
            for o in p:
                if isinstance(o, NonTerminal) and o not in visited:
                    to_visit.add(o)
                s += str(o) + " "
            s += "| "
        s = s[:-3]
        for i in to_visit:
            if i not in visited:
                s += "\n" + i._rules(visited)
        return s

    def gather(self):
        return self._gather(set())

    def _gather(self, visited):
        if self in visited:
            return (set(), set())
        visited.add(self)
        terminals = set()
        nonterminals = set([self])
        additional = set()
        for p in self.production:
            for r in p:
                if isinstance(r, Terminal):
                    terminals.add(r)
                else:
                    nonterminals.add(r)
        for nt in nonterminals:
            if nt not in visited:
                (rt, rnt) = nt._gather(visited)
                terminals |= rt
                additional |= rnt
        return (terminals, nonterminals | additional)


class Grammar:
    def __init__(self, S):
        self.start = S
        # START
        new_start = NonTerminal(self.start.symbol + "0")
        (term, nonterm) = self.start.gather()
        for nt in nonterm:
            for p in nt.production:
                for j, r in enumerate(p):
                    if self.start == r:
                        p[j] = new_start
        new_start.add(self.start, f = identity)
        self.start = new_start
        # TERM
        index = 0
        (term, nonterm) = self.start.gather()
        term_cache = dict()
        for nt in nonterm:
            for p in nt.production:
                for j, r in enumerate(p):
                    if isinstance(r, Terminal) and r is not Terminal.epsilon:
                        if r not in term_cache:
                            x = NonTerminal("X" + str(index))
                            index += 1
                            x.add(r, f = identity)
                            term_cache[r] = x
                        p[j] = term_cache[r]
        # BIN
        index = 0
        (term, nonterm) = self.start.gather()
        for nt in nonterm:
            for i, p in enumerate(nt.production):
                if len(p) > 2:
                    new_symbols = [NonTerminal("Y" + str(i)) for i in range(index, index + len(p) - 2)]
                    index += len(new_symbols)
                    new_symbols[0].add(p[-2], p[-1], f = tuplify)
                    p.pop(); p.pop()
                    for j in range(1, len(new_symbols)):
                        new_symbols[j].add(p.pop(), new_symbols[j - 1], f = lambda x, y: tuplify(x, *y))
                    assert len(p) == 1
                    def f0(f):
                        return lambda x, y: f(x, *y)
                    nt.reduction[i] = f0(nt.reduction[i])
                    p.append(new_symbols[-1])
        # DEL
        (term, nonterm) = self.start.gather()
        nullable = set()
        last_size = -1
        while last_size < len(nullable):
            last_size = len(nullable)
            for nt in nonterm:
                for p in nt.production:
                    if len(p) == 1 and p[0] == Terminal.epsilon:
                        nullable.add(nt)
                    if all(x in nullable for x in p):
                        nullable.add(nt)
        for nt in nonterm:
            have = set()
            to_add = []
            for i, p in enumerate(nt.production):
                have.add(tuple(p))
                if len(p) == 2:
                    def f1(f):
                        return lambda x: f(None, x)
                    def f2(f):
                        return lambda x: f(x, None)
                    if p[0] in nullable:
                        to_add.append(([p[1]], f1(nt.reduction[i])))
                    if p[1] in nullable:
                        to_add.append(([p[0]], f2(nt.reduction[i])))
            for i in to_add:
                if tuple(i[0]) not in have:
                    have.add(tuple(i[0]))
                    nt.production.append(i[0])
                    nt.reduction.append(i[1])
        for nt in nonterm:
            to_remove = set()
            for i, p in enumerate(nt.production):
                if len(p) == 1 and p[0] == Terminal.epsilon:
                    to_remove.add(i)
            if len(to_remove) > 0:
                new_prod = []
                new_redu = []
                exnum = len(nt.production)
                for i in range(exnum):
                    if i not in to_remove:
                        new_prod.append(nt.production[i])
                        new_redu.append(nt.reduction[i])
                nt.production = new_prod
                nt.reduction = new_redu
        # UNIT
        (term, nonterm) = self.start.gather()
        ok = False
        while not ok:
            ok = True
            for nt in nonterm:
                to_remove = set()
                to_add = dict()
                have = set()
                for i, p in enumerate(nt.production):
                    if len(p) == 1 and isinstance(p[0], NonTerminal):
                        to_remove.add(i)
                        for j, q in enumerate(p[0].production):
                            tq = tuple(q)
                            a = nt.reduction[i]
                            b = p[0].reduction[j]
                            if q != p and tq not in to_add:
                                if len(q) == 1:
                                    def f1(a, b):
                                        return lambda x: a(b(x))
                                    to_add[tq] = f1(nt.reduction[i], p[0].reduction[j])
                                elif len(q) == 2:
                                    def f1(a, b):
                                        return lambda x, y: a(b(x, y))
                                    to_add[tq] = f1(nt.reduction[i], p[0].reduction[j])
                    else:
                        have.add(tuple(p))
                if len(to_remove) > 0:
                    new_prod = []
                    new_redu = []
                    exnum = len(nt.production)
                    for i in range(exnum):
                        if i not in to_remove:
                            new_prod.append(nt.production[i])
                            new_redu.append(nt.reduction[i])
                    nt.production = new_prod
                    nt.reduction = new_redu
                for i in to_add:
                    if len(i) < 2:
                        ok = False
                    if i not in have:
                        have.add(i)
                        nt.production.append(list(i))
                        nt.reduction.append(to_add[i])

    def rules(self):
        return self.start.rules()

    def parse(self, x):
        return ParseResult(self, [Terminal(y) for y in x])

class ParseResult:
    def __init__(self, g, x):
        self.grammar = g
        self.x = x
        self.nt2i = dict()
        self.i2nt = dict()
        self.table = []
        self.OK = object()
        n = len(x)
        (term, nonterm) = g.start.gather()
        index = 0
        for nt in nonterm:
            self.nt2i[nt] = index
            self.i2nt[index] = nt
            index += 1
        table = self.table
        for i in range(n + 1):
            table.append([])
            for j in range(n):
                table[-1].append([])
                for k in range(index):
                    table[-1][-1].append(set())
        for i in range(n):
            for nt in nonterm:
                for pi, p in enumerate(nt.production):
                    if len(p) == 1 and isinstance(p[0], Terminal):
                        if x[i] == p[0]:
                            table[1][i][self.nt2i[nt]].add((p[0].symbol, pi))
        for l in range(2, n + 1):
            for s in range(n - l + 1):
                for p in range(1, l):
                    for nt in nonterm:
                        for prod_index, prod in enumerate(nt.production):
                            if len(prod) == 2:
                                a = self.nt2i[nt]
                                b = self.nt2i[prod[0]]
                                c = self.nt2i[prod[1]]
                                if len(table[p][s][b]) > 0 and len(table[l - p][s + p][c]) > 0:
                                    table[l][s][a].add(((p, s, b), (l - p, s + p, c), prod_index))

    def count_parses(self):
        @lru_cache(maxsize=None)
        def count(a, b, c):
            if a == 1:
                return len(self.table[a][b][c])
            result = 0
            for i in self.table[a][b][c]:
                result += count(*i[0]) * count(*i[1])
            return result
        return count(len(self.x), 0, self.nt2i[self.grammar.start])

    def parse_trees(self):
        def tree(a, b, c):
            if a == 1:
                return [self.i2nt[c].reduction[i[1]](i[0]) for i in self.table[a][b][c]]
            result = []
            for i in self.table[a][b][c]:
                x = tree(*i[0])
                for j in x:
                    y = tree(*i[1])
                    for k in y:
                        result.append(self.i2nt[c].reduction[i[2]](j, k))
            return result
        return tree(len(self.x), 0, self.nt2i[self.grammar.start])

    def parse_tree(self):
        assert self.count_parses() == 1
        return self.parse_trees()[0]

if __name__ == "__main__":
    sys.setrecursionlimit(2**31-1)
    S = NonTerminal("S")
    B = NonTerminal("B")
    C = NonTerminal("C")
    S.add(C, Terminal("a"))
    S.add(Terminal("d"))
    B.add(Terminal.epsilon)
    B.add(Terminal("a"))
    C.add(Terminal("b"))
    C.add(B, C, Terminal("b"))
    C.add(Terminal("b"), Terminal("b"))
    print(S.rules())
    print()
    G = Grammar(S)
    print(G.rules())
    p = G.parse("a" * 20 + "b" * 300 + "a")
    print()
    print("Done CYK")
    print("Number of possible parses: {}".format(p.count_parses()))
    #print("Possible parses: {}".format(p.parse_trees()))
