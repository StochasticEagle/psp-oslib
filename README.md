# OldSchool Library (OSLib)
## Overview

OldSchool Library (OSLib) is a lightweight and versatile C++ library designed to provide functionality and tools commonly used in classic game development and retro programming projects. It bridges the gap between modern and old-school development environments, making it easier to create and manage applications with a nostalgic touch.

## Features

- **Graphics and Rendering:** Basic graphics functions and rendering capabilities for 2D game development.
- **Input Handling:** Read PSP controller buttons and analog input.
- **Sound Management:** Simple sound and music playback functionalities.
- **Utility Functions:** A collection of utility functions for handling files, strings, and more.
- **PSP Target:** Builds using the PSP SDK; host operating systems require a working PSP cross-toolchain.

## Building

### Library

**Makefile (legacy):**
```bash
make
make install
```

Install the PSP SDK and external dependencies, including `psp-math`, first.

**CMake:**
```bash
cmake --preset psp
cmake --build --preset psp -j
cmake --install build
```

### Samples

Each sample can be built standalone from its own directory using the same commands above.

To build all samples at once from the `samples/` directory:

**Makefile:**
```bash
cd samples && make
```

**CMake:**
```bash
cd samples
cmake --preset psp
cmake --build --preset psp -j
```

### Editor / LSP support (compile_commands.json)

The CMake build exports `compile_commands.json` automatically via `CMAKE_EXPORT_COMPILE_COMMANDS=ON`. This file is used by clangd (and other LSP clients) for accurate code completion, diagnostics, and navigation.

After running `cmake --preset psp` at the root, a symlink `compile_commands.json → build/compile_commands.json` is already in place. clangd will pick it up automatically.

> `compile_commands.json` is in `.gitignore` — run the cmake configure step once after cloning to generate it.

## Installation

Install using the selected build system's `install` target; it installs `libosl.a` and the OSLib headers into the configured PSP SDK prefix. `psp-math` is a separate package and its archive/header must remain installed independently.

Tracker module playback requires libxmp-lite. Applications using `oslLoadSoundFileMOD()` must link with `-lxmp-lite`.

OSLib uses the external `psp-math` package for VFPU math. Install `psp-math` before building OSLib, and link applications using OSLib math APIs with `-l:psp-math.a` after `-losl` (or link the CMake `osl` target, which propagates the archive dependency). The math headers are supplied by `psp-math`; OSLib no longer bundles `libpspmath`.

## Math API and initialization

OSLib's existing math names are aliases for the canonical `<pspmath.h>` API, including `oslVfpu_sinf`, `oslVfpu_cosf`, `oslSin`, and `oslCos`. Integer-degree `oslSini(angle, dist)` and `oslCosi(angle, dist)` use the single read-only sine/cosine lookup table in `psp-math` and preserve the integer-radius Q14 multiplication behavior. **Do not call `oslSetupFTrigo()`**: it and OSLib's former writable sine/cosine arrays have been removed. Table lookup needs no initialization. New math code should use `<pspmath.h>` directly, not the legacy compatibility header. See [PSP-Math](https://github.com/StochasticEagle/psp-math) for fixed-point conventions and limits.

## Documentation

You can find the documentation in the `doc` directory, or consult it online here:  
[http://dogo.github.io/oslib/](http://dogo.github.io/oslib/)

## Contributing

We welcome contributions to OSLib! If you’d like to contribute, please follow these steps:

1. Fork the repository.
2. Create a feature branch.
3. Make your changes and commit them.
4. Push your branch to your fork.
5. Create a pull request describing your changes.

Please follow the coding guidelines and write tests for any new features or bug fixes.

## License

OSLib is free software, licensed under the GPLv2. See `COPYING` for details.

## Thanks

- **Brunni** for the original OSLib
- **Sakya** for the first modification of OSLib
- **BenHur** for intraFont
