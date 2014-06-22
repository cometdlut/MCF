@echo off

set Config=Debug32
set CXXFlags=-fno-builtin -g -O0
if "%1"=="Release" (
	set Config=Release32
	set CXXFlags=-DNDEBUG -O3 -ffunction-sections -fdata-sections -Wl,--gc-sections,-s
)

set Lib=-lmcf -lmcfcrt -lstdc++ -lmingwex -lmingw32 -lgcc -lgcc_eh -lmcfcrt -lgcc -lmingwex -ldlmalloc -lmsvcrt -lkernel32 -luser32 -lshell32 -ladvapi32

call mingw 32

mcfbuild -p../../MCFCRT/MCFBuild.mcfproj -s../../MCFCRT -d../../.Built/%Config%/mcfcrt -o../../.Built/%Config%/libmcfcrt.a %* || exit /b 1
mcfbuild -p../../MCF/MCFBuild.mcfproj -s../../MCF -d../../.Built/%Config%/mcf -o../../.Built/%Config%/libmcf.a %* || exit /b 1
mcfbuild -p../../External/dlmalloc/MCFBuild.mcfproj -s../../External/dlmalloc -d../../.Built/%Config%/dlmalloc -o../../.Built/%Config%/libdlmalloc.a %* || exit /b 1
mcfbuild -p../../External/lzma/MCFBuild.mcfproj -s../../External/lzma -d../../.Built/%Config%/lzma -o../../.Built/%Config%/liblzma.a %* || exit /b 1
mcfbuild -p../../External/zlib/MCFBuild.mcfproj -s../../External/zlib -d../../.Built/%Config%/zlib -o../../.Built/%Config%/libz.a %* || exit /b 1

g++ %CXXFlags% -std=c++1y -Wnoexcept -Wall -Wextra -Wsign-conversion -Wsuggest-attribute=noreturn -pipe -mfpmath=both -march=core2 -masm=intel dll.cpp -shared -o ".dll-%Config%.dll" -I../.. -L../../.Built/%Config% -static -nostdlib -Wl,-e__MCF_DllStartup,--disable-runtime-pseudo-reloc,--disable-auto-import,--wrap=atexit,--wrap=malloc,--wrap=realloc,--wrap=calloc,--wrap=free %Lib% || exit /b 1

g++ dlltest.cpp ".dll-%Config%.dll" -std=c++1y -o ".%Config%.exe" -static -O3 -Wl,-s