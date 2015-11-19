from datetime import date as d


def valid(n):
    if len(n) != 10 or (n[-1] not in '90'):
        return False

    try:
        c = 2000
        if n[-1] == '9':
            c = 1900

        d(c + int(n[4:6]), int(n[2:4]), int(n[:2]))
    except ValueError:
        return False

    s = 0
    for i in range(7, -1, -1):
        s += (2 + i % 6) * int(n[i])

    return (11 - s % 11) == int(n[-2])
