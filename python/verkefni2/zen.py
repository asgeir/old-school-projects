import re

TOKENIZE = re.compile(
    r'''
    (?P<tag>\w+)
    (?:\*
        (?P<multiplier>\d+)
    )?
    (?:(?P<operator>[>+])|$)
    ''', re.VERBOSE)


def zen_token_expand(tokens):
    if not tokens:
        return []

    tag, mul, op = tokens[0]
    if mul:
        mul = int(mul)

    line = []
    if op == '>':
        nestedTags = zen_token_expand(tokens[1:])
        line = ['<' + tag + '>'] + nestedTags + ['</' + tag + '>']

        if mul:
            line = line * mul
    elif op == '+' or op == '':
        siblingTags = zen_token_expand(tokens[1:])
        line = ['<' + tag + '>', '</' + tag + '>']

        if mul:
            line = line * mul

        line += siblingTags

    return line


def zen_expand(template):
    lines = zen_token_expand(TOKENIZE.findall(template))
    return ''.join(lines)

if __name__ == '__main__':
    print(zen_expand('a+div+p*3'))
    print()
    print(zen_expand('dd'))
    print()
    print(zen_expand('table>tr*3>td*2'))
    print()
    print(zen_expand('div*3>p*2+ul>li*4>a+p*2>a'))
