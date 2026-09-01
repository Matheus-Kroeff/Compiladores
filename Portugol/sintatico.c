#include <stdio.h>
#include <stdlib.h>


void consome(TAtomo atomo);
TInfoAtomo lookahead;


void programa() {
    consome(ALGORITMO);
    consome(IDENTIFICADOR);
    consome(PONTO_VIRGULA);
    bloco();
    consome(PONTO);
}


void bloco() {
    declaracao_variaveis();
    declaracao_de_rotinas();
    comando_composto();
}

void declaracao_variaveis() {
    // Verifica se VAR existe
    if(lookahead.atomo == VAR) {
        consome(VAR);
        lista_variaveis();
        consome(PONTO_VIRGULA);

        while(lookahead.atomo == IDENTIFICADOR) {
            lista_variaveis();
            consome(PONTO_VIRGULA);
        }
    }
}

void lista_variaveis() {
    consome(IDENTIFICADOR);
    while(lookahead.atomo == VIRGULA) {
        consome(VIRGULA);
        consome(IDENTIFICADOR);
    }
    consome(DOIS_PONTOS);
    tipo();
}

void declaracao_de_rotinas() {

}

void declaracao_de_funcao() {

}

void declaracao_de_procedimento() {

}

void tipo() {

}

void parametros_formais() {

}

void parametro_formal() {

}

void comando_composto() {

}

void comando() {

}

void comando_atribuicao() {

}

void comando_entrada() {

}

void comando_saida() {
    
}

void comando_condicional() {

}

void comando_repeticao() {

}

void chamada_procedimento() {

}

void lista_expressao() {

}

void expressao() {

}

void operador_relacional() {

}

void expressao_simples() {

}

void operador_adicao() {

}

void termo() {

}

void operador_multiplicacao() {

}

void fator() {

}

