/*
- Matheus Leonardo Cardoso Kroeff - 10426434
- Naoto Ushizaki - 10437445
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "semantico.c"

// --- Variáveis Globais de Controle ---
int escopo_atual = 0;    // 0 = global, 1 = local
int deslocamento = 0;    // endereço relativo para AMEM / tabela de símbolos
int num_rotinas = 0;     // contador para rótulos de rotinas (DSVS)

// --- Protótipos das Funções Sintáticas ---
void consome(TAtomo atomo);
void programa();
void bloco();
int declaracao_de_variaveis();
void lista_variaveis();
void declaracao_de_rotinas();
void declaracao_de_funcao();
void declaracao_de_procedimento();
void tipo();
void parametros_formais();
void parametro_formal();
void comando_composto();
void comando();
void comando_atribuicao();
void comando_entrada();
void comando_saida();
void comando_condicional();
void comando_repeticao();
void chamada_procedimento();
void lista_expressao();
void expressao();
void operador_relacional();
void expressao_simples();
void operador_adicao();
void termo();
void operador_multiplicacao();
void fator();

int eh_operador_relacional(TAtomo atomo);
int eh_operador_adicao(TAtomo atomo);
int eh_operador_multiplicacao(TAtomo atomo);

TInfoAtomo lookahead;

void consome(TAtomo atomo){
    if(lookahead.atomo == atomo){
        lookahead = obter_atomo();
    } else {
        printf("Erro sintatico na linha %d -> Esperado: [%s] | Encontrado: [%s]\n", 
               lookahead.linha, nomes_atomos[atomo], nomes_atomos[lookahead.atomo]);
        libera_tabela();
        exit(1);
    }
}

// <programa> ::= algoritmo identificador ‘;’ <bloco> ‘.’
void programa() {
    printf("INPP\n"); // Instrução de inicio de programa

    consome(ALGORITMO);
    consome(IDENTIFICADOR);
    consome(PONTO_VIRGULA);
    bloco();
    consome(PONTO);

    printf("PARA\n"); //Instrução de fim de programa?
}

// <bloco> ::= <declaração_variáveis> <declaração_de_rotinas> <comando_composto>
void bloco() {
    int vars_globais = declaracao_de_variaveis();

    if (lookahead.atomo == FUNCAO || lookahead.atomo == PROCEDIMENTO) {
        printf("DSVS L1\n");
        declaracao_de_rotinas();
        printf("L1: NADA\n");
    }

    comando_composto();

    if (vars_globais > 0) {
        printf("DMEM %d\n", vars_globais);
    }
}

// <declaração_variáveis> ::= [ var <lista_variaveis> ‘;’ { <lista_variaveis> ‘;’ } ]
int declaracao_de_variaveis() {
    int inicio_deslocamento = deslocamento;

    if(lookahead.atomo == VAR) {
        consome(VAR);
        lista_variaveis(CAT_VARIAVEL);
        consome(PONTO_VIRGULA);

        while(lookahead.atomo == IDENTIFICADOR) {
            lista_variaveis(CAT_VARIAVEL);
            consome(PONTO_VIRGULA);
        }
        
        int alocados = deslocamento - inicio_deslocamento;
        if (alocados > 0) {
            printf("AMEM %d\n", alocados);
        }
        return alocados;
    }
    return 0;
}

// <lista_variaveis> ::= identificador { ‘,’ identificador } ‘:’ <tipo>
void lista_variaveis(TCategoria cat) {
    char temp_id[16];
    int linha_id = lookahead.linha;

    // Processa o primeiro identificador
    strncpy(temp_id, lookahead.atributo.id, 15);
    temp_id[15] = '\0';
    consome(IDENTIFICADOR);
    
    insere(temp_id, cat, escopo_atual, deslocamento++, linha_id);

    // Processa identificadores adicionais separados por vírgula
    while(lookahead.atomo == VIRGULA) {
        consome(VIRGULA);
        
        linha_id = lookahead.linha;
        strncpy(temp_id, lookahead.atributo.id, 15);
        temp_id[15] = '\0';
        consome(IDENTIFICADOR);
        
        insere(temp_id, cat, escopo_atual, deslocamento++, linha_id);
    }
    
    consome(DOIS_PONTOS);
    tipo();
}

// <declaracao_de_rotinas> ::= { <declaração_de_função>|<declaração_de_procedimento> }
void declaracao_de_rotinas() {
    while(lookahead.atomo == FUNCAO || lookahead.atomo == PROCEDIMENTO) {
        if(lookahead.atomo == FUNCAO) {
            declaracao_de_funcao();
        } else {
            declaracao_de_procedimento();
        }
    }
}

// <declaração_de_função> ::= funcao <tipo> identificador <parametros_formais> <declaracao_de_variaveis> <comando_composto>
void declaracao_de_funcao() {
    consome(FUNCAO);
    tipo();
    
    char id[16];
    int linha_id = lookahead.linha;
    strncpy(id, lookahead.atributo.id, 15);
    id[15] = '\0';
    
    insere(id, CAT_FUNCAO, escopo_atual, 0, linha_id);
    consome(IDENTIFICADOR);
    
    escopo_atual = 1;
    deslocamento = 0; // Zerado no início da rotina para escopo local
    
    parametros_formais();
    int vars_locais = declaracao_de_variaveis();
    comando_composto();
    
    if (vars_locais > 0) {
        printf("DMEM %d\n", vars_locais);
    }
    printf("RTPR\n");
    
    remove_locais();
    escopo_atual = 0;
}

// <declaracao_de_procedimento> ::= procedimento identificador <parametros_formais> <declaracao_de_variaveis> <comando_composto>
void declaracao_de_procedimento() {
    consome(PROCEDIMENTO);
    
    char id[16];
    int linha_id = lookahead.linha;
    strncpy(id, lookahead.atributo.id, 15);
    id[15] = '\0';
    
    insere(id, CAT_PROCEDIMENTO, escopo_atual, 0, linha_id);
    consome(IDENTIFICADOR);
    
    escopo_atual = 1;
    deslocamento = 0; // Zerado no início da rotina para escopo local
    
    parametros_formais();
    int vars_locais = declaracao_de_variaveis();
    comando_composto();
    
    if (vars_locais > 0) {
        printf("DMEM %d\n", vars_locais);
    }
    printf("RTPR\n");
    
    remove_locais();
    escopo_atual = 0;
}

// <tipo> ::= caractere | inteiro | logico
void tipo() {
    if (lookahead.atomo == CARACTERE) {
        consome(CARACTERE);
    } else if (lookahead.atomo == INTEIRO) {
        consome(INTEIRO);
    } else {
        consome(LOGICO);
    }
}

// <parâmetros_formais> ::= ‘(’ <parâmetro_formal> { ‘;’ parâmetro_formal } ‘)’ | ‘(’ ‘)’
void parametros_formais() {
    consome(ABRE_PARENTESES);
    if (lookahead.atomo != FECHA_PARENTESES) {
        parametro_formal();
        while (lookahead.atomo == PONTO_VIRGULA) {
            consome(PONTO_VIRGULA);
            parametro_formal();
        }
    }
    
    consome(FECHA_PARENTESES);
}

// <parâmetro_formal> ::= [var] <lista_variaveis>
void parametro_formal() {
    if(lookahead.atomo == VAR) {
        consome(VAR);
    }
    lista_variaveis(CAT_PARAMETRO);
}

// <comando_composto> ::= inicio <comando> { ‘;’ <comando> } fim
void comando_composto() {
    consome(INICIO);
    comando();
    while(lookahead.atomo == PONTO_VIRGULA) {
        consome(PONTO_VIRGULA);
        comando();
    }
    consome(FIM);
}

// <comando> ::= <comando_atribuição> | <comando_entrada> |<comando_saida> |<comando_condicional> |<comando_repeticao> |<chamada_procedimento> |<comando_composto>
void comando() {
    if (lookahead.atomo == INICIO) {
        comando_composto();
    } else if (lookahead.atomo == LEIA) {
        comando_entrada();
    } else if (lookahead.atomo == ESCREVA) {
        comando_saida();
    } else if (lookahead.atomo == SE) {
        comando_condicional();
    } else if (lookahead.atomo == ENQUANTO) {
        comando_repeticao();
    } else if (lookahead.atomo == IDENTIFICADOR) {
        consome(IDENTIFICADOR);
        if (lookahead.atomo == ATRIBUICAO) {
            consome(ATRIBUICAO);
            expressao();
        } else {
            if (lookahead.atomo == ABRE_PARENTESES) {
                consome(ABRE_PARENTESES);
                lista_expressao();
                consome(FECHA_PARENTESES);
            }
        }
    } else {
        consome(INICIO); // Forçar erro
    }
}

// <comando_atribuição> ::= identificador ‘:=’ <expressão>
void comando_atribuicao() {
    consome(IDENTIFICADOR);
    consome(ATRIBUICAO);
    expressao();
}

// <comando_entrada> ::= leia ‘(’ identificador { ‘,’ identificador } ‘)’
void comando_entrada() {
    consome(LEIA);
    consome(ABRE_PARENTESES);
    consome(IDENTIFICADOR);
    while(lookahead.atomo == VIRGULA) {
        consome(VIRGULA);
        consome(IDENTIFICADOR);
    }
    consome(FECHA_PARENTESES);
}

// <comando_saida> ::= escreva ‘(’ <lista_expressão> ‘)’
void comando_saida() {
    consome(ESCREVA);
    consome(ABRE_PARENTESES);
    lista_expressao();
    consome(FECHA_PARENTESES);
}

// <comando_condicional> ::= se <expressão> entao <comando> [ senao <comando> ]
void comando_condicional() {
    consome(SE);
    expressao();
    consome(ENTAO);
    comando();
    if(lookahead.atomo == SENAO) {
        consome(SENAO);
        comando();
    }
}

// <comando_repeticao> ::= enquanto <expressão> faca <comando>
void comando_repeticao() {
    consome(ENQUANTO);
    expressao();
    consome(FACA);
    comando();
}

// <chamada_procedimento> ::= identificador [ ‘(’ <lista_expressão> ‘)’ ]
void chamada_procedimento() {
    consome(IDENTIFICADOR);
    if(lookahead.atomo == ABRE_PARENTESES) {
        consome(ABRE_PARENTESES);
        lista_expressao();
        consome(FECHA_PARENTESES);
    }
}

// <lista_expressão> ::= <expressão> { ‘,’ <expressão> }
void lista_expressao() {
    expressao();
    while(lookahead.atomo == VIRGULA) {
        consome(VIRGULA);
        expressao();
    }
}

int eh_operador_relacional(TAtomo atomo) {
    if (atomo == DIFERENTE ||
        atomo == MENOR ||
        atomo == MENOR_IGUAL ||
        atomo == MAIOR_IGUAL ||
        atomo == MAIOR ||
        atomo == IGUAL) {
        return 1;
    } else {
        return 0;
    }
}

// <expressão> ::= <expressão_simples> [ <operador_relacional> <expressão_simples> ]
void expressao() {
    expressao_simples();
    if (eh_operador_relacional(lookahead.atomo)) {
        operador_relacional();
        expressao_simples();
    }
}


// <operador_relacional> ::= ‘<>’ | ‘<’ | ‘<=’ | ‘>=’ | ‘>’ | ‘=’
void operador_relacional() {
    switch (lookahead.atomo) {
        case DIFERENTE:
            consome(DIFERENTE);
            break;
        case MENOR:
            consome(MENOR);
            break;
        case MENOR_IGUAL:
            consome(MENOR_IGUAL);
            break;
        case MAIOR_IGUAL:
            consome(MAIOR_IGUAL);
            break;
        case MAIOR:
            consome(MAIOR);
            break;
        case IGUAL:
            consome(IGUAL);
            break;
        default:
            consome(IGUAL); // força erro sintático caso chegue aqui
            break;
    }
}

// <expressão_simples> ::= <termo> { <operador_adição> <termo> }
void expressao_simples() {

    termo();
    while (eh_operador_adicao(lookahead.atomo)) {
        operador_adicao();
        termo();
    }
}

int eh_operador_adicao(TAtomo atomo) {
    if( atomo == SOMA ||
        atomo == SUBTRACAO ||
        atomo == MOD ||
        atomo == OU) {
         return 1;
        }
    else {
        return 0;
    }

}


// <operador_adição> ::= ‘+’ | ‘-’ | mod | ou
void operador_adicao() {
    switch(lookahead.atomo) {
        case SOMA:
            consome(SOMA);
            break;
        case SUBTRACAO:
            consome(SUBTRACAO);
            break;
        case MOD:
            consome(MOD);
            break;
        case OU:
            consome(OU);
            break;
        default:
            consome(OU);
            break;
    }
}

// <termo> ::= <fator> { <operador_multiplicação> <fator> }
void termo() {
    fator();
    while (eh_operador_multiplicacao(lookahead.atomo)) {
        operador_multiplicacao();
        fator();
    }
}

int eh_operador_multiplicacao(TAtomo atomo) {
    if( atomo == MULTIPLICACAO ||
        atomo == DIVISAO ||
        atomo == E) {
            return 1;
        } else {
            return 0;
        }
}

// <operador_multiplicação> ::= ‘*’ | div | e
void operador_multiplicacao() {
    switch(lookahead.atomo) {
        case MULTIPLICACAO:
            consome(MULTIPLICACAO);
            break;
        case DIVISAO:
            consome(DIVISAO);
            break;
        case E:
            consome(E);
            break;
        default:
            consome(DIVISAO);
            break;
    }
}

// <fator> ::= identificador [ ‘(’ <lista_expressão> ‘)’ ] | constint | constchar | ‘(’ <expressão> ‘)’ | ( ‘+’ | ‘-’ | nao ) <fator> | verdadeiro | falso
void fator() {
    if(lookahead.atomo == IDENTIFICADOR) {
        consome(IDENTIFICADOR);
        if(lookahead.atomo == ABRE_PARENTESES) {
            consome(ABRE_PARENTESES);
            lista_expressao();
            consome(FECHA_PARENTESES);
        }
    } else if(lookahead.atomo == CONSTINT) {
        consome(CONSTINT);
    } else if(lookahead.atomo == CONSTCHAR) {
        consome(CONSTCHAR);
    } else if(lookahead.atomo == ABRE_PARENTESES) {
        consome(ABRE_PARENTESES);
        expressao();
        consome(FECHA_PARENTESES);
    } else if(lookahead.atomo == VERDADEIRO) {
        consome(VERDADEIRO);
    } else if(lookahead.atomo == FALSO) {
        consome(FALSO);
    } else {
        switch(lookahead.atomo) {
            case SOMA:
                consome(SOMA);
                break;
            case SUBTRACAO:
                consome(SUBTRACAO);
                break;
            case NAO:
                consome(NAO);
                break;
            default:
                consome(SOMA);
                break;
        }
        fator();
    }
}
