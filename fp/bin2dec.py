#!/usr/bin/env python

import readline
import bincvt


readline.parse_and_bind('tab: complete')
try:
    while 1:
        s = input('>> ').strip()
        if s:
            print(f"{s} => {bincvt.bin2dec(s)}")
except (EOFError, KeyboardInterrupt):
    pass
