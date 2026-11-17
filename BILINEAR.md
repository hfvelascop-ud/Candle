# Candle 11.2: bilinear heightmap option

Based on upstream commit `e9288ecf5` (`v11.2`). The About dialog identifies this
custom build as `11.2-bilinear`; original authorship and GPL licensing remain.

## Use

1. Load the original, uncompensated G-code and create or load a heightmap.
2. Enter heightmap **Edit** mode while the sender is stopped.
3. In **Interpolation grid > Type**, choose **Bilinear** or **Bicubic**.
4. Save the map if the selected method should be reused. Leave Edit mode and
   enable **Use heightmap**. Candle rebuilds compensated G-code using that method.
5. Keep the original heightmap reference XY and work Z zero consistent. The
   interpolation method does not change work offsets, probe commands or spindle
   control. Do not enable a map again on externally pre-compensated G-code.

Both the preview and G-code correction use the selected calculation, including
the first trajectory point. Changing methods discards the transformed-program
cache. Measured heights and probe-grid dimensions are not changed by the switch.

Bilinear interpolation uses the four corners of each measured cell. Finite
heights stay within their four-corner range inside that cell. Outside the map
it holds the nearest boundary height; this is not a measurement of unprobed
material. Keep the full cutting path within the measured area. Missing corner
measurements and invalid grid geometry return NaN rather than invented zeros.

The interpolation grid still determines trajectory subdivision. Follow the
upstream guidance of at least about ten times the probe-point count per axis
(for example 70 x 30 for a measured 7 x 3 grid). A denser calculated grid does
not add measurements or improve probe repeatability.

## Maps and settings

The existing `candle map v3` interpolation-type field is retained: `0` means
Bicubic (unchanged legacy default), `1` means Bilinear. Existing v3 maps remain
readable; select Bilinear explicitly to change their method. Unknown type values
fall back to Bicubic. This change does not introduce support for older map file
signatures that upstream 11.2 itself cannot read.

Stock Candle does not implement Bilinear even if a map contains type `1`.
Use this custom build for maps saved with the Bilinear selection.

## Validation

The regression executable uses real Qt 5 and Candle's HeightMapTableModel. It
checks offset non-square maps, measured nodes, asymmetric weighted points,
inclined planes, method dispatch, legacy bicubic dispatch, bounded alternating
heights, invalid data and boundaries. Assertions also check nominal cut depth
plus interpolated correction. These arithmetic tests do not simulate the
complete GUI, serial sender, firmware or physical machine.

With Qt 5 development packages available:

```sh
cmake -S tests -B build-tests -DCMAKE_BUILD_TYPE=Release
cmake --build build-tests --config Release
ctest --test-dir build-tests -C Release --output-on-failure
```

For a vcpkg setup, pass the same CMake toolchain file as for the app build.
The `Heightmap interpolation tests` GitHub Actions workflow runs these tests on
pushes to `heightmap-bilineal` and pull requests.

Build the Windows application with the upstream instructions or its existing
`Release / Windows (x64)` workflow, choosing `heightmap-bilineal` as source.
Passing the math tests is not a Windows application build or a machine test.

Before cutting: inspect a synthetic flat and inclined map, save/reopen each
method, inspect exported transformed G-code, and toggle methods after a prior
transformation to check that the program is regenerated rather than compounded.
Perform a controlled trial on scrap with verified work zero before PCB work.
