all: test 

cminus: parser.y lexer.l
	bison -d parser.y
	flex lexer.l
	gcc -o cminus parser.tab.c lex.yy.c 

codegen: codegen.c
	gcc -o codegen codegen.c

clean:
	rm -f cminus codegen lex.yy.c parser.tab.c parser.tab.h saida_*.txt arvore_*.txt tabela_*.txt intermediario_*.txt

test: cminus codegen
	./cminus exemplo/sort.txt saida_sort.txt arvore_sort.txt tabela_sort.txt
	./codegen arvore_sort.txt intermediario_sort.txt
	./cminus exemplo/gcd.txt saida_gcd.txt arvore_gcd.txt tabela_gcd.txt
	./codegen arvore_gcd.txt intermediario_gcd.txt
	./cminus exemplo/withoutmain.txt saida_withoutmain.txt arvore_withoutmain.txt tabela_withoutmain.txt
	./codegen arvore_withoutmain.txt intermediario_withoutmain.txt
	./cminus exemplo/teste.txt saida_teste.txt arvore_teste.txt tabela_teste.txt
	./codegen arvore_teste.txt intermediario_teste.txt

.PHONY: all clean test cminus codegen
