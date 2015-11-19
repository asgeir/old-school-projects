def countdown(filePath, chars):
    words = [
        w.replace('\r', '').replace('\n', '')
        for w in open(filePath, 'r').readlines()]
    words = [w for w in words if len(w) >= 4]

    result = []
    for w in words:
        wc = [c for c in chars]

        candidate = True
        for c in w:
            try:
                i = wc.index(c)
                wc.pop(i)
            except ValueError:
                candidate = False
                break

        if candidate:
            result.append(w)

    return sorted(result)
