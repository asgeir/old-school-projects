def genops(length):
    state = [0] * length
    endState = [2] * length

    while state != endState:
        yield [['', '+', '-'][x] for x in reversed(state)]

        for i in range(length):
            newState = state[i] + 1
            state[i] = newState % 3

            if (newState > 2) and (i < (length-1)):
                state[i+1] += newState // 3
            else:
                break

    yield [['', '+', '-'][x] for x in reversed(state)]


def insert_operators(eqn, target):
    for ops in genops(len(eqn) - 1):
        ops.append('')

        tmpEqn = []
        for i in range(len(eqn)):
            tmpEqn.append(str(eqn[i]))
            tmpEqn.append(ops[i])

        tmpEqn = ''.join(tmpEqn)
        if eval(tmpEqn) == target:
            return tmpEqn + '={0}'.format(target)

    return None


if __name__ == '__main__':
    print(insert_operators([14, 8, 2, 17, 5, 9], 83))
    print('expected something')
    print()
    print(insert_operators([34, 9, 82, 21, 32], 32850))
    print('expected something')
    print()
    print(insert_operators([1, 2, 3], 5))
    print('expected None')
