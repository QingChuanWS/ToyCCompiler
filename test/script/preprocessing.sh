###
 # This project is exclusively owned by QingChuanWS and shall not be used for
 # commercial and profitting purpose without QingChuanWS's permission.
 # 
 # @ Author: bingshan45@163.com
 # @ Github: https://github.com/QingChuanWS
 # @ Description: 
 # 
 # Copyright (c) 2023 by $QingChuanWS, All Rights Reserved. 
### 
preprocessing_c_files() {
  local source_dir="$1"
  local output_dir="$2"
  local compiler=/usr/bin/g++-10

  if [ ! -d "$output_dir" ]; then
    mkdir "$output_dir"
  fi

  for file in "$source_dir"/*.c; do
    if [ -f "$file" ]; then
      filename=$(basename "$file")
      result_file="$output_dir/$filename"

      $compiler -o- -E -P -C $file > $result_file
      chmod a+x $result_file
    fi
  done
  echo "Preprocess Completed!"
  echo
}
