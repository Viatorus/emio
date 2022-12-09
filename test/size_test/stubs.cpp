extern "C" {

void _exit(int /*unused*/) {
  while (1) {
  }
}

int _getpid() {
  return 1;
}

int _kill(int /*unused*/, int /*unused*/) {
  return -1;
}

void* _sbrk(int /*unused*/) {
  return 0;
}

int _isatty(int /*unused*/) {
  return 1;
}

int _lseek(int /*unused*/, int /*unused*/, int /*unused*/) {
  return 0;
}

struct stat;

int _fstat(int /*unused*/, struct stat* /*st*/) {
  return 0;
}

int _read(int /*unused*/, void* /*unused*/) {
  return 0;
}

int _write(int /*unused*/, void* /*unused*/) {
  return 0;
}

int _close(int /*unused*/) {
  return -1;
}
}