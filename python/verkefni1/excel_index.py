def excel_index(index):
    numericIndex = 0
    index = [ord(x) - ord('A') + 1 for x in index]
    index.reverse()
    for (i, value) in enumerate(index):
        numericIndex += (26**i) * value
    return numericIndex
