all: test 

cminus: parser.y lexer.l
	bison -d parser.y
	flex lexer.l
	gcc -o cminus parser.tab.c lex.yy.c 

clean:
	rm -f cminus lex.yy.c parser.tab.c parser.tab.h saida_*.txt arvore_*.txt tabela_*.txt

test: cminus
	./cminus exemplo/sort.txt saida_sort.txt arvore_sort.txt tabela_sort.txt
	./cminus exemplo/gcd.txt saida_gcd.txt arvore_gcd.txt tabela_gcd.txt
	./cminus exemplo/withoutmain.txt saida_withoutmain.txt arvore_withoutmain.txt tabela_withoutmain.txt
	./cminus exemplo/teste.txt saida_teste.txt arvore_teste.txt tabela_teste.txt

.PHONY: all clean test cminus
