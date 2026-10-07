mkdir msvc64
cmake -S source -Bmsvc64  -G "Visual Studio 18 2026" -DBUILD_WITH_RTTI=OFF -DCMAKE_INSTALL_PREFIX="msvc/x86_64"
cmake --build msvc64 --target install --config Release
cmake --build msvc64 --target install --config Debug
