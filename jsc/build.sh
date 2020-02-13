#!/bin/bash

gcc -g "./src/$1.c" `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0 i3ipc-glib-1.0` -o "./bin/$1.so" -fPIC -shared