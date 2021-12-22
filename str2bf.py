#!/bin/env python3
from os import stat_result
from sys import argv
import collections


def main():
    if len(argv) < 2:
        print(f"Usage: {argv[0]} <string>")
        exit(1)
    mclfs = factors(most_common_letter(argv[1]))
    mclfh = int(len(mclfs) - 3)
    print_bf(mclfs[mclfh], strton(argv[1]), mclfs[len(mclfs) - 1])


def print_bf(sn, narr, mc):
    print(f"{'+'*sn}[>{'+'*int(mc/sn)}<-]>", end="")
    for i in narr:
        if mc < i:
            print(f"{'+'*(i-mc)}", end="")
            mc += i - mc
        else:
            print(f"{'-'*(mc-i)}", end="")
            mc -= mc - i
        print(".")


def factors(n):
    lst = []
    for i in range(1, n + 1):
        if n % i == 0:
            lst.append(i)
    return lst


def most_common_letter(string):
    return ord(collections.Counter(string).most_common(1)[0][0])


def strton(string):
    retlst = []
    for i in string:
        retlst.append(ord(i))
    return retlst


def average(string):
    ch_sum = 0
    for i in string:
        ch_sum += ord(i)
    return int(ch_sum / len(string))


if __name__ == "__main__":
    main()
