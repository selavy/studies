#include <super/super.h>

namespace super {

Pair::~Pair() {}

Pair make(int x, int y)
{
    return { x, y };
}

}  // namespace super
