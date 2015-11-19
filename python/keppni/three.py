balanced = lambda p: sum([{'(':1,')':-1}[x] for x in p])  == 0
