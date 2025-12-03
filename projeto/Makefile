all: test 

cminus: parser.y lexer.l
	bison -d parser.y
	flex lexer.l
	gcc -o cminus parser.tab.c lex.yy.c 

clean:
	rm -f cminus cminus.exe lex.yy.c parser.tab.c parser.tab.h saida_*.txt ast_*.txt

test: cminus
	./cminus exemplo/sort.txt saida_sort.txt ast_sort.txt
	./cminus exemplo/gcd.txt saida_gcd.txt ast_gcd.txt
	./cminus exemplo/withoutmain.txt saida_withoutmain.txt ast_withoutmain.txt
	./cminus exemplo/teste.txt saida_teste.txt ast_teste.txt

.PHONY: all clean test cminus
