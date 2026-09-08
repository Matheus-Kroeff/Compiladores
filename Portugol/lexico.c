#include <stdio.h>
#include <stdlib.h>

char *buffer;

typedef enum { 
    ERRO,

    //SIMBOLOS
    PONTO_VIRGULA,
    VIRGULA,
    DOIS_PONTOS,
    PONTO,
    SOMA,
    SUBTRACAO,
    MULTIPLICACAO,
    DIVISAO,
    MENOR,
    MAIOR,
    MENOR_IGUAL,
    MAIOR_IGUAL,
    IGUAL,
    ABRE_PARENTESES,
    FECHA_PARENTESES,
    ATRIBUICAO,
    DIFERENTE,

    //PALAVRAS RESERVADAS
    ALGORITMO,
    CARACTERE,
    DIV,
    E,
    ENQUANTO,
    ENTAO,
    ESCREVA,
    FACA,
    FALSO,
    FIM,
    FUNCAO,
    INICIO,
    INTEIRO,
    LEIA,
    LOGICO,
    MOD,
    OU,
    PROCEDIMENTO,
    SE,
    SENAO,
    VAR,
    VERDADEIRO,
    NAO,

    //ÁTOMOS ESPECIAIS
    CONSTINT,
    CONSTCHAR,
    COMENTARIO,
    FIM_ARQUIVO,
    IDENTIFICADOR,

} TAtomo;

typedef struct{
    TAtomo atomo;
    int linha;
    union{
        int numero; // atributo do átomo constint (constante inteira)
        char id[16]; // atributo identificador
        char ch; // atributo do átomo constchar (constante caractere)
    }atributo;
}TInfoAtomo;

TInfoAtomo obter_atomo();

FILE *arquivo;
int linha_atual = 1;

TInfoAtomo obter_atomo() {
    TInfoAtomo info;
    info.linha = linha_atual;
    
    char c = fgetc(arquivo);

    // Ignorar espaços, tabulações e quebras de linha
    while (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
        if (c == '\n') linha_atual++;
        c = fgetc(arquivo);
    }

    // Fim de arquivo
    if (c == EOF) {
        info.atomo = FIM_ARQUIVO;
        return info;
    }

    // Identificadores e Palavras Reservadas (começam com letra)
    if (isalpha(c)) {
        char buffer_id[100];
        int i = 0;
        
        while (isalnum(c) || c == '_') {
            if (i < 99) buffer_id[i++] = tolower(c); // Normaliza para minúsculo para facilitar
            c = fgetc(arquivo);
        }
        buffer_id[i] = '\0';
        ungetc(c, arquivo); // Devolve o último caractere que não pertence ao identificador

        info.atomo = confere_palavra_reservada(buffer_id);
        
        if (info.atomo == IDENTIFICADOR) {
            strncpy(info.atributo.id, buffer_id, 16);
            info.atributo.id[15] = '\0';
        }
        return info;
    }

    // Constantes Inteiras (começam com dígito)
    if (isdigit(c)) {
        int valor = 0;
        while (isdigit(c)) {
            valor = valor * 10 + (c - '0');
            c = fgetc(arquivo);
        }
        ungetc(c, arquivo);
        
        info.atomo = CONSTINT;
        info.atributo.numero = valor;
        return info;
    }

    // Constantes Caractere
    if (c == '\'') {
        char ch = fgetc(arquivo);
        char aspas = fgetc(arquivo); // Deve fechar com '\''
        if (aspas == '\'') {
            info.atomo = CONSTCHAR;
            info.atributo.ch = ch;
        } else {
            info.atomo = ERRO;
        }
        return info;
    }

    // Símbolos e Operadores de um ou dois caracteres 
    switch (c) {
        case ';': 
            info.atomo = PONTO_VIRGULA; 
            break;
        case ',': 
            info.atomo = VIRGULA; 
            break;
        case '.': 
            info.atomo = PONTO; 
            break;
        case '+': 
            info.atomo = SOMA; 
            break;
        case '-': 
            info.atomo = SUBTRACAO; 
            break;
        case '*': 
            info.atomo = MULTIPLICACAO; 
            break;
        case '/': 
            info.atomo = DIVISAO; 
            break;
        case '=': 
            info.atomo = IGUAL; 
            break;
        case '(': 
            info.atomo = ABRE_PARENTESES; 
            break;
        case ')': 
            info.atomo = FECHA_PARENTESES; 
            break;
        
        case ':': {
            char proximo = fgetc(arquivo);
            if (proximo == '=') {
                info.atomo = ATRIBUICAO;
            } else {
                ungetc(proximo, arquivo);
                info.atomo = DOIS_PONTOS;
            }
            break;
        }

        case '<': {
            char proximo = fgetc(arquivo);
            if (proximo == '=') {
                info.atomo = MENOR_IGUAL;
            } else if (proximo == '>') {
                info.atomo = DIFERENTE;
            } else {
                ungetc(proximo, arquivo);
                info.atomo = MENOR;
            }
            break;
        }

        case '>': {
            char proximo = fgetc(arquivo);
            if (proximo == '=') {
                info.atomo = MAIOR_IGUAL;
            } else {
                ungetc(proximo, arquivo);
                info.atomo = MAIOR;
            }
            break;
        }

        default:
            info.atomo = ERRO;
            break;
    }

    return info;
}
