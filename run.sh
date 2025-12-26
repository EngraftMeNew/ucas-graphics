#!/usr/bin/env bash
set -u
set -o pipefail

ASSETS_DIR="assets"
OUTPUT_DIR="output"

# 三个比例 + 对应文件名标签（避免浮点转字符串带来的坑）
RATIOS=("0.25" "0.50" "0.75")
TAGS=("r25" "r50" "r75")

# 找可执行文件：优先根目录 ./simplify.exe
EXE=""
if [[ -x "./simplify.exe" ]]; then
  EXE="./simplify.exe"
elif [[ -x "./simplify" ]]; then
  EXE="./simplify"
elif [[ -x "./bin/simplify" ]]; then
  EXE="./bin/simplify"
elif [[ -x "./bin/simplify.exe" ]]; then
  EXE="./bin/simplify.exe"
else
  echo "[ERROR] Cannot find executable: ./simplify.exe (or ./simplify, ./bin/simplify, ./bin/simplify.exe)"
  exit 1
fi

mkdir -p "$OUTPUT_DIR"

shopt -s nullglob
objs=("$ASSETS_DIR"/*.obj)
if (( ${#objs[@]} == 0 )); then
  echo "[ERROR] No .obj found in $ASSETS_DIR"
  exit 1
fi

echo "[INFO] Using executable: $EXE"
echo "[INFO] Found ${#objs[@]} obj files in $ASSETS_DIR"
echo

fail_count=0

for src in "${objs[@]}"; do
  base="$(basename "$src")"
  name="${base%.*}"
  out_dir="$OUTPUT_DIR/$name"
  mkdir -p "$out_dir"

  echo "[INFO] Processing: $src"
  echo "       Output dir: $out_dir"

  for i in "${!RATIOS[@]}"; do
    r="${RATIOS[$i]}"
    tag="${TAGS[$i]}"
    dst="$out_dir/${name}_${tag}.obj"
    log="$out_dir/${name}_${tag}.log"

    echo "       -> $tag (ratio=$r)"
    if ! "$EXE" "$src" "$dst" "$r" >"$log" 2>&1; then
      echo "       [WARN] Failed: $name $tag (see $log)"
      fail_count=$((fail_count + 1))
    fi
  done

  echo
done

if (( fail_count > 0 )); then
  echo "[DONE] Finished with $fail_count failure(s). Check logs under $OUTPUT_DIR/*/*.log"
else
  echo "[DONE] All meshes processed successfully. Results are under: $OUTPUT_DIR/"
fi
