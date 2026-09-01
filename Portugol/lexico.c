#include <stdio.h>
#include <stdlib.h>

char *buffer;

typedef enum { 
    ERRO,

    //SIMBULOS
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

