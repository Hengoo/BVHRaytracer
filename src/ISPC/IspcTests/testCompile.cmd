ispc constAll.ispc -o ISPCBuild/constAll.s --target=sse4-i32x4 --emit-asm
ispc paramAll.ispc -o ISPCBuild/paramAll.s --target=sse4-i32x4 --emit-asm
ispc paramLoop.ispc -o ISPCBuild/paramLoop.s --target=sse4-i32x4 --emit-asm
ispc paramAllV2.ispc -o ISPCBuild/paramAllV2.s --target=sse4-i32x4 --emit-asm
ispc paramLoopV2.ispc -o ISPCBuild/paramLoopV2.s --target=sse4-i32x4 --emit-asm
ispc paramMemory.ispc -o ISPCBuild/paramMemory.s --target=sse4-i32x4 --emit-asm


:: --target=sse4-i32x4
:: --target=avx2-i32x8
:: --emit-asm  for assembler code