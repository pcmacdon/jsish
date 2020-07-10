cd openssl &&
CC="musl-gcc -static -idirafter /usr/include/ -idirafter /usr/include/x86_64-linux-gnu/" ./Configure  no-shared linux-x86_64 &&
make build_libs
