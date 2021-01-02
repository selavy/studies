#!/usr/bin/env python3

import math
import random


def padto(s: str, n: int, val: str = '0'):
    while len(s) < n:
        s += val
    return s


def leftpad(s: str, n: int, val: str = '0'):
    while len(s) < n:
        s = val + s
    return s


_DEBUG = 0


def log(msg: str) -> None:
    if _DEBUG:
        print(msg)


def frombinary16(s: str) -> float:
    s = s.strip()
    s = s.replace(' ', '')
    if s.startswith('0b'):
        s = s[2:]
    if len(s) != 16:
        raise ValueError('Invalid length for binary16')
    sign = int(s[0])
    exponent = s[1:6]
    mantissa = s[6:16]
    log(f"Sign:     {sign}")
    log(f"Exponent: {exponent}")
    log(f"Mantissa: {mantissa}")
    eint = int(exponent, 2)
    mint = int(mantissa, 2)
    s = '+' if sign != 0 else '-'
    if eint == 0b00000 and mint == 0:
        m = 0
        log(f"{s}zero")
    elif eint == 0b11111 and mint == 0:
        m = float('inf')
        log(f"{s}infinity")
    elif eint == 0b11111 and mint != 0:
        log("NaN")
        m = float('nan')
    elif eint == 0b00000 and mint != 0:
        m = mint / 2**10 * 2**-14
        log(f"subnormal: (-1)^{sign} x 0.{mantissa} x 2^-14 = {m}")
    else:
        m = (1 + mint / 2**10) * 2**(eint - 15)
        log(f"(-1)^{sign} x 1.{mantissa} x 2^{eint - 15} = {m}")
    return m


def check(result: str, exponent: int):
    first, second = result.split('.')
    integral = float(int(first, 2))
    mantissa = 0.0
    i = 1
    for c in second:
        if c == '1':
            mantissa += 1 / math.pow(2, i)
        i += 1
    final = integral + mantissa
    final = final * math.pow(2, exponent)
    log(f"{result} x 2**{exponent} = {final}")


def tobinary16(s: float) -> str:
    log(f"tobinary16({s})")
    s = float(s)
    sign = 0 if s >= 0 else 1
    s = abs(s)
    integral = int(s)
    fraction = s - integral
    mantissa1 = bin(integral)[2:]
    mantissa2 = ''
    # TODO: how far do I need to go?
    for i in range(32):
        x = 1 / math.pow(2, i+1)
        if fraction >= x:
            fraction -= x
            mantissa2 += '1'
        else:
            mantissa2 += '0'
    log(f"sign     = {sign}")
    log(f"mantissa = {mantissa1}.{mantissa2}")
    exponent = 0
    if mantissa1[0] == '1':
        exponent = len(mantissa1) - 1
        result = mantissa1[0] + '.'
        if len(mantissa1) > 1:
            result += mantissa1[1:]
        result += mantissa2
    else:
        i = mantissa2.find('1')
        if i == -1:
            result = '0.0000000000'
            exponent = 0
        else:
            exponent = -1 * i - 1
            result = mantissa2[i] + '.'
            if len(mantissa2) > i:
                result += mantissa2[i+1:]
    result = padto(result, n=10+1+1, val='0')
    result = result[0:10+1+1]
    log(f"Result: {result} x 2**{exponent}")
    check(result, exponent)
    # TODO: generate sub-normals
    # exponent := [1, 31) => [-14, 16)
    if exponent < -14:    # round to zero
        assert result[1] == '.'
        mantissa = result[0] + result[2:]
        exponent += 1
        while exponent < -14:
            mantissa = '0' + mantissa[0:9]
            exponent += 1
            if mantissa == '0000000000':
                exponent = 0
                break
        exp = '00000'
        # result = '0000000000'
        # exp = bin(exponent)[2:]
        result = mantissa
    elif exponent >= 16:  # round to infinity
        exp = '11111'
        result = '0000000000'
    else:
        exp = bin(exponent + 15)[2:]
        result = result[2:12]

    exp = leftpad(exp, 5, '0')
    if len(exp) != 5:
        raise ValueError(f"Invalid exponent: {exp}")
    mant = padto(result, 10, '0')
    log(f"Sign     = {sign}")
    log(f"Exponent = {exp}")
    log(f"Mantissa = {mant}")
    log("")
    return f'{sign}{exp}{mant}'


