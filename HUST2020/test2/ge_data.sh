#!bin/bash
RANDOM=1213

function rand(){
    tmp=$(($2-$1+1))
    num=$(($RANDOM+1000000000))
    echo $(($num%$tmp+$1))
}

for i in {1..100000}
do
    rnd=$(rand 0 10000000)
    echo $rnd >> data.txt
done
