#  https://graphics.stanford.edu/~seander/bithacks.html#NextBitPermutation
def selector(values, setBits):
    maxBits = len(values)

    def select(v):
        out = []
        for i in range(maxBits):
            if (v & (1 << i)):
                out.append(values[i])

        return out

    v = (2 ** setBits) - 1
    endState = v << (maxBits - setBits)

    yield select(v)
    while v != endState:
        t = (v | (v - 1)) + 1
        v = t | ((((t & (-t % (1 << maxBits))) // (v & (-v % (1 << maxBits)))) >> 1) - 1)
        yield select(v)


def normalize(perm):
    ref = sorted(perm)
    return [ref.index(x) for x in perm]


def contains_pattern(perm, patt):
    if len(patt) > len(perm):
        return False

    for p in selector(perm, len(patt)):
        if normalize(p) == patt:
            return True

    return False


if __name__ == '__main__':
    print(contains_pattern(
        [14, 12, 6, 10, 0, 9, 1, 11, 13, 16, 17, 3, 7, 5, 15, 2, 4, 8],
        [3, 0, 1, 2]))
    print(True)
