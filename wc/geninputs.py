#!/usr/bin/env python

from random import choice, shuffle
import sys

PROB_LETTER = 0.7
PROB_SPACE  = 0.98
PROB_LINE   = 1.0


if __name__ == '__main__':
    if len(sys.argv) == 2:
        max_size = int(sys.argv[1])
    else:
        max_size = 100000000
    letters = []
    for i in range(65, 91):
        letters.append(chr(i))
    for i in range(97, 123):
        letters.append(chr(i))
    for i in range(5):
        letters.append(' ')
    letters.append('\n')
    shuffle(letters)

    for i in range(max_size):
        sys.stdout.write(choice(letters))
