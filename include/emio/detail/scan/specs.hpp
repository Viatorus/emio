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

struct scan_specs {
  bool alternate_form{false};
  char type{no_type};
};

inline constexpr int no_size = -1;

struct scan_string_specs {
  reader remaining;
  int32_t size{no_size};
};

}  // namespace emio::detail::scan
