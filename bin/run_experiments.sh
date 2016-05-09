#!/bin/bash
# 1:markov_bin 2:orig_bin 3: test_files_folder 4:out_file

function run_markov_gpu
{
  bin=$1
  test_folder=$2
  output_file=$3
  test_file=$4
  devices=$5

  printf "\n===========================================\
          \nTest file: %s\nDevices: %s\
          \n===========================================\n" \
          $test_file $devices | tee -a $output_file
  (time ./$bin -f $2/$test_file -S stats/rockyou.wstat -T 26 -d $devices) 2>> $output_file
}

function run_markov_cpu
{
  bin=$1
  test_folder=$2
  output_file=$3
  test_file=$4
  threads=$5

  printf "\n===========================================\
          \nTest file: %s\nThreads: %d\
          \n===========================================\n" \
          $test_file $threads | tee -a $output_file
  (time ./$bin -f $2/$test_file -S stats/rockyou.wstat -T 26 -t $threads -c) 2>> $output_file
}

function run_brute_gpu
{
  bin=$1
  test_folder=$2
  output_file=$3
  test_file=$4
  devices=$5

  printf "\n===========================================\
          \nTest file: %s\nDevices: %s\
          \n===========================================\n" \
          $test_file $devices | tee -a $output_file
  (time ./$bin -f $2/$test_file -d $devices) 2>> $output_file
}

function run_brute_cpu
{
  bin=$1
  test_folder=$2
  output_file=$3
  test_file=$4
  threads=$5

  printf "\n===========================================\
          \nTest file: %s\nThreads: %d\
          \n===========================================\n" \
          $test_file $threads | tee -a $output_file
  (time ./$bin -f $2/$test_file -t $threads -c) 2>> $output_file
}

markov_bin=$1
brute_bin=$2
test_files_folder=$3
out_file=$4

run_brute_gpu $brute_bin $test_files_folder $out_file "b_zzzzzzzz.pdf" "0:1:2048000"
run_brute_gpu $brute_bin $test_files_folder $out_file "b_bbbbbbbbb.pdf" "0:1:2048000,0:2:2048000"
run_brute_gpu $brute_bin $test_files_folder $out_file "b_ccccccccc.pdf" "0:1:2048000,0:2:2048000,0:3:2048000"

run_markov_gpu $markov_bin $test_files_folder $out_file "m_wj4pfpfp.pdf" "0:1:2048000"
run_markov_gpu $markov_bin $test_files_folder $out_file "m_milarilar.pdf" "0:1:2048000,0:2:2048000"
run_markov_gpu $markov_bin $test_files_folder $out_file "m_sholishol.pdf" "0:1:2048000,0:2:2048000,0:3:2048000"

# run_markov_cpu $markov_bin $test_files_folder $out_file "r5_3.pdf" 4
# run_brute_cpu $brute_bin $test_files_folder $out_file "r5_3.pdf" 4
