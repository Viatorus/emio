#include "floff.h"

int main() {
  using jkj::floff::main_cache_compressed;
  using jkj::floff::extended_cache_super_compact;

  char buffer[1024];
  double x = 1.2345678e-101;
  *(jkj::floff::floff<main_cache_compressed, extended_cache_super_compact>(x, 123, buffer)) = '\0';
}
