# OCR Provider Template

This template builds a minimal DracoPho OCR provider plugin. Replace the sample implementation with your OCR engine, keep `providerId()` stable, and update `metadata.json` before distribution.

## Build

```bash
cmake -S . -B build -DMARK_SHOT_PLUGIN_SDK_DIR=/path/to/mark-shot/plugin-sdk
cmake --build build --parallel
```

## Install For Local Testing

```bash
mkdir -p ~/.local/share/mark-shot/plugins
cp build/libmark-shot-sample-ocr.so ~/.local/share/mark-shot/plugins/
```

Set `ocr.provider` to `plugin:sample-ocr` in DracoPho settings, or choose it from the Plugins settings page.
