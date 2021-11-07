rm -f out.txt
gcc pzip.c -o pzip -Wall -Werror -pthread -O
pzip sample.txt > out.txt