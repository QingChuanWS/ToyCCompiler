#!/bin/sh
###
 # This project is exclusively owned by QingChuanWS and shall not be used for
 # commercial and profitting purpose without QingChuanWS's permission.
 # 
 # @Author: bingshan45@163.com
 # Github: https://github.com/QingChuanWS
 # @Description: 
 # 
 # Copyright (c) 2023 by QingChuanWS, All Rights Reserved. 
### 
tmp=`mktemp -d /tmp/toyc-test-XXXXXX`
echo > $tmp/empty.c

check() {
    if [ $? -eq 0 ]; then
        echo "testing $1 ... passed"
    else
        echo "testing $1 ... failed"
        exit 1
    fi
}

# -o
rm -f $tmp/out

build_path=$1

$build_path"/toyc" -o $tmp/out $tmp/empty.c
[ -f $tmp/out ]
check -o

# --help
$build_path"/toyc" --help 2>&1 | grep -q toyc
check --help

echo OK
