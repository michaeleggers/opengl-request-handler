# Server that listens on incoming requests and plays some graphics on the desktop.

## What it does

This program listens on port `8081` for incoming requests. If a request comes in
it blows a ton of confetti on your desktop using a compute-shader on your GPU.

## How is it useful?

Well. *Usefulness* is a very subjective term. I use it so that
I get notified when a chat-message from twitch comes in using this little program:
https://github.com/michaeleggers/obs-twitch-bot.

## TODOs

- [x] Use native networking API to listen for requests and send a response.
- [x] Start SDL window with OpenGL context.
- [x] Move request-handling into its own thread.
- [x] Experiment with wayland directly to make window non-clickable (failed, but was fun).
- [x] Load GLSL code from disk and compile to SPIR-V via glslang.
- [x] Create uniform buffer to animate shader with CPU-side data.
- [x] Render a shit-load of particles using a compute-shader.
- [ ] Allow multiple events to be played at the same time.
- [ ] Investigate possible read/write hazard in compute shader. Weird artifacts appear sometimes.
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

### Wayland

Start the program with
```
SDL_VIDEODRIVER=x11 ./curltest
```
Otherwise the window will not be placed as the top-most one.



