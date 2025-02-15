# cards_scanner


## Build and run
```bash
mkdir -p build && cd build
conan install .. --build=missing -s compiler.cppstd=20
cmake .. --preset conan-release  
apt install A LOT OF STUFF until conan no longer complains
cmake --build Release/ --parallel $(nproc)
./mtg_ocr path/to/card_image.jpg
```