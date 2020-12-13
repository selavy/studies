#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <vector>

union binary32 {
    float    f;
    uint32_t i;
    uint8_t  b[4];
};

int main(int argc, char** argv)
{
    binary32 val;
    // val.f = 1.1f;
    val.f = 5.5f;
    printf("float = %0.10f => 0x%02X%02X%02X%02X\n", val.f, val.b[0], val.b[1], val.b[2], val.b[3]);
    printf("                     => 0x%08X\n", val.i);
    return 0;
}