if 0:
    for i in range(100):
        x = random.random() * 10.0
        y = random.random() * 10.0
        print(f"    ({x}, {y}),")

if 1:
    cs = [
	(1.0, 1.0),
	(1.5, 1.0),
	(0.5, 1.0),
	(0.5, 10.1),
	(1.5, 10.1),
	(1.5, 100.1),
	(1.5, 100.01),
	(0.01, 100.01),
	(0.00001, 0.00001),
	(1.00001, 5.00001),
	(124.1234, 1.1829237),
	(0.27272, 0.72728),
	(6.828282, 7.18282346),
	(0.6106367660844092, 6.6719760034299656),
	(2.2861758147090603, 0.16159267716543746),
	(8.20404181868691, 2.0506431085778454),
	(1.403190716388657, 6.754682258257178),
	(7.1794438939076155, 1.1029659927015423),
	(8.523769848129529, 8.181144383615834),
	(6.143277738144254, 8.071632816893628),
	(2.64524055453468, 7.881881442259013),
	(9.166856781183295, 1.0453784602375926),
	(8.882494763446667, 1.5221511804768373),
	(5.341212652873246, 9.590488562289336),
	(3.401313317146272, 2.7432610263196056),
	(8.39049720064664, 9.939016342446314),
	(6.505235201173271, 1.608063278794859),
	(6.221317127244332, 4.10331717473793),
	(8.158594913768384, 2.5320079536202056),
	(8.36988498187779, 3.340256306094762),
	(6.754872214132247, 7.416609620024208),
	(5.694040523321809, 7.150540780676204),
	(6.558853304306172, 9.680459397007473),
	(3.4720967802762406, 7.988764463292517),
	(9.004121154999671, 8.193431821536505),
	(8.723423420359563, 2.428140361113348),
	(9.905791422977714, 8.400112437347328),
	(5.813471028574286, 1.7948160364680354),
	(2.540860522023185, 0.13289907654805755),
	(5.683268162879667, 2.389665245888332),
	(1.1278556931333394, 5.220879669046953),
	(3.832484246994843, 1.787820081897341),
	(0.9597278424132882, 0.5274002306716874),
	(3.6887608375490477, 4.855294189368947),
	(1.2430969887852061, 3.956796257709235),
	(3.7056261186502395, 7.910604005939947),
	(4.432744699567087, 2.4424231658806237),
	(7.123910566305342, 6.951034414294303),
	(1.9120035492457976, 8.080566646608954),
	(2.6743276491644097, 1.3179706881486564),
	(9.82760964764326, 6.606937105104304),
	(7.9652812383242555, 9.060715964449786),
	(4.140544584074098, 9.002870014841363),
	(2.444912967670836, 8.171045840371058),
	(4.224058311535385, 1.5441648960574739),
	(1.2304278090782284, 9.459166092208292),
	(6.699304833299328, 2.867339480086609),
	(0.9487714624515897, 6.672251662708161),
	(7.207417623787604, 8.075600366972065),
	(5.4213433519372405, 4.364042606239749),
	(3.946218119651075, 1.3285243949855619),
	(0.9430354363401361, 9.34218754032393),
	(7.231115613009331, 8.059024839399797),
	(5.339212400180333, 9.270778604597332),
	(8.64239463771132, 3.0317753206950027),
	(3.829916186601986, 9.22515312906231),
	(8.423461474230646, 3.9178032556608198),
	(0.9457509008945764, 5.522599014680592),
	(1.9185868754899482, 4.884034142571129),
	(2.8916359268029135, 4.196719264314891),
	(9.12146305298389, 6.301123209029043),
	(4.323913106210915, 8.914977605920015),
	(9.16351203772515, 3.0006762274279364),
	(4.975987976802281, 8.131024213222735),
	(2.205135682955647, 9.815131734018323),
	(9.829951332514142, 9.529549703542127),
	(6.813918807614644, 9.98861403049913),
	(4.886199505936616, 8.745954201947688),
	(9.485693615204106, 4.141389510397957),
	(8.523882958848766, 2.626651578130291),
	(9.748525657273445, 1.1082424770524246),
	(4.040192541184586, 8.271805270692454),
	(4.4280415042630095, 1.2631470689274416),
	(0.4076227861739401, 8.27795098254882),
	(5.204021110297916, 3.6747608113049415),
	(0.35094593755608083, 6.802841267167068),
	(4.727128085380359, 5.788432585913147),
	(7.876270670371564, 2.5422290319218535),
	(2.6999785828495213, 8.374301672954314),
	(7.842345974670425, 5.79255148035964),
	(6.430209187056483, 3.656805564147395),
	(6.623100499049828, 1.6996416388783753),
	(1.0465442002891645, 9.19927213735875),
	(7.742440843431932, 4.354885651735978),
	(4.561036232827599, 9.979649118990652),
	(2.5778224710262574, 0.5489388361659997),
	(1.279926793408288, 2.2220592139590654),
	(1.5212268855612487, 8.135272313049043),
	(7.967020125071931, 8.163501331746229),
	(9.801133700497937, 2.982502729459969),
	(5.238363277546543, 3.3173459267488568),
	(2.922300676149617, 8.684276860010069),
	(7.499270957287259, 6.824238481869434),
	(8.23949769370187, 6.543582349535624),
	(0.7987647279903498, 2.306762865191583),
	(7.284316649810512, 8.619366667588897),
	(5.935667808889239, 0.9168618840439369),
	(8.407918452001974, 5.867616257651377),
	(7.951908584729143, 2.29360518409771),
	(0.648536227777281, 9.76477212761575),
	(2.1834452441892847, 6.8984122255004845),
	(0.8553981507314545, 1.2545040913184824),
	(8.161759934874306, 7.159356673582287),
	(2.9881942686393836, 0.6763172457652156),
	(0.683587784537274, 6.442578813761456),
	(3.2784216933399115, 6.201393279087654),
	(8.757233928138737, 6.132026263163074),
	(1.438647505380708, 3.0590948293520572),
	(9.099073231667063, 6.928423866846466),
	(3.9017292445940086, 7.919342688654005),
	(9.718670567508067, 3.382734400826103),
	(7.021512017239591, 0.06275293133955695),
	(1.341908541792367, 1.0888469459712302),
    ]


