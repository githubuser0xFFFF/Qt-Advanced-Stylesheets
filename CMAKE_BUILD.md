# CMake build guide

Support CMake build system

## Use CMake build

### Build

```bash
# Create build dir
mkdir build
cd build

# Config project（Use Qt6）
cmake ..

# Or assign Qt5
cmake -DQT_VERSION_MAJOR=5 ..

# Build
cmake --build .

# install (optional)
cmake --install .
```

### Build optional

- `ACSS_BUILD_STATIC`: Build static library (default is OFF, build shared library)
- `ACSS_BUILD_EXAMPLES`: Build example programs (default is ON)

#### Build static library examples

```bash
cmake -DACSS_BUILD_STATIC=ON ..
cmake --build .
```

#### Do not build example programs

```bash
cmake -DACSS_BUILD_EXAMPLES=OFF ..
cmake --build .
```

### Specify Qt installation path

If CMake cannot find Qt automatically, you can specify it manually:

```bash
cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.5.0/msvc2019_64" ..
```

### Specify the installation directory

```bash
cmake -DCMAKE_INSTALL_PREFIX=/path/to/install ..
cmake --build .
cmake --install .
```

## Use in other projects

### Use CMake

In your project's CMakeLists.txt:

```cmake
find_package(QtAdvancedStylesheets REQUIRED)

add_executable(myapp main.cpp)

target_link_libraries(myapp
    PRIVATE
        QtAdvancedStylesheets::qtadvancedcss
)
```

### Use qmake

The original qmake build system remains unchanged and can be used as before.

## Project Structure

```tree

Qt-Advanced-Stylesheets/
├── CMakeLists.txt                  # Main CMake file
├── cmake/                          # CMake configuration file
│   └── QtAdvancedStylesheetsConfig.cmake.in
├── src/                            # source
│   └── CMakeLists.txt
├── examples/                       # Example Programs
│   ├── CMakeLists.txt
│   ├── exporter/
│   │   └── CMakeLists.txt
│   └── full_features/
│       └── CMakeLists.txt
└── styles/                         # style files
```

## Precautions / Notes

1. Minimum CMake version required: 3.16
2. Support Qt5 and Qt6
3. By default, shared libraries are built. The Debug version will have a 'd' suffix added.
4. Output files：
   - Library file: `build/lib/`
   - Executable file: `build/bin/`
