#!/bin/zsh

setopt NULL_GLOB

for file in *.vert *.frag *.comp; do
  echo "Compiling $file"
  glslangValidator -V "$file" -o "$file.spv"
done