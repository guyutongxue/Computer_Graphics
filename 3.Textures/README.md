# Camera

## Build Instruction

Because some C++20 feature is used, only GCC 10 may compile this project.

```sh
mkdir build
cd build
conan install .. -b missing # add `-pr ../conan_mingw_profile.txt` for MinGW Building
cmake .. # add `-G"MinGW Makefiles"` for MinGW Building
```

## Using instruction

Execute the program. **Make sure that `Hand.fbx` is located directly at working directory.**

This project is based on `1.Hand`. So these interactions are same as it:

- Swipe the mouse to change the direction of camera.
- Scroll the mouse to change the angle of camera.
- <kbd>ESC</kbd> Quit the app.
- <kbd>r</kbd> Toggle the **r**otation.
- <kbd>c</kbd> Clear current gesture.
- <kbd>0</kbd> <kbd>1</kbd> <kbd>2</kbd> <kbd>3</kbd> <kbd>4</kbd> <kbd>5</kbd> <kbd>6</kbd> <kbd>7</kbd> <kbd>8</kbd> <kbd>9</kbd> Show the gesture of these numbers. (See [here](https://en.wikipedia.org/wiki/Chinese_number_gestures))
- <kbd>o</kbd> Show the gesture of **o**k.
- <kbd>t</kbd> Show the gesture of **t**humb-up/down.
- <kbd>↑</kbd> <kbd>↓</kbd> <kbd>←</kbd> <kbd>→</kbd> Change the position of camera (front, back, left, right).
- <kbd>PgUp</kbd> <kbd>PgDn</kbd> Change the position of camera (up, down).
- <kbd>Shift</kbd> Change the direction of model.
    - Press left <kbd>Shift</kbd> to turn left.
    - Press right <kbd>Shift</kbd> to turn right.
    - Press them at same time to make model up-side-down.
- <kbd>Alt</kbd> Unlock the cursor.

Additional feature of this project:

- ImGui: control a transform of camera. (Use <kbd>Alt</kbd> to show cursor)
    - `Start pos`: The start point `(x, y, z)`.
    - `Start ang:p`: The pitch angle at beginning.
    - `Start ang:y`: The yaw angle at beginning.
    - `End pos`: The end point `(x, y, z)`.
    - `End ang:p`: The destination pitch angle.
    - `End ang:y`: The destination yaw angle.
    - Click `Start transform` to start this camera transform. Click `Stop transform` to stop transforming.
- During sliding above sliders, two line will change simutaneously indicating this two camera:
    - Green line indicates start point and direction
    - Red line indicates end point and direction
- Press <kbd>s</kbd> to preview start camera. Press <kbd>e</kbd> to preview end camera.

Row angle is not supported, and won't be supported.