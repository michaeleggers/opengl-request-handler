# Server that listens on incoming requests and plays some graphics on the desktop.

## WIP! 

- As of now, only (very basic) request handling is in place.

TODO:
- [x] Use native networking API to listen for requests and send a response.
- [x] Start SDL window with OpenGL context.
- [x] Move request-handling into its own thread.
- [x] Experiment with wayland directly to make window non-clickable (failed, but was fun).
- [ ] Use shader program to do some fun stuff when request comes in.
- [ ] Load external models (OBJ).
