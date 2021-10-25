#pragma once

#include <memory>
#include <type_traits>
#include <stdexcept>

struct bad_any_cast : std::runtime_error
{
    bad_any_cast() noexcept : runtime_error("bad any cast") {}
};
