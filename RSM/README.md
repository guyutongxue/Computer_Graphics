# RSM - Reflective Shadow Maps

*The final assignment of course Coumputer Graphics, an implementation of [Reflective Shadow Maps](https://users.soe.ucsc.edu/~pang/160/s13/proposal/mijallen/proposal/media/p203-dachsbacher.pdf)*

## Build Instruction

**Some C++2b features are used. You must compile it under GCC 11 or Clang 13.** Cmake minimum version 3.20.5 required.

```sh
mkdir build
cd build
conan install .. -b missing # add `-pr ../mingw.txt` for MinGW Building
cmake .. # add `-G"MinGW Makefiles"` for MinGW Building
make # `mingw32-make` for MinGW building
```

## Using Instruction

- Use <kbd>W</kbd> <kbd>A</kbd> <kbd>S</kbd> <kbd>D</kbd> <kbd>Space</kbd> <kbd>Shift</kbd> to move camera.
- Press <kbd>Alt</kbd> to unlock cursor.
- Use ImGui to control light and reflectivity.
