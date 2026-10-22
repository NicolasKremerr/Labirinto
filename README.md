# Labirinto Giratorio - M2

Algoritmos e Programacao II - Prof. Thiago Felski
Aluno: Nicolas Kremer

## Versao
O trabalho foi desenvolvido na versao Windows.

## Formato do arquivo de jogo salvo (jogo_salvo.csv)
O jogo e salvo quando o jogador sai com ESC e o arquivo e apagado quando ele vence a fase.

- Linha 1: numero do mapa, dimensao n, orientacao, linha do jogador, coluna do jogador, movimentos, rotacoes e o que esta embaixo do jogador, separados por virgula.
- Linha 2: titulo do mapa.
- Proximas n linhas: o cenario como esta no momento.
- Ultimas n linhas: onde ficam as portas (0, 6 ou 7).

Gravei a matriz das portas porque um bloco pode estar em cima de uma porta, e isso nao da para descobrir so olhando o cenario. Se o arquivo nao existir, estiver vazio ou com algum valor invalido, o jogo considera que nao ha jogo salvo.

## Funcoes da M1 antes e depois

| Funcao | Na M1 | Na M2 | Por que mudou |
|---|---|---|---|
| moveJogador | (na M1, a movimentacao era feita em controle e blocos) | moveJogador(Jogo &jogo, char tecla) | agora o estado todo vai dentro da struct |
| aplicaGravidade | aplicaGravidade(int mat[TAM][TAM], int tipoPorta[TAM][TAM], int lado) | aplicaGravidade(Jogo &jogo) | o mapa pode ter tamanho n, nao so 11 |
| rotacionarEsquerda | rotacionar_esquerda(int &px, int &py, int &lado, int sobre, int mat[TAM][TAM], int mat_aux[TAM][TAM], int tipoPorta[TAM][TAM], int tipoPorta_aux[TAM][TAM], bool &jogadorMorreu) | rotacionarEsquerda(Jogo &jogo) | gira o cenario e as portas juntos |
| design | design(int mat[TAM][TAM], int i, int j, int lado) | desenharCelula(int celula, int orientacao) | recebe so o que precisa |
| controle | controle(int &px, int &py, int &lado, int mat[TAM][TAM], int mat_aux[TAM][TAM], int tipoPorta[TAM][TAM], int tipoPorta_aux[TAM][TAM], int numeroMapa, int &movimentos, int &rotacoes, int &sobre) | jogar(Jogo &jogo) e processaTecla(Jogo &jogo, char tecla) | a mesma funcao trata a tecla no teclado e na solucao |
| carregaMapa | carregaMapa(int numero, int mat[TAM][TAM], int tipoPorta[TAM][TAM]) | carregarMapa(int numero, Jogo &jogo, string &motivo) | o mapa agora vem de arquivo e e validado |

## Defeitos da M1 que foram corrigidos
- Quando o jogador batia na parede em cima da alavanca ou de uma porta, o jogo esquecia o que tinha embaixo dele.
- A gravidade nao olhava as bordas e achava que o mapa era sempre 11 x 11.
- O estado da celula abaixo do jogador precisava ser preservado, inclusive quando havia uma porta ou alavanca.
- As portas precisavam continuar identificadas mesmo quando a celula estava temporariamente ocupada pelo jogador ou por um bloco.
- A gravidade precisava considerar corretamente uma porta aberta ou fechada como apoio ou passagem para o bloco.

## Decisoes de projeto
- **Struct por valor ou por referencia:** usei referencia nas funcoes que mudam o jogo, para nao copiar as matrizes toda hora. As funcoes que so desenham recebem const Jogo &. Em mapaEstavel passei por valor de proposito, porque a copia e o que eu quero comparar.
- **Posicao do jogador:** guardo a posicao e ela tambem aparece como 2 no cenario. Para nao ficar diferente, so atualizo as duas juntas, dentro de moveJogador e das rotacoes.
- **O que grava no jogo salvo:** o cenario, a matriz das portas e os contadores. So grava quando sai com ESC.
- **Jogar e reproduzir solucao:** os dois usam a funcao processaTecla, entao nenhuma regra esta escrita duas vezes.
- **Desempate do ranking:** menos movimentos primeiro e, se empatar, menos rotacoes. Se ainda empatar, fica na frente quem registrou antes.

## Desafios extras
Nenhum.

## Mapas
Estao na pasta mapa1 a mapa5, cada um com o .csv e o .sol. Todas as solucoes passam em Ver solucao > Verificar todos os mapas.
