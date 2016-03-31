#!/bin/bash

# Arguments:
# $1 = model 
# $2 threshold 
# $3 limits

model=$1

# Split limits into array
limits=$(echo $3 | tr "," "\n")

# Output file name preffix
output="results/rockyou-phpbb/t"

output=$output$2
for x in $limits
do
  output=$output"_"$x
done

# Model suffix
m_suffix=""
if [ "$1" == "layered" ]
then
  m_suffix="l"
fi

output=$output$m_suffix".txt"

# Argument with limits
limits=""
if [ $3 ]
then
  limits="--limits "$3
fi

# Run test
if [ -e "$output" ]
then
  echo "File $output already exists"
else
  nice -n 19 ./test --stat stats/rockyou.wstat -f dictionaries/phpbb_1-7.dic --max 7 --model $model --threshold $2 $limits -v > $output 
fi

# Cat result
cat $output | tail -n 1

