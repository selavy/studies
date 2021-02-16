#pragma once

namespace super {

struct Pair
{
    ~Pair() noexcept;
    // ~Pair() noexcept = default;

    int x;
    int y;
};

extern Pair make(int x, int y);

}  // namespace super
