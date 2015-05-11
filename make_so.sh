gcc -c -fPIC -llua -I/usr/include/lua5.2 hv.c
gcc -shared -o hv.so hv.o
