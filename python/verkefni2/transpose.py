def transpose(M):
    if not M or not M[0]:
        return M
    Mt = [[] for i in range(len(M[0]))]
    for line in M:
        for col, value in enumerate(line):
            Mt[col].append(value)
    return Mt

