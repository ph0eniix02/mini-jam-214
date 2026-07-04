#!/bin/bash
# compile the game
gcc -W -Wall -pedantic main.c -I./vendored/raylib/src/ -L./vendored/raylib/src/ -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o game

# create the compile_commands.json for clangd (just for development)
bear -- gcc -W -Wall -pedantic main.c -I./vendored/raylib/src/ -L./vendored/raylib/src/ -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o game

