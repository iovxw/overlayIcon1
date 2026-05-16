# overlayIcon1

最小 C++ 复现，用图片输入/输出模拟 KDE `StatusNotifierItemSource::overlayIcon()` 的绘制逻辑。

## 输入

默认读取子模块中的两张图片：

- `/home/runner/work/overlayIcon1/overlayIcon1/ksni_overlay_icon_pixmap_bug/data/default256.png`
- `/home/runner/work/overlayIcon1/overlayIcon1/ksni_overlay_icon_pixmap_bug/data/overlay.png`

## 构建

```bash
cmake -S /home/runner/work/overlayIcon1/overlayIcon1 -B /home/runner/work/overlayIcon1/overlayIcon1/build
cmake --build /home/runner/work/overlayIcon1/overlayIcon1/build
```

## 运行

```bash
/home/runner/work/overlayIcon1/overlayIcon1/build/overlay_icon_repro
```

也可以显式传入：

```bash
/home/runner/work/overlayIcon1/overlayIcon1/build/overlay_icon_repro ICON_PNG OVERLAY_PNG OUTPUT_DIR
```

## 输出

程序会按照 KDE 对应函数的相同逻辑生成并写出这些图片：

- `16x16.png`
- `22x22.png`
- `32x32.png`
- `48x48.png`
- `all.png`

默认输出目录：`/home/runner/work/overlayIcon1/overlayIcon1/outputs`
