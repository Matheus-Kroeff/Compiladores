/*
- Matheus Leonardo Cardoso Kroeff - 10426434
- Naoto Ushizaki - 10437445
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRIME_NUMER 211

typedef enum { CAT_VARIAVEL, CAT_PARAMETRO, CAT_FUNCAO, CAT_PROCEDIMENTO } TCategoria;

typedef struct _TNo {
   char ID[16];
   int endereço;       
   TCategoria categoria;
   int escopo;          // 0 = global, 1 = local
   struct _TNo *prox;
} TNo;

typedef struct {
    TNo *entradas[PRIME_NUMER];
} TTabelaSimbolos;

TTabelaSimbolos tabela;   // global: começa toda NULL

// Protótipos das funções
int hashMack(char *s); 
void libera_tabela();
TNo *busca(char *id);
void insere(char *id, TCategoria categoria, int escopo, int endereco, int linha);
void remove_locais();
void imprime_tabela();

// Busca na Tabela de Símbolos
TNo *busca(char *id) {
    int i = hashMack(id);
    for (TNo *p = tabela.entradas[i]; p != NULL; p = p->prox)
        if (strcmp(p->ID, id) == 0)
            return p;
    return NULL;
}

// Inserção na Tabela de Símbolos
void insere(char *id, TCategoria categoria, int escopo, int endereco, int linha) {
    int i = hashMack(id);

    // Verifica se já existe um nó com o mesmo ID e no mesmo escopo
    for (TNo *p = tabela.entradas[i]; p != NULL; p = p->prox) {
        if (strcmp(p->ID, id) == 0 && p->escopo == escopo) {
            printf("Erro semantico na linha %d -> Identificador '%s' ja declarado no escopo %d\n", linha, id, escopo);
            libera_tabela();
            exit(1);
        }
    }

    // Alocação e preenchimento do novo nó
    TNo *novo = (TNo *) malloc(sizeof(TNo));
    if (!novo) {
        printf("Erro de alocação de memória!\n");
        libera_tabela();
        exit(1);
    }

    strncpy(novo->ID, id, 15);
    novo->ID[15] = '\0';
    novo->categoria = categoria;
    novo->escopo = escopo;
    novo->endereço = endereco;

    // Inserção no início da lista encadeada da entrada i
    novo->prox = tabela.entradas[i];
    tabela.entradas[i] = novo;
}

// Remove todos os símbolos com escopo local (escopo == 1) ao fim de uma rotina
void remove_locais() {
    for (int i = 0; i < PRIME_NUMER; i++) {
        TNo **pp = &tabela.entradas[i];
        while (*pp != NULL) {
            if ((*pp)->escopo == 1) {
                TNo *removido = *pp;
                *pp = removido->prox;
                free(removido);
            } else {
                pp = &((*pp)->prox);
            }
        }
    }
}

// Impressão da Tabela de Símbolos
void imprime_tabela() {
    const char *nomes_categorias[] = {
        "VARIAVEL",
        "PARAMETRO",
        "FUNCAO",
        "PROCEDIMENTO"
    };

    printf("\n--- TABELA DE SÍMBOLOS ---\n");
    for (int i = 0; i < PRIME_NUMER; i++) {
        TNo *p = tabela.entradas[i];
        while (p != NULL) {
            if (p->categoria == CAT_VARIAVEL || p->categoria == CAT_PARAMETRO) {
                printf("[%d] ID: %-15s | Categoria: %-12s | Escopo: %d | Endereço: %d\n", 
                       i, p->ID, nomes_categorias[p->categoria], p->escopo, p->endereço);
            } else {
                printf("[%d] ID: %-15s | Categoria: %-12s | Escopo: %d\n", 
                       i, p->ID, nomes_categorias[p->categoria], p->escopo);
            }
            p = p->prox;
        }
    }
    printf("---------------------------\n");
}

// Liberação da Memória da Tabela de Símbolos
void libera_tabela() {
    for (int i = 0; i < PRIME_NUMER; i++) {
        TNo *p = tabela.entradas[i];
        while (p != NULL) {
            TNo *temp = p->prox;
            free(p);
            p = temp;
        }
        tabela.entradas[i] = NULL;
    }
}