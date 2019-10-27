#!/bin/bash


g++ `pkg-config --cflags glfw3` -o main main.cpp `pkg-config --libs glfw3` -lGLEW -lGL -std=c++11 -march=native -O3 -Wall