def make_cases1(cs):
    print(f'TEST_CASE("Autogenerated add tests")')
    print(f'{{')
    for x, y in cs:
        z_exact = x + y
        binary16_x = tobinary16(x)
        binary16_y = tobinary16(y)
        x_ = frombinary16(binary16_x)
        y_ = frombinary16(binary16_y)
        z = x_ + y_
        binary16_z = tobinary16(z)

        # binary16_z = tobinary16(z)
        log(f"frombinary: {x:.10f} => {frombinary16(binary16_x)}")
        log(f"frombinary: {y:.10f} => {frombinary16(binary16_y)}")
        log(f"frombinary: {z:.10f} => {frombinary16(binary16_z)}")
        print(f'    SECTION("{x} + {y} = {z:.10f} ({z_exact:.10f})")')
        print(f'    {{')
        print(f'         binary16 x = binary16_fromrep(0b{binary16_x}); // {x}')
        print(f'         binary16 y = binary16_fromrep(0b{binary16_y}); // {y}')
        print(f'         binary16 z = binary16_fromrep(0b{binary16_z}); // {z}')
        print(f'         binary16 a = binary16_add(x, y);')
        print(f'         INFO("x = {x:.10f} = " << binary16_tofloat(x) << " = " << dump_u16(x.rep));')  # noqa
        print(f'         INFO("y = {y:.10f} = " << binary16_tofloat(y) << " = " << dump_u16(y.rep));')  # noqa
        print(f'         INFO("z = {z:.10f} = " << binary16_tofloat(z) << " = " << dump_u16(z.rep));')  # noqa
        print(f'         INFO("a                = " << binary16_tofloat(a) << " = " << dump_u16(a.rep));')  # noqa
        print(f'         CHECK(a.rep == z.rep);')
        print(f'    }}')
    print(f'}}')


def make_cases2(cs):
    for x, y in cs:
        x = f'{x:0.10f}'
        y = f'{y:0.10f}'
        print(f'    SECTION("{x} + {y}")')
        print(f'    {{')
        print(f'        auto a = {x}_h;')
        print(f'        auto b = {y}_h;')
        print(f'        auto c = a + b;')
        print(f'        auto d = binary16_fromfloat((float)a);')
        print(f'        auto e = binary16_fromfloat((float)b);')
        print(f'        auto f = binary16_add(d, e);')
        print(f'        CHECK((float)c == binary16_tofloat(f));')
        print(f'    }}')
        print(f'')


make_cases2(cs)


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
