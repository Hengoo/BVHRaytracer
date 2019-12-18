set OPT=--emit-asm --target=sse4-i32x4 --x86-asm-syntax=intel

ispc constAll.ispc -o ISPCBuild/constAll.s %OPT%
ispc paramAll.ispc -o ISPCBuild/paramAll.s %OPT%
ispc paramLoop.ispc -o ISPCBuild/paramLoop.s %OPT%
ispc paramAllV2.ispc -o ISPCBuild/paramAllV2.s %OPT%
ispc paramLoopV2.ispc -o ISPCBuild/paramLoopV2.s %OPT%
ispc paramMemory.ispc -o ISPCBuild/paramMemory.s %OPT%


:: --target=sse4-i32x4
:: --target=avx2-i32x8
:: --emit-asm  for assembler code