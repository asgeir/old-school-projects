def palindrome(value, base):
    def baseN(value, base, numerals="0123456789abcdefghijklmnopqrstuvwxyz"):
        if value < 0:
            value = value % (1 << 32)
        strValue = []
        while value:
            strValue.insert(0, numerals[value % base])
            value = value // base
        return strValue
    def is_palindrome(string):
        length = len(string)
        if length < 2 and length >= 0:
            return True
        if string[0] == string[-1]:
            return is_palindrome(string[1:-1])
        return False
    return (base > 0) and is_palindrome(baseN(value, base, numerals=range(1 << 32)))
