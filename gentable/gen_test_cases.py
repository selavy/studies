#!/usr/bin/env python3


import random


N = 32
M = 64
# NOTE: 0 is a reserved value
randu   = lambda x: random.randint(1, 2**x-1)
randU32 = lambda: randu(32)
randU64 = lambda: randu(64)

fmt_by_dtype = {
    'u32hex': '0x{:08x}',
    'u64hex': '0x{:016x}',
}
cpp_by_dtype = {
    'u32hex': 'uint32_t',
    'u64hex': 'uint64_t',
}

# key = randU32()
# vals = [(key, randU32(), randU64()) for _ in range(N)]
# keys = [(x[0], x[1]) for x in vals]
# success = [random.choice(vals) for _ in range(M)]
# failure = []

keys = [(randU32(),) for _ in range(M)]
vals = [(randU32(), randU64()) for _ in range(N)]

def genval():
    y = randU32()
    while y in vals:
        y = randU32()
    return y

miss = [(genval(),) for _ in range(M)]


def print_vector(vals, name, dtypes, indent=0):
    indent = ' ' * indent
    tabs = indent + '    '
    cpptypes = [cpp_by_dtype[dt] for dt in dtypes]
    if len(cpptypes) == 1:
        cctype = cpptypes[0]
        def fmtrow(vs): return vs
    else:
        cctype = f"std::tuple<{', '.join(cpptypes)}>"
        def fmtrow(vs): return f"{{ {vs} }}"

    fmts = [fmt_by_dtype[dt] for dt in dtypes]
    print(f"{indent}const std::vector<{cctype}> {name} = {{")
    rows = [
        tabs + fmtrow(', '.join([fmt.format(v) for v, fmt in zip(vs, fmts)])) + ','
        for vs in vals
    ]
    print("\n".join(rows))
    print(f"{indent}}};")


print('TEST_CASE("Insert random values and look them up", "[gentbl]")')
print('{')
print_vector(keys, name='keys', dtypes=['u32hex'], indent=4)
print()
print_vector(vals, name='vals', dtypes=['u32hex', 'u64hex'], indent=4)
print()
print_vector(miss, name='miss', dtypes=['u32hex'], indent=4)
print()
print('}')


# print("const std::vector<std::tuple<uint32_t, uint32_t, uint64_t>> vs = {")
# for _ in range(N):
#     print("    {{ 0x{:08x}, 0x{:08x}, 0x{:016x} }},".format(
#         randU32(), randU32(), randU64()))
# print("};")
