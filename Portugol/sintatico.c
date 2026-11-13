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
int deslocamento = 0;    // endereço relativo corrente das variáveis
int rotulo_atual = 1;    // contador único para todos os rótulos do programa (L1, L2, ...)
TNo *rotina_atual = NULL;

int proximo_rotulo() {
    return rotulo_atual++;
}

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
int lista_expressao();
void expressao();
void operador_relacional();
void expressao_simples();
void operador_adicao();
void termo();
void operador_multiplicacao();
void fator();
void erro_semantico(int linha, const char *msg, const char *id);

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

void erro_semantico(int linha, const char *msg, const char *id) {
    printf("Erro semantico na linha %d -> %s '%s'\n", linha, msg, id);
    libera_tabela();
    exit(1);
}

// <programa> ::= algoritmo identificador ‘;’ <bloco> ‘.’
void programa() {
    printf("INPP\n");
    
    consome(ALGORITMO);
    consome(IDENTIFICADOR);
    consome(PONTO_VIRGULA);
    
    bloco();
    
    consome(PONTO);
}

// <bloco> ::= <declaração_variáveis> <declaração_de_rotinas> <comando_composto>
void bloco() {
    int vars_globais = declaracao_de_variaveis();
    int rotulo_inicio = proximo_rotulo();
    
    if (lookahead.atomo == FUNCAO || lookahead.atomo == PROCEDIMENTO) {
        printf("DSVS L%d\n", rotulo_inicio);
        declaracao_de_rotinas();
    }
    // Sempre emite um rótulo antes do corpo principal para garantir execução no interpretador MEPA
    printf("L%d: NADA\n", rotulo_inicio);
    
    comando_composto();
    
    if (vars_globais > 0) {
        printf("DMEM %d\n", vars_globais);
    }
    printf("PARA\n");
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
    
    insere(temp_id, cat, escopo_atual, deslocamento++, 0, linha_id);

    // Processa identificadores adicionais separados por vírgula
    while(lookahead.atomo == VIRGULA) {
        consome(VIRGULA);
        
        linha_id = lookahead.linha;
        strncpy(temp_id, lookahead.atributo.id, 15);
        temp_id[15] = '\0';
        consome(IDENTIFICADOR);
        
        insere(temp_id, cat, escopo_atual, deslocamento++, 0, linha_id);
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
    
    int rotulo_rotina = proximo_rotulo();
    insere(id, CAT_FUNCAO, escopo_atual, 0, rotulo_rotina, linha_id);
    rotina_atual = busca(id); // Registra que estamos dentro desta função
    consome(IDENTIFICADOR);
    
    escopo_atual = 1;
    deslocamento = 0;
    
    parametros_formais();
    int num_params = deslocamento;
    
    atualiza_enderecos_rotina(id, num_params);
    
    printf("L%d: ENPR 1\n", rotulo_rotina);
    
    deslocamento = 0;
    int vars_locais = declaracao_de_variaveis();
    
    comando_composto();
    
    if (vars_locais > 0) {
        printf("DMEM %d\n", vars_locais);
    }
    printf("RTPR 1,%d\n", num_params);
    
    remove_locais();
    rotina_atual = NULL; // Limpa ao sair da função
    escopo_atual = 0;
}

// <declaracao_de_procedimento> ::= procedimento identificador <parametros_formais> <declaracao_de_variaveis> <comando_composto>
void declaracao_de_procedimento() {
    consome(PROCEDIMENTO);
    
    char id[16];
    int linha_id = lookahead.linha;
    strncpy(id, lookahead.atributo.id, 15);
    id[15] = '\0';
    
    int rotulo_rotina = proximo_rotulo();
    // Guarda o número do rótulo no campo endereço da rotina na tabela de símbolos
    insere(id, CAT_PROCEDIMENTO, escopo_atual, 0, rotulo_rotina, linha_id);
    consome(IDENTIFICADOR);
    
    escopo_atual = 1;
    deslocamento = 0; // Zerado no início da rotina para contar parâmetros
    
    parametros_formais();
    int num_params = deslocamento; // Quantidade de parâmetros formais declarados

    atualiza_enderecos_rotina(id, num_params);
    
    printf("L%d: ENPR 1\n", rotulo_rotina);
    
    deslocamento = 0; // Reinicia deslocamento para as variáveis locais começarem em 0
    int vars_locais = declaracao_de_variaveis();
    
    comando_composto();
    
    if (vars_locais > 0) {
        printf("DMEM %d\n", vars_locais);
    }
    printf("RTPR 1,%d\n", num_params);
    
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
        char id[16];
        int linha_id = lookahead.linha;
        strncpy(id, lookahead.atributo.id, 15);
        id[15] = '\0';

        consome(IDENTIFICADOR);

        TNo *no = busca(id);
        if (no == NULL) {
            erro_semantico(linha_id, "Identificador nao declarado", id);
        }

        if (lookahead.atomo == ATRIBUICAO) {
            int pode_atribuir = (no->categoria == CAT_VARIAVEL) ||
                                (no->categoria == CAT_PARAMETRO) ||
                                (no->categoria == CAT_FUNCAO && no == rotina_atual);

            if (!pode_atribuir) {
                erro_semantico(linha_id, "Identificador nao pode receber atribuicao neste escopo", id);
            }

            consome(ATRIBUICAO);
            expressao();

            int nivel = (no->categoria == CAT_FUNCAO) ? 1 : no->escopo;
            printf("ARMZ %d,%d\n", nivel, no->endereço);
        } else {
            // Chamada de procedimento (com ou sem parênteses)
            if (no->categoria != CAT_PROCEDIMENTO) {
                erro_semantico(linha_id, "Identificador nao eh um procedimento", id);
            }

            int args_passados = 0;
            if (lookahead.atomo == ABRE_PARENTESES) {
                consome(ABRE_PARENTESES);
                args_passados = lista_expressao();
                consome(FECHA_PARENTESES);
            }

            if (args_passados != no->num_params) {
                erro_semantico(linha_id, "Numero incorreto de argumentos na chamada de", id);
            }

            printf("CHPR L%d,%d\n", no->rotulo, escopo_atual);

            if (no->num_params > 0) {
                printf("DMEM %d\n", no->num_params);
            }
        }
    } else {
        consome(INICIO);
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

    char id[16];
    int linha_id = lookahead.linha;
    strncpy(id, lookahead.atributo.id, 15);
    id[15] = '\0';
    consome(IDENTIFICADOR);

    TNo *no = busca(id);
    if (no == NULL) {
        erro_semantico(linha_id, "Identificador nao declarado", id);
    }
    if (no->categoria != CAT_VARIAVEL && no->categoria != CAT_PARAMETRO) {
        erro_semantico(linha_id, "Identificador invalido para leitura", id);
    }
    printf("LEIT\n");
    printf("ARMZ %d,%d\n", no->escopo, no->endereço);

    while (lookahead.atomo == VIRGULA) {
        consome(VIRGULA);

        linha_id = lookahead.linha;
        strncpy(id, lookahead.atributo.id, 15);
        id[15] = '\0';
        consome(IDENTIFICADOR);

        no = busca(id);
        if (no == NULL) {
            erro_semantico(linha_id, "Identificador nao declarado", id);
        }
        if (no->categoria != CAT_VARIAVEL && no->categoria != CAT_PARAMETRO) {
            erro_semantico(linha_id, "Identificador invalido para leitura", id);
        }
        printf("LEIT\n");
        printf("ARMZ %d,%d\n", no->escopo, no->endereço);
    }

    consome(FECHA_PARENTESES);
}

// <comando_saida> ::= escreva ‘(’ <lista_expressão> ‘)’
void comando_saida() {
    consome(ESCREVA);
    consome(ABRE_PARENTESES);

    expressao();
    printf("IMPR\n");
    while (lookahead.atomo == VIRGULA) {
        consome(VIRGULA);
        expressao();
        printf("IMPR\n");
    }

    consome(FECHA_PARENTESES);
}

// <comando_condicional> ::= se <expressão> entao <comando> [ senao <comando> ]
void comando_condicional() {
    int rotulo_falso = proximo_rotulo();

    consome(SE);
    expressao();
    printf("DSVF L%d\n", rotulo_falso);

    consome(ENTAO);
    comando();

    if (lookahead.atomo == SENAO) {
        int rotulo_fim = proximo_rotulo();
        printf("DSVS L%d\n", rotulo_fim);
        printf("L%d: NADA\n", rotulo_falso);

        consome(SENAO);
        comando();

        printf("L%d: NADA\n", rotulo_fim);
    } else {
        printf("L%d: NADA\n", rotulo_falso);
    }
}

// <comando_repeticao> ::= enquanto <expressão> faca <comando>
void comando_repeticao() {
    int rotulo_inicio = proximo_rotulo();
    int rotulo_fim = proximo_rotulo();

    consome(ENQUANTO);
    printf("L%d: NADA\n", rotulo_inicio);
    expressao();
    printf("DSVF L%d\n", rotulo_fim);

    consome(FACA);
    comando();
    printf("DSVS L%d\n", rotulo_inicio);
    printf("L%d: NADA\n", rotulo_fim);
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
int lista_expressao() {
    int count = 1;
    expressao();
    while (lookahead.atomo == VIRGULA) {
        consome(VIRGULA);
        expressao();
        count++;
    }
    return count;
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
        TAtomo op = lookahead.atomo;
        operador_relacional();
        expressao_simples();

        switch (op) {
            case IGUAL:       
                printf("CMIG\n"); break;
            case DIFERENTE:   
                printf("CMDG\n"); break;
            case MENOR:       
                printf("CMME\n"); break;
            case MENOR_IGUAL: 
                printf("CMEG\n"); break;
            case MAIOR:       
                printf("CMMA\n"); break;
            case MAIOR_IGUAL: 
                printf("CMAG\n"); break;
            default: 
                break;
        }
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
        TAtomo op = lookahead.atomo;
        operador_adicao();
        termo();

        switch (op) {
            case SOMA:      
                printf("SOMA\n"); break;
            case SUBTRACAO: 
                printf("SUBT\n"); break;
            case OU:        
                printf("DISJ\n"); break;
            case MOD:       
                printf("MODI\n"); break;
            default: 
                break;
        }
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
        TAtomo op = lookahead.atomo;
        operador_multiplicacao();
        fator();

        switch (op) {
            case MULTIPLICACAO: 
                printf("MULT\n"); break;
            case DIVISAO:
            case DIV:
                printf("DIVI\n"); break;
            case E:             
                printf("CONJ\n"); break;
            default: 
                break;
        }
    }
}

int eh_operador_multiplicacao(TAtomo atomo) {
    return (atomo == MULTIPLICACAO || atomo == DIVISAO || atomo == DIV || atomo == E);
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
        case DIV:           
            consome(DIV); 
            break;
        case E:
            consome(E);
            break;
        default:
            consome(DIV);
            break;
    }
}

