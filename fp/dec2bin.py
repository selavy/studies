#!/usr/bin/env python

import readline
import bincvt


readline.parse_and_bind('tab: complete')
try:
    while 1:
        s = input('>> ').strip()
        if not s:
            continue
        print(f"{s} => {bincvt.dec2bin(s)}")
except (EOFError, KeyboardInterrupt):
    pass
