all: test 

cminus: parser.y lexer.l
	bison -d parser.y
	flex lexer.l
	gcc -o cminus parser.tab.c lex.yy.c 

clean:
	rm -f cminus lex.yy.c parser.tab.c parser.tab.h

test: cminus
	./cminus exemplo/sort.txt saida_sort.txt
	./cminus exemplo/gcd.txt saida_gcd.txt
	./cminus exemplo/withoutmain.txt saida_withoutmain.txt

.PHONY: all clean test cminus
