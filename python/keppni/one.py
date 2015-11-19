duplicates = lambda l: list({x: 1 for i,x in enumerate(l) if x in l[i+1:]}.keys())
