import operator
import re

MATCHER = re.compile(r'^[^d](?:[^\s]+\s+){4}(?P<size>[0-9]+)\s+(?:[^\s+]+\s+){3}(?P<name>.+)$', re.MULTILINE)

def process_ls(ls_output):
    matches = MATCHER.findall(ls_output)
    matches = [(int(size), name) for (size, name) in matches]
    matches.sort(key=operator.itemgetter(1))
    matches.sort(key=operator.itemgetter(0), reverse=True)
    return [name for size, name in matches]
