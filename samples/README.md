OSLib Samples
-----------------

All samples need a file named bkg.png (the background image).

Building any sample with CMake (standalone):
```bash
cmake --preset psp
cmake --build --preset psp -j
```

Or can use the presets:

```bash
cmake --preset psp
cmake --build --preset psp -j
```

Outputs: `cbuild/EBOOT.PBP` and copied assets.
