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
  echo "start "$ut" memory check";
  src_code=$gene_dir"/source.c"
  output=$gene_dir"/tmp.txt"
  tmpfile=$gene_dir"/tmp.s"

  $CXX -o- -E -P -C $ut_name > $src_code

  valgrind --tool=memcheck --leak-check=full --track-origins=yes $complier $src_code -o $tmpfile > "$output" 2>&1
  grep -q "ERROR SUMMARY: 0 errors" "$output"
  ret=$?
  
  if [ $ret != 0 ]; then
    echo "test case : "
    echo "$1"
    read -p "find memory error, check memory, see tmp.txt for the details." char
  fi
  echo $ut" memory check pass."
  echo "---------------------------------" 
}

function_check(){
  ut_assemly=$ut".s"
  $CXX -o- -E -P -C $ut_name | $complier -o $ut_assemly -
  $CXX -o $ut $ut_assemly -xc $bash_dir"/common"
  echo $ut
  $ut || exit 1;
}

# build path of project.
build=$1
# if using memory check or function check
is_memory=$2

# self-compiler
complier=$build"/toyc"
# currently shell file path
bash_dir=`dirname $0`
# parent compiler 
CXX=/usr/bin/g++-10 

# generate test directory.
gene_dir=$build"/test" 
if [ ! -d $gene_dir  ];then
  mkdir $gene_dir
fi

files=$(ls $bash_dir)
for file in $files
do
  # unit test name
  ut_name=$bash_dir/$file
  # unit test
  ut=$gene_dir/${file%%.*}

  if [ "${file##*.}"x = "c"x ];then
    if [ "$2" == "memo" ];then
      memory_check
    elif [ "$2" == "func" ]; then
      function_check
    else
      echo "unknown test arguement."
    fi
  fi
done
