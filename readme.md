# Compilador C-Minus

Compilador para a linguagem C-Minus desenvolvido como projeto da disciplina de Compiladores da UNIFESP.

## Estrutura do Projeto

O projeto está organizado da seguinte forma:

```
projeto/
├── build/              # Executáveis e arquivos gerados (cminus, codegen, *.tab.c, *.tab.h)
├── saida/              # Arquivos de saída do analisador léxico
├── ast/                # Árvores sintáticas abstratas (AST)
├── tabela/             # Tabelas de símbolos
├── intermediario/      # Código intermediário (Three-Address Code)
├── exemplo/            # Arquivos de teste em C-Minus
│   ├── sort.txt
│   ├── gcd.txt
│   ├── teste.txt
│   └── withoutmain.txt
├── lexer.l             # Analisador léxico (Flex)
├── parser.y            # Analisador sintático e semântico (Bison)
├── codegen.c           # Gerador de código intermediário
└── Makefile            # Arquivo de compilação
```

## Requisitos

- GCC (GNU Compiler Collection)
- Bison (GNU Parser Generator)
- Flex (Fast Lexical Analyzer)
- Make

### Instalação no Windows

Recomenda-se usar MinGW ou WSL (Windows Subsystem for Linux).

### Instalação no Linux/Mac

```bash
sudo apt-get install gcc bison flex make  # Ubuntu/Debian
```

## Como Compilar

### Compilação completa com testes

Execute todos os testes automaticamente:

```bash
make
```

Ou:

```bash
make test
```

Este comando irá:
1. Compilar o analisador léxico/sintático/semântico (`cminus`)
2. Compilar o gerador de código intermediário (`codegen`)
3. Criar as pastas de saída
4. Executar os testes com os arquivos de exemplo
5. Gerar todos os arquivos de saída

### Compilação individual

Compilar apenas o analisador:

```bash
make cminus
```

Compilar apenas o gerador de código:

```bash
make codegen
```

### Limpar arquivos gerados

```bash
make clean
```

## Como Usar o Compilador

### Uso Manual

#### 1. Análise Léxica, Sintática e Semântica

```bash
./build/cminus <arquivo_entrada.txt> <saida.txt> <arvore.txt> <tabela.txt>
```

**Parâmetros:**
- `arquivo_entrada.txt`: Código fonte em C-Minus
- `saida.txt`: Saída do analisador léxico (tokens)
- `arvore.txt`: Árvore sintática abstrata (AST)
- `tabela.txt`: Tabela de símbolos

**Exemplo:**

```bash
./build/cminus exemplo/sort.txt saida/saida_sort.txt ast/arvore_sort.txt tabela/tabela_sort.txt
```

#### 2. Geração de Código Intermediário

```bash
./build/codegen <arvore.txt> <codigo_intermediario.txt>
```

**Parâmetros:**
- `arvore.txt`: Árvore sintática abstrata gerada anteriormente
- `codigo_intermediario.txt`: Código intermediário de três endereços

**Exemplo:**

```bash
./build/codegen ast/arvore_sort.txt intermediario/intermediario_sort.txt
```

### Exemplo Completo

Para compilar um novo arquivo:

```bash
# 1. Certifique-se de que os executáveis existem
make cminus codegen

# 2. Crie as pastas de saída (se não existirem)
make dirs

# 3. Execute a análise
./build/cminus meu_programa.txt saida/saida_meu_programa.txt ast/arvore_meu_programa.txt tabela/tabela_meu_programa.txt

# 4. Gere o código intermediário
./build/codegen ast/arvore_meu_programa.txt intermediario/intermediario_meu_programa.txt
```

## Interpretando os Resultados

### 1. Saída do Analisador Léxico (`saida/`)

Contém a lista de tokens identificados no código fonte.

**Exemplo:**
```
[INT] [ID:minloc] [LPAREN] [INT] [ID:a] [LBRACKET] [RBRACKET] ...
```

### 2. Árvore Sintática Abstrata (`ast/`)

Representa a estrutura hierárquica do programa.

**Exemplo:**
```
Stmt: Prog
----Exp: Type: int
--------Stmt: Func: minloc
------------Exp: Type: int
----------------Stmt: Var: Id: a[]
------------Exp: Type: int
----------------Stmt: Var: Id: low
```

### 3. Tabela de Símbolos (`tabela/`)

Lista todas as variáveis e funções declaradas com seus tipos, escopos e linha de declaração.

**Exemplo:**
```
=== TABELA DE SIMBOLOS ===

Nome                 Tipo       Linha      Escopo          Parametros
--------------------------------------------------------------------------------
minloc               int        6          global          3         
a                    int        6          minloc          0         
low                  int        6          minloc          0         
```

### 4. Código Intermediário (`intermediario/`)

Código de três endereços (Three-Address Code) gerado a partir da AST.

**Exemplo:**
```
# Intermediate Code (Three-Address Code)
# Generated from AST

func minloc:
  var int i
  var int x
  var int k
  k = low
  t0 = a[low]
  x = t0
```

## Análise de Erros

O compilador detecta e reporta:

- **Erros Sintáticos**: Estrutura inválida do código
  ```
  [ERRO SINTATICO na linha 5]: syntax error
  ```

- **Erros Semânticos**: Variáveis não declaradas, tipos incompatíveis, etc.
  ```
  Erro semantico: variavel 'x' nao declarada.
  Erro semantico: funcao 'main' nao declarada.
  ```

## Exemplos Incluídos

### `sort.txt`
Programa de ordenação por seleção de um array.

### `gcd.txt`
Cálculo do máximo divisor comum (MDC).

### `teste.txt`
Programa de teste geral.

### `withoutmain.txt`
Exemplo de erro semântico (sem função main).

## Recursos da Linguagem C-Minus

- Tipos: `int`, `void`
- Arrays: `int a[10]`
- Funções com parâmetros
- Estruturas de controle: `if`, `else`, `while`
- Operações: `+`, `-`, `*`, `/`, `<`, `<=`, `>`, `>=`, `==`, `!=`
- I/O: `input()`, `output()`
- Comentários: `/* ... */` e `//`

## Correções de Memory Leaks

O projeto foi otimizado para prevenir vazamentos de memória:
- Liberação adequada de strings alocadas dinamicamente
- Gerenciamento correto de nós temporários da AST
- Controle de escopos com flag para strings literais

## Problemas Conhecidos

- Erro sintático para no primeiro erro encontrado (planejado melhorar)
