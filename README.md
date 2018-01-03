# sdf_text_sample
A simple cross platform OpenGL application using signed distance field (SDF) font rendering to
render text that can be scaled freely while still appearing sharp.

Tested on linux, windows, android and emscripten.

## Building
Requirements:
- cmake 3.2 or newer
- on Windows: Visual studio (tested with vs14) or mingw
- on linux: SDL2 dev package must be installed

## License
The project is licensed under the [zlib license](LICENSE.txt)

## Links
 Using:
- [fontstash](https://github.com/memononen/fontstash) for font rendering.
- [SDL2](https://libsdl.org/) for cross platform windowing and input.
- [glew](http://glew.sourceforge.net/) for OpenGL extension loading.
