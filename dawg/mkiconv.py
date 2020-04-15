#!/usr/bin/env python


N = 128
vals = [-1]*N
for i in range(N):
    ch = chr(i)
    if 'A' <= ch <= 'Z':
        vals[i] = i - ord('A')
    elif 'a' <= ch <= 'z':
        vals[i] = i - ord('a')


def fmtgrp(grp):
    return [f"{g:2d}" for g in grp]


groupsz = 16
grps = []
grp = []
for v in vals:
    if len(grp) >= groupsz:
        grps.append(fmtgrp(grp))
        grp = []
    grp.append(v)
if grp:
    grps.append(fmtgrp(grp))

print("""#pragma once

#include <cassert>

// Generated by mkiconv.py. Do not edit by hand.
""")


print(f"constexpr int iconv_table[{len(vals)}] = {{")
for grp in grps:
    print(f"    {', '.join(grp)},")
print("};")

print("""
constexpr int iconv(char c) noexcept {
#ifndef NDEBUG
    assert(('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z'));
    return iconv_table[static_cast<unsigned char>(c & 0x7Fu)];
#else
    return iconv_table[static_cast<unsigned char>(c)];
#endif
}
""")

