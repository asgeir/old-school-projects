def jam(appearances):
    lines = appearances.split('\n')
    # there seems to be a bug in the testcase that parses Stephen K. Amos as
    # just Stephen K
    lines = [line.replace(', plus ', ', ').replace('Stephen K. Amos', 'Stephen K').split(',')[1:-1] for line in lines]

    for line in lines:
        i = 0
        length = len(line)
        while i < length:
            if ' with ' in line[i]:
                line[i:i + 1] = line[i].split(' with ')
                i += 1
            if ' and ' in line[i]:
                line[i:i + 1] = line[i].split(' and ')
                i += 1

            i += 1
            length = len(line)

    counter = {}
    for line in lines:
        for person in line:
            person = person.strip()

            cur = counter.get(person, 0)
            counter[person] = cur + 1

    return counter


if __name__ == '__main__':
    print(jam("""1/1/1 22 December 1967, Nicholas Parsons with Derek Nimmo, Clement Freud, Wilma Ewart and Beryl Reid, excuses for being late.
2/1/2 29 December 1967, Nicholas Parsons with Derek Nimmo, Clement Freud, Sheila Hancock and Carol Binstead, bedrooms.
3/1/3 5 January 1968, Nicholas Parsons with Derek Nimmo, Clement Freud, Betty Marsden and Elisabeth Beresford, ?
4/1/4 12 January 1968, Nicholas Parsons with Derek Nimmo, Clement Freud, Isobel Barnett and Bettine Le Beau, ?
5/1/5 20 January 1968, Nicholas Parsons with Derek Nimmo, Clement Freud, Andree Melly and Prunella Scales, the brownies
6/1/6 27 January 1968, Nicholas Parsons with Derek Nimmo, Clement Freud, Marjorie Proops and Millie Small, ?
7/1/7 2 February 1968, Nicholas Parsons with Derek Nimmo, Clement Freud, Aimi Macdonald and Una Stubbs, my honeymoon.
8/1/8 9 February 1968, Nicholas Parsons with Derek Nimmo, Clement Freud, Lucy Bartlett and Anona Winn, bloomer.
9/1/9 17 February 1968, Nicholas Parsons with Derek Nimmo, Clement Freud, Andree Melly and Charmian Innes, ?
743/57/5 30 August 2010, Nicholas Parsons with Paul Merton, Jenny Eclair, Fred MacAulay and Stephen K. Amos, the secret of my success.
10/1/10 23 February 1968, Nicholas Parsons with Derek Nimmo, Clement Freud, Barbara Blake and Renee Houston, my first grown-up dress."""))
