# trace-xyyx

Simple C Tracing example, for x86 and raspberry pi pico.

Steps to build it:

1. `export PICO_SDK_FETCH_FROM_GIT=true` or `export PICO_SDK_PATH = <path>`
1. `mkdir build`
1. `cd build`
1. `cmake ..`
1. `make -j 8`
1. then copy `trace-xyyx.uf2` to your pico-pi for fun!

To add a trace point, edit the `metadata` file. Then run `xxd -i metadata > metadata.h` to override the `metadata.h` file.

This needs more work, it is still non-functional.

To test on x86, run `gcc ctf.c xyyx.c -o xyyx.c`
