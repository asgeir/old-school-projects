import re

PARSER = re.compile(r'''
    (?:
        (?: ^ | ,)
        (?:
            (?:" ([^"]*) ") |
            ([^",$]*)
        )
    )
''', re.X)


def process_csv(filePath):
    lines = [line.replace('\n', '') for line in open(filePath, 'r').readlines()]
    lines = [PARSER.findall(line) for line in lines]
    lines = [[tok1 or tok2 for tok1, tok2 in line] for line in lines]

    shoppers = {}
    for line in lines:
        name = line[0]
        itemCount = int(line[2])
        itemPrice = int(line[3])

        shoppers.setdefault(name, 0)
        shoppers[name] += itemCount * itemPrice

    return shoppers


if __name__ == '__main__':
    print(process_csv('./process_csv_example_2.txt'))
