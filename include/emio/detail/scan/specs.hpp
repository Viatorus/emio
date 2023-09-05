//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <cstdint>

namespace emio::detail::scan {

inline constexpr char no_type = 0;

struct scan_specs {
  bool alternate_form{false};
  char type{no_type};
};

}  // namespace emio::detail::scan
