#!/bin/bash
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
memory_check(){
  src_code=$gene_dir"/source.c"
  output=$gene_dir"/tmp.txt"
  out_file=$gene_dir"/tmp.s"

  $COMPILER -o- -E -P -C $test_name > $src_code

  valgrind --tool=memcheck --leak-check=full --track-origins=yes $complier $src_code -o $out_file > "$output" 2>&1
  grep -q "ERROR SUMMARY: 0 errors" "$output"
  ret=$?
  
  if [ $ret != 0 ]; then
    echo "test case : "
    echo "$1"
    read -p "find memory error, check complier memory, see tmp.txt for the details." char
  fi
  echo "Memory check pass."; echo;
}

function_check(){
  $COMPILER -o- -E -P -C $test_name | $complier -o $gene_file".s" -
  $COMPILER -o $gene_file $gene_file".s" -xc $test_path"/common"
  echo $gene_file
  $gene_file || exit 1;
}

build_path=$1

complier=$build_path"/toyc" # self-compiler
test_path=`dirname $0` # currently shell file path
COMPILER=/usr/bin/g++-10 # parent compiler

gene_dir=$build_path"/test" 
if [ ! -d $gene_dir  ];then
  mkdir $gene_dir
fi

files=$(ls $test_path)
for filename in $files
do
  test_name=$test_path/$filename
  gene_file=$gene_dir/${filename%%.*}".out"

  if [ "${filename##*.}"x = "c"x ];then
    memory_check
    function_check
  fi
done
