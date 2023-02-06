#!/bin/bash
echo "Running..."
make
rm outputD.txt
rm output.txt
for (( i = 1; i < 19; i++ )); do
	#statements
	if ((i!=16)); then
		#statements
		t="testcase$i.my"
		t1="testcase$i.ll"
		echo "Compile and run $t -----------------"
		./mylang2ir $t
		cat "testcase$i.txt" >> outputD.txt
		lli $t1 >> output.txt
	fi
done
	echo "Comparing ..." 
	dos2unix outputD.txt
	diff -s outputD.txt output.txt | cat -t || [ $? -eq 1 ]