// <fator> ::= identificador [ ‘(’ <lista_expressão> ‘)’ ] | constint | constchar | ‘(’ <expressão> ‘)’ | ( ‘+’ | ‘-’ | nao ) <fator> | verdadeiro | falso
void fator() {
    if (lookahead.atomo == IDENTIFICADOR) {
        char id[16];
        int linha_id = lookahead.linha;
        strncpy(id, lookahead.atributo.id, 15);
        id[15] = '\0';
        
        consome(IDENTIFICADOR);

        TNo *no = busca(id);
        if (no == NULL) {
            erro_semantico(linha_id, "Identificador nao declarado", id);
        }

        if (lookahead.atomo == ABRE_PARENTESES) {
            // Chamada de função com argumentos
            if (no->categoria != CAT_FUNCAO) {
                erro_semantico(linha_id, "Identificador nao eh uma funcao", id);
            }

            printf("AMEM 1\n");
            consome(ABRE_PARENTESES);
            int args_passados = lista_expressao();
            consome(FECHA_PARENTESES);

            if (args_passados != no->num_params) {
                erro_semantico(linha_id, "Numero incorreto de argumentos na chamada de", id);
            }

            printf("CHPR L%d,%d\n", no->rotulo, escopo_atual);
        } else {
            if (no->categoria == CAT_VARIAVEL || no->categoria == CAT_PARAMETRO) {
                printf("CRVL %d,%d\n", no->escopo, no->endereço);
            } else if (no->categoria == CAT_FUNCAO) {
                // Chamada de função sem argumentos
                if (no->num_params != 0) {
                    erro_semantico(linha_id, "Numero incorreto de argumentos na chamada de", id);
                }

                printf("AMEM 1\n");
                printf("CHPR L%d,%d\n", no->rotulo, escopo_atual);
            } else {
                erro_semantico(linha_id, "Uso invalido de procedimento em expressao", id);
            }
        }
    } else if (lookahead.atomo == CONSTINT) {
        int valor = lookahead.atributo.numero;
        consome(CONSTINT);
        printf("CRCT %d\n", valor);
    } else if (lookahead.atomo == CONSTCHAR) {
        char ch = lookahead.atributo.ch;
        consome(CONSTCHAR);
        printf("CRCT %d\n", ch);
    } else if (lookahead.atomo == ABRE_PARENTESES) {
        consome(ABRE_PARENTESES);
        expressao();
        consome(FECHA_PARENTESES);
    } else if (lookahead.atomo == VERDADEIRO) {
        consome(VERDADEIRO);
        printf("CRCT 1\n");
    } else if (lookahead.atomo == FALSO) {
        consome(FALSO);
        printf("CRCT 0\n");
    } else {
        TAtomo op = lookahead.atomo;
        switch (op) {
            case SOMA:      consome(SOMA); break;
            case SUBTRACAO: consome(SUBTRACAO); break;
            case NAO:       consome(NAO); break;
            default:        consome(SOMA); break;
        }
        
        fator();

        if (op == SUBTRACAO) {
            printf("INVR\n");
        } else if (op == NAO) {
            printf("NEGA\n");
        }
    }
}
