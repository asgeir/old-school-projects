import glob
import os.path
import re


ENTRY = re.compile(r'^\s*set\s+([^\s]+)\s+([^\n]+)\n$')


def parse_submissions(directory):
    accepted = []

    fileNames = glob.glob(os.path.join(directory, '*', 'data.tcl'))
    for name in fileNames:
        lines = [ENTRY.findall(line) for line in open(name, 'r').readlines()]
        lines = [line[0] for line in lines if line]

        props = dict(lines)
        try:
            if props['Classify'] == 'Accepted':
                accepted.append((int(props['Date']), props['Team'], props['Problem']))
        except KeyError:
            pass

    return [(team, problem) for date, team, problem in sorted(accepted)]


if __name__ == '__main__':
    print(parse_submissions('submissions'))
