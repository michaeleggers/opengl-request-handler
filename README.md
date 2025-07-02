# Server that listens on incoming requests and plays some graphics on the desktop.

## WIP! 

- As of now, only (very basic) request handling is in place.

## TODOs

- [x] Use native networking API to listen for requests and send a response.
- [x] Start SDL window with OpenGL context.
- [x] Move request-handling into its own thread.
- [x] Experiment with wayland directly to make window non-clickable (failed, but was fun).
- [x] Load GLSL code from disk and compile to SPIR-V via glslang.
- [x] Create uniform buffer to animate shader with CPU-side data.
- [ ] Use shader program to do some fun stuff when request comes in.
- [ ] Load external models (OBJ).

## Build requirements

- Linux OS
- clang++
- glslang (comes with eg. the Vulkan SDK)

Create Makefile (or whatever you need) via CMake. Then build the program. It will be put into the `/bin/` directory.

## Starting the program

The program expects you to either have the `shaders` next to the executable directory:

```bash
cp -r shaders bin/
```

**or** you pass the base-directory as a commandline-argument eg:

```bash
./curltest /home/username/repos/opengl-request-handler/
```


