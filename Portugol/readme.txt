==========================================================================
 COMPILADORES - TRABALHO 2: ANALISADOR SEMÂNTICO E GERAÇÃO DE CÓDIGO MEPA
==========================================================================

Docente: Fabio Lubacheski

Integrantes:
  - Matheus Leonardo Cardoso Kroeff - 10426434
  - Naoto Ushizaki                  - 10437445


--------------------------------------------------------------------------
1. COMO COMPILAR E EXECUTAR
--------------------------------------------------------------------------

Compilação (o hash_mack.o fornecido deve estar na mesma pasta):

    gcc -Wall -Wno-unused-result -g -Og compilador.c hash_mack.o -o compilador

Execução:

    ./compilador programa.txt

A saída mostra na tela o código MEPA gerado e a tabela de símbolos.
O código MEPA é impresso na saída padrão (stdout) e a tabela de símbolos
na saída de erro (stderr). Assim, as duas aparecem no monitor, mas é
possível gravar apenas o código MEPA em um arquivo e executá-lo direto
no interpretador da disciplina:

    ./compilador programa.txt > programa.mepa
    ./mepa -r -f programa.mepa

Em caso de erro, o compilador para na primeira ocorrência e informa a
linha e o tipo do erro (léxico, sintático ou semântico).


--------------------------------------------------------------------------
2. ATÉ ONDE O TRABALHO FOI CONCLUÍDO
--------------------------------------------------------------------------

O trabalho foi concluído integralmente, com exceção do operador "mod"
(ver seção 6). Foram implementados:

  - Análise léxica (reaproveitada da fase 1, sem impressão de átomos e
    comentários).
  - Análise sintática descendente recursiva conforme a gramática.
  - Tabela de símbolos com hash (função hashMack do hash_mack.o) e
    tratamento de colisões por lista encadeada.
  - Análise semântica:
      * identificador declarado mais de uma vez no mesmo escopo;
      * identificador usado sem ter sido declarado;
      * uso incompatível com a categoria (ex.: atribuir a um procedimento,
        usar procedimento dentro de expressão, ler um nome de função);
      * atribuição ao nome da função permitida somente dentro dela;
      * número de argumentos diferente do número de parâmetros.
  - Geração de código MEPA para: declaração de variáveis, expressões
    (aritméticas, relacionais e lógicas, com operadores unários),
    atribuição, leia, escreva, se/senao, enquanto, procedimentos e
    funções com parâmetros, variáveis locais e recursão.

O código gerado foi testado no interpretador MEPA da disciplina com,
entre outros: laços com condicionais aninhados, procedimento com
parâmetros e variáveis locais, fatorial recursivo, fibonacci com dupla
recursão, chamadas aninhadas (ex.: soma(dobro(3), dez) * 2) e o exemplo
"procedimento2" da Aula 7. O compilador também foi verificado com o
valgrind, sem erros de memória, inclusive nas execuções que terminam
com erro.


--------------------------------------------------------------------------
3. ORGANIZAÇÃO DOS ARQUIVOS
--------------------------------------------------------------------------

  compilador.c  - função main: abre o arquivo, inicia a análise, imprime
                  e libera a tabela de símbolos.
  lexico.c      - analisador léxico (obter_atomo).
  semantico.c   - tabela de símbolos: estruturas TNo e TTabelaSimbolos,
                  busca, inserção, ajuste de endereços de rotinas,
                  remoção dos símbolos locais, impressão e liberação.
  sintatico.c   - analisador sintático, com as verificações semânticas e
                  a geração de código MEPA feitas dentro das próprias
                  funções da gramática.
  hash_mack.o   - função hashMack fornecida pelo professor.


--------------------------------------------------------------------------
4. DECISÕES DE PROJETO
--------------------------------------------------------------------------

Tabela de símbolos
  - Cada entrada guarda: ID, endereço, rótulo, número de parâmetros,
    categoria (variável, parâmetro, função ou procedimento) e escopo
    (0 = global, 1 = local).
  - Novos símbolos são inseridos no início da lista encadeada. Com isso,
    quando uma variável local tem o mesmo nome de uma global, a busca
    encontra a local primeiro (sombreamento).
  - A verificação de "já declarado" considera o nome e o escopo, então
    um local pode ter o mesmo nome de um global.
  - Ao terminar uma função ou procedimento, os símbolos locais
    (parâmetros e variáveis locais) são removidos da tabela. Por isso,
    a tabela impressa no final contém apenas os símbolos globais
    (variáveis, funções e procedimentos).

