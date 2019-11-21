#!/bin/bash

#g++ -o main main.cpp -std=c++11 -march=native -O3 -I/usr/include/reactphysics3d/ -lreactphysics3d


g++ `pkg-config --cflags glfw3` -o main main.cpp `pkg-config --libs glfw3` -lGLEW -lGL -lreactphysics3d -I/usr/include/reactphysics3d -std=c++11 -march=native -O3 -Wall
