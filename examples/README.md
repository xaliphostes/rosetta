Go inside each leaf folder, and then type:

- For Python:
    ```sh
    mkdir build && cd build
    cmake ..
    cmake --build .
    python3.14 test.py
    ```

- For JavaScript (node.js):
    ```sh
    npm i # first time, it will install (and compile as well)
    ```
    then
    ```sh
    npm run build # (compile)
    node test.js  #(run)
    ```
