#!/bin/bash

pkName=fsDigger;
folder_build=build;
mkdir --parents $folder_build
echo "build_dir=$folder_build"

perl -p \
-e 's/ (write\s?\()/ fsDigger_$1/;' \
-e 's/(finish_up).?\(\);/fsDigger_$1();/;' \
-e 's/^(int )?main\s?\(/$1dd_main (/;' \
-e 's/static (.*(ibuf|input_file|obuf|output_file|r_full|w_full|w_bytes|w_partial|seek_bytes|seek_records))/$1/;' \
-e 's/(\W)STDOUT_FILENO/$1DD_STDOUT_FILENO/;' \
coreutils/src/dd.c \
| perl -p -0 \
-e 's@#include.*"system.h"@$&\n#include "../../src/fsDigger.h"@;' \
-e 's/(static.void.(cleanup))(.*?{\R)(.*?)\R{2}(.*?)}/$1_in$3$4\n}\n\n$1_out$3$5\n}\n\n$1$3 $2_in();\n $2_out();\n}/s;' \
-e 's/static.(void.(finish_up|cleanup_in|cleanup_out|print_stats|process_signals))/$1/sg;'\
> coreutils/src/_dd.c

fileName=_dd; \
gcc -Wall \
-c \
-I ./coreutils/lib \
-o ./coreutils/build/$fileName.o \
./coreutils/src/$fileName.c \
&& ar rcs \
coreutils/build/lib_dd.a \
coreutils/build/_dd.o

fileName=main; \
g++ -Wall \
-std=gnu++17 \
-o ./$folder_build/$fileName.o \
-c ./$fileName.cpp \
&& \
g++ -Wall \
-std=gnu++17 \
-Wl,--allow-multiple-definition \
-Wl,--library-path=./lib/,--library=pthread \
-o ./$folder_build/$pkName \
./$folder_build/$fileName.o \
./coreutils/build/lib_dd.a \
./coreutils/lib/libcoreutils.a	\
./coreutils/src/libver.a

echo "Build completed..."
