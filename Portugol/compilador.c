/*
- Matheus Leonardo Cardoso Kroeff - 10426434
- Naoto Ushizaki - 10437445
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexical_analyser.c"
#include "syntatic_analyser.c"

extern FILE *arquivo;
extern TInfoAtomo lookahead;
extern TInfoAtomo obter_atomo();

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso correto: %s <input.txt>\n", argv[0]);
        return 1;
    }

    arquivo = fopen(argv[1], "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo: %s\n", argv[1]);
        return 1;
    }

    // inicializa o lookahead
    lookahead = obter_atomo();

    // chama a análise sintática
    programa();

    printf("Código rodou, nenhum erro encontrado. %d linhas analisadas -> programa sintaticamente correta\n", lookahead.linha);

    fclose(arquivo);
    return 0;

}
