#!/bin/bash

set -e

pkName=fsDigger;
build_dir=build;
mkdir --parents $build_dir

if [ $COREUTILS_TAG ]; then
	COREUTILS_DIR="coreutils_$COREUTILS_TAG"
	rm -rf $COREUTILS_DIR
	mkdir --parents $COREUTILS_DIR
	git clone --branch $COREUTILS_TAG https://github.com/coreutils/coreutils.git "$COREUTILS_DIR"
	cd $COREUTILS_DIR
	./bootstrap
	CFLAGS="-Wsuggest-attribute=malloc" ./configure
	perl -i -p \
		-e 's/WERROR_CFLAGS.*=.*/WERROR_CFLAGS =/' \
		./Makefile
	make
	cd ..
fi

mkdir --parents $COREUTILS_DIR/$build_dir

perl -p \
-e 's/ (write\s?\()/ fsDigger_$1/;' \
-e 's/(finish_up).?\(\);/fsDigger_$1();/;' \
-e 's/^(int )?main\s?\(/$1dd_main (/;' \
-e 's/static (.*(ibuf|input_file|obuf|output_file|r_full|w_full|w_bytes|w_partial|seek_bytes|seek_records))/$1/;' \
-e 's/(\W)STDOUT_FILENO/$1DD_STDOUT_FILENO/;' \
$COREUTILS_DIR/src/dd.c \
| perl -p -0 \
-e 's@#include.*"system.h"@$&\n#include "../../src/fsDigger.h"@;' \
-e 's/(static.void.(cleanup))(.*?{)(.*?\R)(\R.*?)\R(\R.*?if.*?)}/$1_in$3$5\n}\n\n$1_out$3$6\n}\n\n$1$3$4 $2_in();\n $2_out();\n}/s;' \
-e 's/static.(void.(finish_up|cleanup_in|cleanup_out|print_stats|process_signals))/$1/sg;'\
> $COREUTILS_DIR/src/_dd.c
#-e 's/(static.void.(cleanup))(.*?{\R)(.*?)\R{2}(.*?)}/$1_in$3$4\n}\n\n$1_out$3$5\n}\n\n$1$3 $2_in();\n $2_out();\n}/s;' \

fileName=_dd; \
gcc -Wall \
-c \
-I ./$COREUTILS_DIR/lib \
-o ./$COREUTILS_DIR/$build_dir/$fileName.o \
./$COREUTILS_DIR/src/$fileName.c \
&& ar rcs \
$COREUTILS_DIR/$build_dir/lib_dd.a \
$COREUTILS_DIR/$build_dir/_dd.o

fileName=main; \
g++ -Wall \
-std=gnu++17 \
-o ./$build_dir/$fileName.o \
-c ./$fileName.cpp \
&& \
g++ -Wall \
-std=gnu++17 \
-Wl,--allow-multiple-definition \
-Wl,--library-path=./lib/,--library=pthread \
-o ./$build_dir/$pkName \
./$build_dir/$fileName.o \
./$COREUTILS_DIR/$build_dir/lib_dd.a \
./$COREUTILS_DIR/lib/libcoreutils.a	\
./$COREUTILS_DIR/src/libver.a

echo	"build_dir = $build_dir" \
		"Build completed..."
