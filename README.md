# overlayIcon1

![](outputs/all.png)

Minimal C++ reproduction for the KDE `StatusNotifierItemSource::overlayIcon()` rendering path.

## Source of the overlay implementation

The project adds `./plasma-workspace` as a git submodule, pins it to commit `30bb007627d1f059cd0f235495840dc376c5106a`, and extracts the exact `StatusNotifierItemSource::overlayIcon()` implementation from:

- `plasma-workspace/applets/systemtray/statusnotifieritemsource.cpp`

The build generates a small translation unit from that function and links it into the repro executable.

## Sample inputs

By default the executable reads these sample images from `./ksni_overlay_icon_pixmap_bug`:

- `./ksni_overlay_icon_pixmap_bug/data/default256.png`
- `./ksni_overlay_icon_pixmap_bug/data/overlay.png`

You can also pass custom input paths on the command line.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

Use the bundled sample inputs:

```bash
./build/overlay_icon_repro
```

Use custom inputs and output directory:

```bash
./build/overlay_icon_repro ICON_PNG OVERLAY_PNG OUTPUT_DIR
```

## Output files

The program writes the icon variants produced by the KDE function to `./outputs` by default:

- `16x16.png`
- `22x22.png`
- `32x32.png`
- `48x48.png`
- `all.png`

`all.png` is a contact sheet containing all generated icons side by side.
