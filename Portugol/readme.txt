Projeto - Compiladores fase 1: análise léxica e sintática

Tree do projeto: 
.
├── compilador
├── compilador.c
├── input.txt
├── lexical_analyser.c
└── syntatic_analyser.c

Compilação do projeto (no wsl):  
gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador

Execução: ./compilador input.txt
Valgrind execução: valgrind --leak-check=yes ./compilador input.txt 

