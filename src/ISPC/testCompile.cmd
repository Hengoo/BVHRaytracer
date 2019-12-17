ispc src/ISPC/test.ispc -o src/ISPC/ISPCBuild/test.obj -h src/ISPC/ISPCBuild/test_ISPC.h --target=sse4-i32x4 --emit-asm

:: --target=sse4-i32x4
:: --target=avx2-i32x8
:: --emit-asm  for assembler code