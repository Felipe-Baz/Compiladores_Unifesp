all: test 

cminus: parser.y lexer.l
	mkdir -p build
	bison -d parser.y
	flex lexer.l
	gcc -o build/cminus parser.tab.c lex.yy.c
	mv parser.tab.c parser.tab.h lex.yy.c build/

codegen: codegen.c
	mkdir -p build
	gcc -o build/codegen codegen.c

dirs:
	mkdir -p saida ast tabela intermediario

clean:
	rm -rf build saida ast tabela intermediario

test: cminus codegen dirs
	./build/cminus exemplo/sort.txt saida/saida_sort.txt ast/arvore_sort.txt tabela/tabela_sort.txt
	./build/codegen ast/arvore_sort.txt intermediario/intermediario_sort.txt
	./build/cminus exemplo/gcd.txt saida/saida_gcd.txt ast/arvore_gcd.txt tabela/tabela_gcd.txt
	./build/codegen ast/arvore_gcd.txt intermediario/intermediario_gcd.txt
	./build/cminus exemplo/withoutmain.txt saida/saida_withoutmain.txt ast/arvore_withoutmain.txt tabela/tabela_withoutmain.txt
	./build/codegen ast/arvore_withoutmain.txt intermediario/intermediario_withoutmain.txt
	./build/cminus exemplo/teste.txt saida/saida_teste.txt ast/arvore_teste.txt tabela/tabela_teste.txt
	./build/codegen ast/arvore_teste.txt intermediario/intermediario_teste.txt

.PHONY: all clean test cminus codegen dirs
