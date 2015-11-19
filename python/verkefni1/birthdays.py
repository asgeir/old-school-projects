def birthdays(lines):
    days = {}
    for line in lines.split('\n'):
        line = line.strip()
        people = days.setdefault(line[:4], [])
        people.append(line)
    for date in days.values():
        date.sort()
    grouped = [tuple(x) for x in days.values() if len(x) > 1]
    grouped.sort()
    return grouped
