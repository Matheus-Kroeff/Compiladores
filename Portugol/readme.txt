Projeto - Compiladores fase 1: análise léxica e sintática
---------------------------------------------------------
- Matheus Leonardo Cardoso Kroeff - 10426434
- Naoto Ushizaki - 10437445
---------------------------------------------------------
OBS: O projeto foi rodado no WSL (claramente para poder testar com valgrind)

Tree do projeto: 
.
├── compilador
├── compilador.c
├── input.txt
├── lexical_analyser.c
└── syntatic_analyser.c

Compilação do projeto (no WSL do Vscode):  
gcc -Wall -Wno-unused-result -g -Og compilador.c -o compilador

Execução [./executavel nome_arquivo.txt]: ./compilador input.txt
Valgrind execução: valgrind --leak-check=yes ./compilador input.txt 

Output: Tentamos deixar o mais fiel/autêntico com o modelo de output 
dos testes do professor.
