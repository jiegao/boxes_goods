#! /bin/bash

rm -f single.log
../build/bin/checkBoxesGoods -input test_list.txt > single.log
echo "../build/bin/checkBoxesGoods -input test_list.txt > single.log"
