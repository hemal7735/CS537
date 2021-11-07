rm -f out.txt
gcc pzip.c -o pzip -Wall -Werror -pthread -O
pzip tests/1.in tests/4.in tests/1.in > out.txt