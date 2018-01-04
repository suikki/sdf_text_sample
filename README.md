# sdf_text_sample [![Travis build status](https://travis-ci.org/suikki/sdf_text_sample.svg?branch=master)](https://travis-ci.org/suikki/sdf_text_sample)

A simple cross platform OpenGL application using signed distance field (SDF)
font rendering to render text that can be scaled freely while still appearing
sharp.

Tested on linux, windows, macOS, android and emscripten.

See the online [demo](https://suikki.github.io/sdf_text_sample/demo/).

## Building


Requirements:
- cmake 3.2 or newer (on path)
- SDL2
  - on linux: SDL2 must be installed (e.g. libsdl2-dev on ubuntu)
  - on macOS: SDL2 must be installed (e.g. brew install sdl2)
  - on Windows: SDL2 binaries will be downloaded by the build script automatically.

Run the appropriate build script from `platforms/` to build the app.

## License
The project is licensed under the [zlib license](LICENSE.txt)

## Links
 Using:
- [fontstash](https://github.com/memononen/fontstash) for font rendering (using a [fork](https://github.com/suikki/fontstash/tree/gl_es2) supporting SDF and OpenGL ES2).
- [SDL2](https://libsdl.org/) for cross platform windowing and input.
- [glew](http://glew.sourceforge.net/) for OpenGL extension loading.
