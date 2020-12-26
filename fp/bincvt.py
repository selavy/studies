#!/usr/bin/env python3

import math


def interp16(s: str) -> str:
    s = s.strip()
    s = s.replace(' ', '')
    if s.startswith('0b'):
        s = s[2:]
    if len(s) != 16:
        raise ValueError('Invalid length for binary16')
    sign = int(s[0])
    exponent = s[1:6]
    mantissa = s[6:16]
    print(f"Sign:     {sign}")
    print(f"Exponent: {exponent}")
    print(f"Mantissa: {mantissa}")
    eint = int(exponent, 2)
    mint = int(mantissa, 2)
    s = '+' if sign != 0 else '-'
    if   eint == 0b00000 and mint == 0:
        print(f"{s}zero")
    elif eint == 0b11111 and mint == 0:
        print(f"{s}infinity")
    elif eint == 0b11111 and mint != 0:
        print("NaN")
    elif eint == 0b00000 and mint != 0:
        m = mint / 2**10 * 2**-14
        print(f"subnormal: (-1)^{sign} x 0.{mantissa} x 2^-14 = {m}")
    else:
        m = (1 + mint / 2**10) * 2**(eint - 15)
        print(f"(-1)^{sign} x 1.{mantissa} x 2^{eint - 15} = {m}")


def dec2bin(s: str) -> str:
    """
    Convert string form of a decimal number its binary form.

    TODO: need to investigate more where this breaks down, e.g. 0.1
    """
    s = s.strip()
    f = float(s)
    frac, intr = math.modf(f)
    sint = bin(int(intr))[2:]
    n = 1
    while frac != float(int(frac)):
        frac *= 10
        n *= 10
    m = math.ceil(math.log(n, 2))
    val = (frac * 2**m) / n
    # assert val == float(int(val)), f"val should be an integer: {val}"
    sfrac = bin(int(val))[2:]
    sfrac = sfrac.rstrip('0')
    return f"{sint}.{sfrac}"


def bin2dec(s: str) -> str:
    """
    Convert string form of a binary number its decimal form.
    """

    s = s.strip()
    n = 0
    m = 0
    i = 0
    N = len(s)
    while i < N and s[i] != '.':
        n *= 2
        if s[i] == '1':
            n += 1
        elif s[i] == '0':
            n += 0
        else:
            raise ValueError(f"Invalid binary digit: {s[i]}")
        i += 1
    if i < N and s[i] == '.':
        i += 1
    z = 1.0
    for j in range(i, N):
        z /= 2
        if s[j] == '1':
            m += z
        elif s[j] == '0':
            m += 0
        else:
            raise ValueError(f"Invalid binary digit: {s[j]}")
    r = m + n
    return str(r)


def padto(s, n):
    while len(s) < n:
        s += '0'
    return s


def getparts(sbin: str):
    Binary16_ExponentBias = 15
    Binary16_ExponentBits = 5
    Binary16_FractionBits = 10

    intr, frac = sbin.split('.')
    # print(f"intr = {intr} frac = {frac}")
    n = len(intr)
    # TODO: handle < 1 case
    # TODO: handle negatives
    assert intr[0] == '1', 'only handling >= 1 case'
    exp = n - 1
    if len(intr) > 1:
        sfrac = intr[1:] + frac
    else:
        sfrac = frac
    sign = 0
    print(f"(-1)**{sign} x 1.{sfrac} x 2^{exp}")
    signbits = '0'
    biasedexp = exp + Binary16_ExponentBias
    bexpbits = bin(biasedexp)[2:]
    if len(bexpbits) > Binary16_ExponentBits:
        raise ValueError(f"Exponent: {exp} unable to fit in exponent")
    bexpbits = padto(bexpbits, Binary16_ExponentBits)
    fracbits = padto(sfrac, Binary16_FractionBits)
    if len(fracbits) > Binary16_FractionBits:
        print(f"warn: dropping some fraction bits from {fracbits}")
    fracbits = fracbits[0:Binary16_FractionBits]

    result = f"{signbits}{bexpbits}{fracbits}"
    print(f"Signbits: {signbits}")
    print(f"BExpbits: {bexpbits} ({int(bexpbits, 2)})")
    print(f"Fracbits: {fracbits}")
    print(f"IEEE    : {result}")
