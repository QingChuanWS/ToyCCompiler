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

source "$(cd "$(dirname "$0")" && pwd)/preprocessing.sh"

# Check if the number of arguments is correct
if [ "$#" -ne 3 ]; then
  echo "Error: Two arguments are required"
  echo "Usage: $0 <src_folder> <output_folder> <compiler>"
  exit 1
fi

src_folder="$1"
output_folder="$2"
CXX="$3"
compiler_path="./toyc"

# Check if src_folder exists
if [ ! -d "$src_folder" ]; then
  echo "Error: test source code folder does not exist: $src_folder"
  exit 1
fi

# Check if output_folder exists, create it if it doesn't
if [ ! -d "$output_folder" ]; then
  mkdir -p "$output_folder"
  echo "intermediate result folder created: $output_folder"
fi

# Check if the file exists
if [ ! -e "$compiler_path" ]; then
  echo "Error: Compiler does not exist: $compiler_path"
  exit 1
fi

function_check() {
  local source_dir="$1"
  local output_dir="$2"
  local compiler="$3"

  if [ ! -d "$output_dir" ]; then
    mkdir -p "$output_dir"
  fi

  processed_files=0
  src_files_count=$(find "$source_dir" -name "*.c" | wc -l)

  for file in "$source_dir"/*.c; do
    if [ -f "$file" ]; then
      echo "$(basename "$file") Compile Function Check..."

      tmp_output_log=$(mktemp)
      tmp_output_asm=$(mktemp)".s"

      binery=${file%%.*}

      $compiler -o $tmp_output_asm $file
      cat $tmp_output_asm
      $CXX -o $binery $tmp_output_asm -xc $src_folder"/common"
      echo $binery
      $binery || exit 1
    fi
  done
  echo
}

preprocessing_c_files $src_folder $output_folder
function_check $output_folder $output_folder $compiler_path
echo "All checks passed"