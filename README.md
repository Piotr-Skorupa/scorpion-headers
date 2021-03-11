# scorpion-headers
Header only libraries

## Building and running unit tests

You need to have CMake build system and Conan package manager installed on your machine.
`sudo apt -y install cmake`
For conan visit: https://conan.io/

```bash
cd test
mkdir build
cd build
conan install ..
cmake ..
make
./bin/scorpion-headers-test
```

## Building and running examples

Go to concrete example `cd examples/StateMachine` and run

```bash
mkdir build
cd build
cmake ..
make
./example
```
