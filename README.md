# trace-xyyx
Simple C Tracing example, for x86 and raspberry pi pico

Steps to build it it. 

`mkdir build`

`cmake ..`

`make -j 8`

then copy trace-xyyx.uf2 to your pico-pi for fun!

To add a trace point, edit the `metadata` file. Then run `xxd -i metadata > metadata.h` to override the metadata.h file.

This needs more work, it is still non-functional.
