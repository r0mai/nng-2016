#!/usr/bin/env python
from __future__ import print_function
import sys


class Parcel:
    def __init__(self, rows, cols, layout):
        self.rows = rows
        self.cols = cols
        self.layout = layout
        self.history = []
        self.rewrite()

    def rewrite(self):
        for i, v in enumerate(self.layout):
            if v != 1:
                continue
            row, col = divmod(i, self.cols)
            if (row == 0 or col == 0 or row + 1 == self.rows
                    or col + 1 == self.cols):
                continue
            self.layout[i] = 5

    def neighbors(self, row, col):
        assert(row >= 0 and row < self.rows)
        assert(col >= 0 and col < self.cols)
        if row > 0:
            yield (row-1, col)
        if col > 0:
            yield (row, col-1)
        if col+1 < self.cols:
            yield (row, col+1)
        if row+1 < self.rows:
            yield (row+1, col)

    def _pos(self, row, col):
        assert(row >= 0 and row < self.rows)
        assert(col >= 0 and col < self.cols)
        return row*self.cols + col

    def height(self, row, col):
        return self.layout[self._pos(row, col)]

    def unbuild(self, row, col):
        assert(self.height(row, col) == 1)
        self.layout[self._pos(row, col)] = 0
        self.history.append((row, col))
        for nr, nc in self.neighbors(row, col):
            h = self.height(nr, nc)
            assert(h == 0 or h > 1)
            self.layout[self._pos(nr, nc)] = max(h - 1, 0)

    def show_layout(self):
        for row in xrange(self.rows):
            p = row * self.cols
            print(self.layout[p:p + self.cols])

    def show_history(self):
        for (row, col) in reversed(self.history):
            print(row, col)

    def demolish(self):
        try:
            p = self.layout.index(1)
            row, col = divmod(p, self.cols)
            print('--')
        except ValueError:
            try:
                # return False
                p = self.layout.index(5)
                row, col = divmod(p, self.cols)
                print('-- {} {}'.format(row, col))
                self.layout[self._pos(row, col)] = 1
            except ValueError:
                return False
        self.unbuild(row, col)
        return True


def load(fname):
    t = []
    with open(fname) as f:
        rows, cols = map(int, f.readline().split())
        for i in xrange(rows):
            xs = map(int, f.readline().split())
            assert(len(xs) == cols)
            t += xs
    return Parcel(rows, cols, t)


if __name__ == '__main__':
    p = load(sys.argv[1])
    p.show_layout()
    while p.demolish():
        p.show_layout()

    # p.show_history()
    # # p.show_layout()
