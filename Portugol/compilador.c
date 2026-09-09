/*
- Matheus Leonardo Cardoso Kroeff - 10426434
- Naoto Ushizaki - 10437445
*/

#include <stdio.h>
#include <stdlib.h>
#include "lexico.c"
#include "sintatico.c"

extern FILE *arquivo;
extern TInfoAtomo lookahead;
extern TInfoAtomo obter_atomo();
extern void programa();

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso correto: %s <arquivo_fonte.txt>\n", argv[0]);
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

    printf("Código rodou, nenhum erro encontrado.\n");

    fclose(arquivo);
    return 0;
}
