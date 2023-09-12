//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <cstdint>

#include "../../reader.hpp"

namespace emio::detail::scan {

inline constexpr char no_type = 0;
inline constexpr int no_width = -1;

struct format_specs {
  bool alternate_form{false};
  char type{no_type};
  int32_t width{no_width};
};

}  // namespace emio::detail::scan
