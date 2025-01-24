#include "floff.h"

int main() {
  using jkj::floff::main_cache_full;
  using jkj::floff::extended_cache_long;

  char buffer[1024];
  double x = 1.2345678e-101;
  *(jkj::floff::floff<main_cache_full, extended_cache_long>(x, 123, buffer)) = '\0';
}
