#pragma once

#include "darray.h"
#include <optional>
#include <string>


std::optional<Darray> deserialize_darray(const std::string& filename);
