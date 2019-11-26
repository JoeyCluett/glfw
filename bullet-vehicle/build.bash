#!/bin/bash

#g++ -o main main.cpp -std=c++11 -march=native -O3 -I/usr/include/reactphysics3d/ -lreactphysics3d

bulletlibs="-I/usr/local/include/bullet -lBulletDynamics -lBulletCollision -lLinearMath -lBulletSoftBody"
stdopts="-std=c++11 -march=native -O3 -Wall"

g++ `pkg-config --cflags glfw3` -o main main.cpp `pkg-config --libs glfw3` -lGLEW -lGL ${bulletlibs} ${stdopts}
