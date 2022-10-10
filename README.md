# PSI-WASM

A PSI library built on top of Microsoft SEAL that compiles to WASM. Fully interoperable with the [Python PSI library](https://github.com/bit-ml/Private-Set-Intersection).

## Building
- Install emsdk (https://github.com/emscripten-core/emsdk).
- Run `make`.

## Running
- `cd dist && python3 -m http.server 8888`.
- Navigate to `localhost:8888` in browser.
