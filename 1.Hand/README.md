# Hand

## Build Instruction

```sh
mkdir build
cd build
conan install .. -b missing # add `-pr ../conan_mingw_profile.txt` for MinGW Building
cmake ..
```

## Using instruction

Execute the program. **Make sure that `Hand.fbx` is located directly at working directory.**

### Mouse interaction

- Swipe the mouse to change the direction of camera.
- Scroll the mouse to change the angle of camera.

### Keyboard interaction

- <kbd>ESC</kbd> Quit the app.
- <kbd>r</kbd> Toggle the **r**otation.
- <kbd>c</kbd> Clear current gesture.
- <kbd>0</kbd> <kbd>1</kbd> <kbd>2</kbd> <kbd>3</kbd> <kbd>4</kbd> <kbd>5</kbd> <kbd>6</kbd> <kbd>7</kbd> <kbd>8</kbd> <kbd>9</kbd> Show the gesture of these numbers. (See [here](https://en.wikipedia.org/wiki/Chinese_number_gestures))
- <kbd>o</kbd> Show the gesture of **o**k.
- <kbd>t</kbd> Show the gesture **thumb**-up/down.
- <kbd>↑</kbd> <kbd>↓</kbd> <kbd>←</kbd> <kbd>→</kbd> Change the position of camera (front, back, left, right).
- <kbd>PgUp</kbd> <kbd>PgDn</kbd> Change the position of camera (up, down).
- <kbd>Shift</kbd> Change the direction of model.
    - Press left <kbd>Shift</kbd> to turn left.
    - Press right <kbd>Shift</kbd> to turn right.
    - Press them at same time to make model up-side-down.
- <kbd>Alt</kbd> Unlock the cursor.

