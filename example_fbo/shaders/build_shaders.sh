#!/bin/sh

glslangValidator -V100 src/main.vert -o spv/main.vert.spv
glslangValidator -V100 src/main.frag -o spv/main.frag.spv
glslangValidator -V100 src/buf.vert -o spv/buf.vert.spv
glslangValidator -V100 src/buf.frag -o spv/buf.frag.spv