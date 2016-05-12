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

# PDF GPU

run_brute_gpu $brute_bin $test_files_folder $out_file "b_zzzzzzzz.pdf" "0:1:2048000"
run_brute_gpu $brute_bin $test_files_folder $out_file "b_bbbbbbbbb.pdf" "0:1:2048000,0:2:2048000"
run_brute_gpu $brute_bin $test_files_folder $out_file "b_ccccccccc.pdf" "0:1:2048000,0:2:2048000,0:3:2048000"

run_markov_gpu $markov_bin $test_files_folder $out_file "m_wj4pfpfp.pdf" "0:1:2048000"
run_markov_gpu $markov_bin $test_files_folder $out_file "m_milarilar.pdf" "0:1:2048000,0:2:2048000"
run_markov_gpu $markov_bin $test_files_folder $out_file "m_sholishol.pdf" "0:1:2048000,0:2:2048000,0:3:2048000"

# ZIP GPU

run_brute_gpu $brute_bin $test_files_folder $out_file "b_kkkkkk.zip" "0:1:204800"
run_markov_gpu $markov_bin $test_files_folder $out_file "m_dm1sm1.zip" "0:1:204800"

run_brute_gpu $brute_bin $test_files_folder $out_file "b_uuuuuu.zip" "0:1:204800,0:2:204800"
run_markov_gpu $markov_bin $test_files_folder $out_file "m_f4jtkj.zip" "0:1:204800,0:2:204800"

run_brute_gpu $brute_bin $test_files_folder $out_file "b_zzzzzz.zip" "0:1:204800,0:2:204800,0:3:204800"
run_markov_gpu $markov_bin $test_files_folder $out_file "m_wj4pfp.zip" "0:1:204800,0:2:204800,0:3:204800"

# ZIP GPU

run_brute_gpu $brute_bin $test_files_folder $out_file "b_bbbbbbbb.doc" "0:1:204800"
run_markov_gpu $markov_bin $test_files_folder $out_file "m_milarila.doc" "0:1:204800"

run_brute_gpu $brute_bin $test_files_folder $out_file "b_dddddddd.doc" "0:1:204800,0:2:204800"
run_markov_gpu $markov_bin $test_files_folder $out_file "m_amoooooo.doc" "0:1:204800,0:2:204800"

run_brute_gpu $brute_bin $test_files_folder $out_file "b_ffffffff.doc" "0:1:204800,0:2:204800,0:3:204800"
run_markov_gpu $markov_bin $test_files_folder $out_file "m_14444444.doc" "0:1:204800,0:2:204800,0:3:204800"

# ZIP CPU

run_brute_cpu $brute_bin $test_files_folder $out_file "b_ddddd.zip" 1
run_markov_cpu $markov_bin $test_files_folder $out_file "m_amooo.zip" 1

run_brute_cpu $brute_bin $test_files_folder $out_file "b_hhhhh.zip" 2
run_markov_cpu $markov_bin $test_files_folder $out_file "m_l1863.zip" 2

run_brute_cpu $brute_bin $test_files_folder $out_file "b_rrrrr.zip" 4
run_markov_cpu $markov_bin $test_files_folder $out_file "m_h3bf0.zip" 4

# PDF CPU

run_brute_cpu $brute_bin $test_files_folder $out_file "b_kkkkkkk.pdf" 1
run_markov_cpu $markov_bin $test_files_folder $out_file "m_dm1sm1s.pdf" 1

run_brute_cpu $brute_bin $test_files_folder $out_file "b_vvvvvvv.pdf" 2
run_markov_cpu $markov_bin $test_files_folder $out_file "m_4df3k8k.pdf" 2

run_brute_cpu $brute_bin $test_files_folder $out_file "b_akkkkkkk.pdf" 4
run_markov_cpu $markov_bin $test_files_folder $out_file "m_dm1sm1st.pdf" 4

# DOC CPU

run_brute_cpu $brute_bin $test_files_folder $out_file "b_bbbbbbb.doc" 1
run_markov_cpu $markov_bin $test_files_folder $out_file "m_milaril.doc" 1

run_brute_cpu $brute_bin $test_files_folder $out_file "b_bbbbbbb.doc" 2
run_markov_cpu $markov_bin $test_files_folder $out_file "m_milaril.doc" 2

run_brute_cpu $brute_bin $test_files_folder $out_file "b_bbbbbbb.doc" 4
run_markov_cpu $markov_bin $test_files_folder $out_file "m_milaril.doc" 4
