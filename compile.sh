#!/bin/bash
set -e

#Compile .cu file
echo $1 --machine 64 or 32
echo $2 -G

# compute_89 for RTX 4090
nvcc -D__USE_OPENCL__ -I"./" -I"/usr/local/cuda/include" -ftz=true \
   -prec-div=false -prec-sqrt=false -arch=compute_89 -O3 --machine $1 $2 -ptx \
   -o clguetzli/clguetzli.cu.ptx$1 clguetzli/clguetzli.cu

#copy to ./bin/Release
mkdir -p bin/Release/clguetzli
cp clguetzli/clguetzli.cu.ptx$1 bin/Release/clguetzli/clguetzli.cu.ptx$1
cp clguetzli/clguetzli.cl bin/Release/clguetzli/clguetzli.cl
cp clguetzli/clguetzli.cl.h bin/Release/clguetzli/clguetzli.cl.h
