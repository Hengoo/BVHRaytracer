python raytracerTemplateGen.py
ispc src/ISPC/rayTracer.ispc -o src/ISPC/ISPCBuild/rayTracer.obj -h src/ISPC/ISPCBuild/rayTracer_ISPC.h --target=sse4-i32x4
:: --target=sse4-i32x4
:: --target=avx2-i32x8
:: --emit-asm  for assembler code
:: --addressing=64  for 64 bit addressing so it can manage more triangles