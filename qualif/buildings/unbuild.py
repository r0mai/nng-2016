 #!/usr/bin/env python
from __future__ import print_function


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

    def square(self, row, col, force=False):
        if (force or (
                row >= 0 and col >= 0 and
                row + 1 < self.rows and col + 1 < self.cols)):
            yield (row, col)
            yield (row, col + 1)
            yield (row + 1, col)
            yield (row + 1, col + 1)

    def _pos(self, row, col):
        assert(row >= 0 and row < self.rows)
        assert(col >= 0 and col < self.cols)
        return row*self.cols + col

    def height(self, row, col):
        return self.layout[self._pos(row, col)]

    def reset(self, row, col, value):
        self.layout[self._pos(row, col)] = value

    def unbuild(self, row, col):
        assert(self.height(row, col) == 1)
        self.reset(row, col, 0)
        self.history.append((row, col))
        for nr, nc in self.neighbors(row, col):
            h = self.height(nr, nc)
            assert(h == 0 or h > 1)
            self.reset(nr, nc, max(h - 1, 0))

    def show_layout(self):
        for row in xrange(self.rows):
            p = row * self.cols
            print(' '.join(map(str, self.layout[p:p + self.cols])))

    def show_history(self):
        for (row, col) in reversed(self.history):
            print(row, col)

    def unrewrite(self):
        for i, v in enumerate(self.layout):
            if v == 5 and not self.bad_pattern(i / self.cols, i % self.cols):
                self.layout[i] = 1
                return True
        return False

    def bad_pattern(self, row, col):
        for (br, bc) in self.square(row - 1, col - 1, force=True):
            count = 0
            for (sr, sc) in self.square(br, bc):
                if self.height(sr, sc) == 2:
                    count += 1
            if count >= 3:
                return True
        return False

    def rank(self):
        count = 0
        hsum = 0
        for row in xrange(self.rows):
            for col in xrange(self.cols):
                h = self.height(row, col)
                hsum += h
                if h == 0:
                    continue
                count += 1
                if row + 1 < self.rows and self.height(row + 1, col) > 0:
                    count += 1
                if col + 1 < self.cols and self.height(row, col + 1) > 0:
                    count += 1
        return count, hsum, abs(hsum - count) / 4

    def demolish(self):
        mark = ''
        try:
            p = self.layout.index(1)
        except ValueError:
            return self.erode()

        row, col = divmod(p, self.cols)
        # print('-- {} {}{}'.format(row+1, col+1, mark))
        self.unbuild(row, col)
        return True

    def positions(self):
        for row in xrange(self.rows):
            for col in xrange(self.cols):
                yield (row, col)

    def count_neighbors(self, row, col):
        count = 0
        for nr, nc in self.neighbors(row, col):
            if self.height(nr, nc) > 0:
                count += 1
        return count

    def erode(self):
        elim = set()
        for row, col in self.positions():
            h = self.height(row, col)
            if h == 0:
                continue
            count = self.count_neighbors(row, col)
            if h < 5 and h == count + 1:
                elim.add((row, col))

        while elim:
            (row, col) = elim.pop()
            self.reset(row, col, 0)
            for nr, nc in self.neighbors(row, col):
                h = self.height(nr, nc)
                if h == 0:
                    continue
                count = self.count_neighbors(nr, nc)
                if h < 5 and h == count + 1:
                    elim.add((nr, nc))

        res = False
        for row, col in self.positions():
            h = self.height(row, col)
            if h != 5:
                continue
            count = self.count_neighbors(row, col)
            if count < 4:
                self.reset(row, col, 1)
                res = True
        return res


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
    p = load('test.map')
    while p.demolish():
        pass

    p.show_layout()

