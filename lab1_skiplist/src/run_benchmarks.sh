#!/bin/bash

# 결과 파일
output_file="output.csv"

# 결과 파일 초기화 (헤더 포함)
echo "WriteCount,ReadCount,Benchmark,InsertTime(µs),Read/DeleteTime(µs)" > "$output_file"

# 실험 파라미터 (자유롭게 수정 가능)
write_counts=(1000 5000 10000)
read_counts=(1000 5000 10000)

# 벤치마크 ID와 이름 매핑
declare -A benchmarks=(
  [0]="Sequential"
  [1]="RevSequential"
  [2]="Uniform"
  [3]="Zipfian"
  [4]="UniformDelete"
  [5]="ZipfianDelete"
  [6]="Scan"
)

# 실행 반복
for w in "${write_counts[@]}"; do
  for r in "${read_counts[@]}"; do
    for b in "${!benchmarks[@]}"; do
      echo "Running ${benchmarks[$b]} with W=$w, R=$r..."
      ./lab1_skiplist "$w" "$r" "$b"
    done
  done
done
