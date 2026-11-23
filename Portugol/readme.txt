COMPILADORES - TRABALHO 2: ANALISADOR SEMÂNTICO E GERAÇÃO DE CÓDIGO MEPA
 
Docente: Fabio Lubacheski
Integrantes:
  - Matheus Leonardo Cardoso Kroeff - 10426434
  - Naoto Ushizaki                  - 10437445
 
 
1. COMO COMPILAR E RODAR
 
Com o hash_mack.o na mesma pasta:
 
    gcc -Wall -Wno-unused-result -g -Og compilador.c hash_mack.o -o compilador
    ./compilador <arquivo de entrada>
 
O código MEPA sai na saída padrão e a tabela de símbolos na saída de
erro, então as duas aparecem na tela, mas é possível gravar só o MEPA
em um arquivo e executá-lo no interpretador da disciplina:
 
    ./compilador <arquivo de entrada> > saida.mepa
    ./mepa -r -f saida.mepa
 
 
2. ATÉ ONDE FOI CONCLUÍDO
 
O trabalho foi concluído, com exceção do operador "mod" (seção 3).
 
  - Tabela de símbolos com hashMack e listas encadeadas.
  - Análise semântica: identificador repetido no mesmo escopo,
    identificador não declarado, uso incompatível com a categoria
    (ex.: atribuir a um procedimento) e número errado de argumentos.
  - Geração de MEPA para variáveis, expressões, atribuição, leia,
    escreva, se/senao, enquanto, procedimentos e funções com
    parâmetros por valor e por referência (var), variáveis locais e
    recursão.
  - Na passagem por referência, o argumento deve ser uma variável ou
    um parâmetro (erro semântico caso contrário).
  - Mensagem de erro com a linha e o tipo (léxico, sintático ou
    semântico).
 
O código gerado foi testado no interpretador da disciplina (ex.:
fatorial e fibonacci recursivos e o proc_troca da Aula 8) e o compilador
foi verificado com o valgrind, sem erros de memória.
 
 
3. BUGS E LIMITAÇÕES CONHECIDAS
 
  - "mod" gera a instrução MODI, que não existe no interpretador.
  - Cada rotina guarda quais parâmetros são por referência em um vetor
    de 20 posições, então esse controle vale para rotinas com até 20
    parâmetros.
  - Uma expressão passada para um parâmetro por referência, como
    p(a + 1), aparece como erro sintático, e não semântico.
  - Texto depois do "." final do programa é ignorado.
  - Identificadores são truncados em 15 caracteres; dois nomes com os
    mesmos 15 primeiros caracteres são considerados iguais.
 
Comportamentos do interpretador que precisamos contornar:
 
  - Pela Aula 7, "RTPR n" deveria remover os n parâmetros da pilha, mas
    no interpretador ele não remove. Por isso geramos "DMEM n" logo
    após cada CHPR de rotina com parâmetros.
  - Um programa sem nenhum rótulo não é executado. Por isso sempre
    geramos um rótulo antes do corpo principal.
  - O interpretador aceita só "ENPR", "RTPR n" e "CHPR Lk" (sem nível).
 
 
4. DECISÕES DE DESIGN
 
  - Símbolos novos entram no início da lista, então uma variável local
    com o mesmo nome de uma global é encontrada primeiro.
  - Ao fim de cada rotina, os símbolos locais são removidos da tabela;
    por isso a tabela impressa no final mostra só os globais.
  - Nível 0 para globais e 1 para locais (a gramática não tem rotinas
    aninhadas). Parâmetros em -(n+3-i) e retorno da função em -(n+3).
    O retorno é feito com "f := ...", aceito só dentro da própria
    função.
  - Parâmetros por referência: o parâmetro "var" é marcado na
    tabela de símbolos. Na chamada, empilhamos o endereço do argumento
    com CREN, ou com CRVL quando o argumento já é um parâmetro por
    referência. Dentro da rotina, a leitura usa
    CRVI e a escrita (atribuição e leia) usa ARMI.
  - Um único contador gera todos os rótulos (L1, L2, ...).
  - Aceitamos chamadas com parênteses vazios, como p().
  - A struct do átomo é zerada no léxico, para que o sintático nunca
    leia memória não inicializada ao guardar um identificador.
 