Endereços e níveis
  - Variáveis globais usam nível 0 e variáveis locais nível 1, com
    endereços começando em 0 em cada escopo. Como a gramática não
    permite rotinas aninhadas, esses dois níveis são suficientes.
  - Parâmetros seguem a convenção da Aula 8: o i-ésimo de n parâmetros
    fica no endereço -(n+3-i), ou seja, de -(n+2) até -3. Os parâmetros
    recebem endereços provisórios durante a leitura e são corrigidos
    quando o número total de parâmetros é conhecido.
  - O valor de retorno de uma função fica no endereço -(n+3). O retorno
    é feito atribuindo ao nome da função (f := ...), que gera
    ARMZ 1,-(n+3). Uma variável "rotina_atual" garante que isso só é
    aceito dentro da própria função.

Rótulos
  - Um único contador gera todos os rótulos do programa (L1, L2, ...),
    evitando colisões entre o desvio sobre as rotinas, as entradas das
    rotinas, os "se" e os "enquanto".
  - O código das rotinas fica entre um DSVS e o rótulo do programa
    principal, para que a execução não entre nas rotinas sem chamada.
  - O rótulo de entrada de cada rotina é guardado na tabela de símbolos
    e usado no CHPR das chamadas.

Chamadas
  - Função: AMEM 1 (espaço do resultado), código dos argumentos,
    CHPR Lk e DMEM n (ver seção 5). O resultado fica no topo da pilha.
  - Procedimento: código dos argumentos, CHPR Lk e DMEM n.


--------------------------------------------------------------------------
5. COMPORTAMENTOS DO INTERPRETADOR MEPA E COMO FORAM TRATADOS
--------------------------------------------------------------------------

Durante os testes no interpretador da disciplina, identificamos:

  a) Formato das instruções de rotina: o interpretador aceita apenas
     "ENPR" sem argumento, "RTPR n" e "CHPR Lk".
     Formas como "ENPR 1", "RTPR 1,n" ou "CHPR Lk,0" geram erro de
     sintaxe, então o compilador usa o formato aceito.

  b) RTPR n não remove os parâmetros da pilha: pela Aula 7, a instrução
     RTPR n deveria fazer s <- s-(n+2), removendo os n parâmetros ao
     sair da rotina. No interpretador da disciplina isso não acontece:
     testando RTPR com valores diferentes de n, o resultado foi sempre
     o mesmo, ou seja, ele desempilha apenas o endereço de retorno e o
     D[1] (s <- s-2).
     Numa função, isso deixava os parâmetros acima do valor de retorno,
     e a expressão usava um parâmetro no lugar do resultado.
     Como o trabalho deve ser executado nesse interpretador, o
     compilador gera "DMEM n" logo após cada CHPR de uma rotina com
     n > 0 parâmetros, removendo os parâmetros pelo lado de quem chama.
     Se o interpretador for corrigido para seguir a Aula 7, basta
     retirar esse DMEM (ele aparece em dois pontos do sintatico.c: na
     chamada de procedimento, em comando(), e na chamada de função, em
     fator()).

  c) Programa sem nenhum rótulo não é executado: o interpretador termina
     sem executar nada e sem mensagem de erro. Por isso o compilador
     sempre gera um rótulo (Lk: NADA) antes do corpo principal, mesmo
     quando não há rotinas.

  d) Não há instrução de resto da divisão (ver seção 6).


--------------------------------------------------------------------------
6. LIMITAÇÕES CONHECIDAS
--------------------------------------------------------------------------

  - Operador "mod": é reconhecido pelo analisador e gera a instrução
    MODI, mas o interpretador da disciplina não possui instrução de
    resto da divisão, então programas que usam "mod" não executam
    corretamente.
  - Parâmetros declarados com "var" são aceitos sintaticamente e
    tratados como passagem por valor, já que o enunciado não exige
    passagem por referência.
  - Quando ocorre um erro, o código MEPA gerado até aquele ponto já foi
    impresso antes da mensagem de erro.
  - Identificadores são truncados em 15 caracteres (limite da struct
    do átomo e da tabela).
  - Se um parâmetro tiver exatamente o mesmo nome da função a que
    pertence, o endereço de retorno da função não é atualizado (a busca
    encontra o parâmetro primeiro).


--------------------------------------------------------------------------
7. EXTENSÕES EM RELAÇÃO À GRAMÁTICA
--------------------------------------------------------------------------

  - Chamada com parênteses vazios, p() ou f(), é aceita, como nos
    exemplos da Aula 7. A gramática exige pelo menos uma expressão
    entre parênteses; a forma sem parênteses (p ou f) continua aceita.
  - O símbolo "/" é aceito como divisão inteira, além de "div".


--------------------------------------------------------------------------
8. CONTATOS
--------------------------------------------------------------------------

  Matheus Leonardo Cardoso Kroeff
    E-mail:   10426434@mackenzista.com.br
    LinkedIn: https://www.linkedin.com/in/matheus-leonardo-cardoso-kroeff-23838535b/
    GitHub:   https://github.com/Matheus-Kroeff

  Naoto Ushizaki
    E-mail:   10437445@mackenzista.com.br
    LinkedIn: https://www.linkedin.com/in/naoto-ushizaki-30b94a172/
    GitHub:   https://github.com/mackcoder
