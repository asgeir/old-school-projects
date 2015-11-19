VALUES = '23456789TJQKA'


def all_same(values):
    base = values[0]
    for i in range(1, len(values)):
        if values[i] != base:
            return False

    return True


def rising_sequence(values):
    prev = values[0]
    for i in range(1, len(values)):
        if values[i] != (prev+1):
            return False

        prev = values[i]

    return True


def rank_hand(cards):
    cards = [(VALUES.find(card[0]), card[1]) for card in cards]

    values = [value for value, suit in cards]
    values.sort()

    suits = [suit for value, suit in cards]
    suits.sort()

    isFlush = all_same(suits)
    isStraight = rising_sequence(values)

    if isStraight and isFlush:
        if values[0] == VALUES.find('T'):
            return 9

        return 8

    isFourOfAKind = all_same(values[:4]) or all_same(values[1:])
    if isFourOfAKind:
        return 7

    isFullHouse = all_same(values[:3]) and all_same(values[3:])
    isFullHouse = isFullHouse or (all_same(values[:2]) and all_same(values[2:]))
    if isFullHouse:
        return 6

    if isFlush:
        return 5

    if isStraight:
        return 4

    isThreeOfAKind = all_same(values[:3]) or all_same(values[2:])
    if isThreeOfAKind:
        return 3

    pairs = []
    for i in range(len(values) - 1):
        if values[i] == values[i+1]:
            try:
                pairs.index(values[i])
            except ValueError:
                pairs.append(values[i])

    if pairs:
        return len(pairs)

    return 0


if __name__ == '__main__':
    print('expect 1 and got {0}'.format(rank_hand(['3D', '2H', '3C', 'QS', '8D'])))
    print('expect 6 and got {0}'.format(rank_hand(['KD', 'KH', 'KC', 'TS', 'TD'])))
    print('expect 9 and got {0}'.format(rank_hand(['JD', 'KD', 'TD', 'QD', 'AD'])))
