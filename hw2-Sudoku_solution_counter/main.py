import sys
import math
from pyeda.inter import *

def main():
    global N, n
    global X
    global F
    global G

    sys.setrecursionlimit(10000)

    fin = sys.argv[1]
    fout = sys.argv[2]

    # Sudoku parsing
    with open(fin, "r") as input:
        G = [[int(n) for n in line.split()] for line in input]

    N = len(G)
    n = int(math.sqrt(N))
    X = exprvars('x', (1, N+1), (1, N+1), (1, N+1))

    # Prefill cell
    for row in range(N):
        for col in range(N):
            if G[row][col] != 0:
                X[row+1, col+1, G[row][col]] = expr(1)

    # Value constraints
    V = And(*[
             And(*[
                 OneHot(*[ X[r, c, v]
                     for v in range(1, N+1) ])
                 for c in range(1, N+1) ])
             for r in range(1, N+1) ])

    # Row constraints
    R = And(*[
            And(*[
                OneHot(*[ X[r, c, v]
                    for c in range(1, N+1) ])
                for v in range(1, N+1) ])
            for r in range(1, N+1) ])

    # Col constraints
    C = And(*[
            And(*[
                OneHot(*[ X[r, c, v]
                    for r in range(1, N+1) ])
                for v in range(1, N+1) ])
            for c in range(1, N+1) ])

    # Box constraints
    B = And(*[
            And(*[
                OneHot(*[ X[n*br+r, n*bc+c, v]
                    for r in range(1, n+1) for c in range(1, n+1) ])
                for v in range(1, N+1) ])
            for br in range(n) for bc in range(n) ])

    # Combine all constraints
    S = And(V, R, C, B)

    # Write answer
    with open(fout, "w") as output:
        output.write(str(S.satisfy_count()))

if __name__ == '__main__':
    main()
