// Minimal plugin for libprobe.so
extern "C" void __attribute__((constructor)) init() {}
extern "C" void __attribute__((destructor)) fini() {}
int main() {}
