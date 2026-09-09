/*
- Matheus Leonardo Cardoso Kroeff - 10426434
- Naoto Ushizaki - 10437445
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

const char *nomes_atomos[] = {
    "ERRO",
    "PONTO_VIRGULA",
    "VIRGULA",
    "DOIS_PONTOS",
    "PONTO",
    "SOMA",
    "SUBTRACAO",
    "MULTIPLICACAO",
    "DIVISAO",
    "MENOR",
    "MAIOR",
    "MENOR_IGUAL",
    "MAIOR_IGUAL",
    "IGUAL",
    "ABRE_PARENTESES",
    "FECHA_PARENTESES",
    "ATRIBUICAO",
    "DIFERENTE",
    "ALGORITMO",
    "CARACTERE",
    "DIV",
    "E",
    "ENQUANTO",
    "ENTAO",
    "ESCREVA",
    "FACA",
    "FALSO",
    "FIM",
    "FUNCAO",
    "INICIO",
    "INTEIRO",
    "LEIA",
    "LOGICO",
    "MOD",
    "OU",
    "PROCEDIMENTO",
    "SE",
    "SENAO",
    "VAR",
    "VERDADEIRO",
    "NAO",
    "CONSTINT",
    "CONSTCHAR",
    "COMENTARIO",
    "FIM_ARQUIVO",
    "IDENTIFICADOR"
};

typedef struct{
    TAtomo atomo;
    int linha;
    union{
        int numero; // atributo do átomo constint (constante inteira)
        char id[16]; // atributo identificador
        char ch; // atributo do átomo constchar (constante caractere)
    }atributo;
}TInfoAtomo;

TAtomo confere_palavra_reservada(char *lexema) {
    if (strcmp(lexema, "algoritmo") == 0) return ALGORITMO;
    if(strcmp(lexema, "caractere") == 0) return CARACTERE;
    if(strcmp(lexema, "div") == 0) return DIV;
    if(strcmp(lexema, "e") == 0) return E;
    if(strcmp(lexema, "enquanto") == 0) return ENQUANTO;
    if(strcmp(lexema, "entao") == 0) return ENTAO;
    if(strcmp(lexema, "escreva") == 0) return ESCREVA;
    if(strcmp(lexema, "faca") == 0) return FACA;
    if(strcmp(lexema, "falso") == 0) return FALSO;
    if(strcmp(lexema, "fim") == 0) return FIM;
    if(strcmp(lexema, "funcao") == 0) return FUNCAO;
    if(strcmp(lexema, "inicio") == 0) return INICIO;
    if(strcmp(lexema, "inteiro") == 0) return INTEIRO;
    if(strcmp(lexema, "leia") == 0) return LEIA;
    if(strcmp(lexema, "logico") == 0) return LOGICO;
    if(strcmp(lexema, "mod") == 0) return MOD;
    if(strcmp(lexema, "ou") == 0) return OU;
    if(strcmp(lexema, "procedimento") == 0) return PROCEDIMENTO;
    if(strcmp(lexema, "se") == 0) return SE;
    if(strcmp(lexema, "senao") == 0) return SENAO;
    if(strcmp(lexema, "var") == 0) return VAR;
    if(strcmp(lexema, "verdadeiro") == 0) return VERDADEIRO;
    if(strcmp(lexema, "nao") == 0) return NAO;

    return IDENTIFICADOR;
    
}

TInfoAtomo obter_atomo();

FILE *arquivo;
int linha_atual = 1;


TInfoAtomo obter_atomo() {
    TInfoAtomo info;
    
    char c = fgetc(arquivo);

    // Ignorar espaços, tabulações e quebras de linha
    while (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
        if (c == '\n') linha_atual++;
        c = fgetc(arquivo);
    }

    // ignorar comentários
    if (c == '{') {
        char proximo = fgetc(arquivo);
        if (proximo == '-') {
            char atual = fgetc(arquivo);
            char anterior = '\0';
            while (!(anterior == '-' && atual == '}')) {
                if (atual == '\n') linha_atual++;
                anterior = atual;
                atual = fgetc(arquivo);
                if (atual == EOF) break;
            }
            return obter_atomo(); // Pega o próximo átomo após o comentário
        } else {
            ungetc(proximo, arquivo);
        }
    }

        info.linha = linha_atual;


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

    // Constantes Caractere (aceita aspas retas ' e curvas ‘ ’)
    if (c == '\'' || (unsigned char)c == 0xE2) { // 0xE2 é o primeiro byte das aspas curvas em UTF-8
        if ((unsigned char)c == 0xE2) {
            // Consome os outros dois bytes da aspa curva de abertura (ex: ‘)
            fgetc(arquivo); 
            fgetc(arquivo);
        }
        
        char ch = fgetc(arquivo);
        
        // Verifica se o próximo caractere fecha a aspa (reta ou curva)
        char prox1 = fgetc(arquivo);
        if (prox1 == '\'') {
            info.atomo = CONSTCHAR;
            info.atributo.ch = ch;
        } else if ((unsigned char)prox1 == 0xE2) {
            // Consome os outros dois bytes da aspa curva de fechamento (ex: ’)
            fgetc(arquivo);
            fgetc(arquivo);
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
