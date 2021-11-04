rm -f out.txt
gcc pzip.c -o pzip -Wall -Werror -pthread -O
pzip in1.txt > out.txt