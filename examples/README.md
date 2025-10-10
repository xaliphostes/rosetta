Go inside each leaf folder, and then type:

- For Python:
    ```sh
    mkdir build && cd build
    cmake ..
    cmake --build .
    python3 test.py
    ```

- For JavaScript:
    ```sh
    npm install # first time
    ```
    then
    ```sh
    npm run build
    node test.js
    ```

- For Lua:
    ```sh
    mkdir build && cd build
    cmake ..
    cmake --build .
    lua test.lua
    ```