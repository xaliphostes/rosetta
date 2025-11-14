# Compiling and testing the examples

## 1. For Python
Go to the `py` folder (not the sub folders) and type:

```sh
mkdir build 
cd build
cmake .. && make
make run
```

**NOTE**: The CMakeFiles.txt is configured for Python 3.14 (toward the end). Make your change...

## 2. For JavaScript (node.js)

For the moment, go inside each leaf folder, and then type:

```sh
npm i # first time, it will install (and compile as well)
```
then
```sh
npm run build # (compile)
node test.js  #(run)
```
