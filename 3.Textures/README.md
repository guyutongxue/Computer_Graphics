# Textures

## Build Instruction

Because some C++17 features are used, some old compilers may not compile this project.

```sh
mkdir build
cd build
conan install .. -b missing # add `-pr ../conan_mingw_profile.txt` for MinGW Building
cmake .. # add `-G"MinGW Makefiles"` for MinGW Building
```

## Using instruction

Execute the program. **Make sure that all below files are located directly at working directory:**
- `texture.bmp`
- `texture_normal.bmp`
- `vert.glsl` (In this project, GLSL is dynamically loaded at runtime.)
- `frag.glsl`

Interact instruction:
- Swipe the mouse to change the direction of camera.
- Scroll the mouse to change the angle of camera.
- <kbd>ESC</kbd> Quit the app.
- <kbd>A</kbd> <kbd>W</kbd> <kbd>S</kbd> <kbd>D</kbd> Change the position of camera (left, front, back, right).
- <kbd>Space</kbd> and left <kbd>Shift</kbd> Change the position of camera (up, down).
- <kbd>Alt</kbd> Unlock the cursor.
- ImGui: control the position of light (Use <kbd>Alt</kbd> to show cursor)

