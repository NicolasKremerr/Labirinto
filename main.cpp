#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
using namespace std;

#define TAM 11

int getch(void) {
  int ch;
  struct termios oldt;
  struct termios newt;
  tcgetattr(STDIN_FILENO, &oldt); // guarda as configurações antigas
  newt = oldt; // copia as configurações antigas para as novas
  newt.c_lflag &= ~(ICANON | ECHO); // faz uma mudança nas novas configurações
  tcsetattr(STDIN_FILENO, TCSANOW, &newt); // aplica as novas configurações
  ch = getchar();                          // pega a tecla
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // restaura as configurações antigas
  return ch;                               // retorna o caractere lido
}

void SetCursorPos(int XPos, int YPos) {
  printf("\033[%d;%dH", YPos + 1, XPos + 1);
}

void pos_inicial(int &px, int &py, int mat[TAM][TAM]){
    for(int i=0; i<TAM; i++){
        for(int j=0; j<TAM; j++){
            if(mat[i][j]==2){
                px = i;
                py = j;
            }
        }
    }
}

// porta tipo 6 (A) e solida (fechada) em 0 e 180 graus; tipo 7 (B) e
// solida em 90 e 270. 
bool portaFechada(int tipo, int lado){
    if(tipo==6){
        return (lado==0 || lado==2);
    } 
    if(tipo==7){
        return (lado==1 || lado==3);
    }
    return false;
}

void lado_atual(int &lado, int valor){

    lado += valor;

    if(lado==4||lado==-4){
        lado=0;
    }

    switch(lado){
        case -1:
            lado=3;
            break;
        case -2:
            lado=2;
            break;
        case -3:
            lado=1;
            break;
    }
}

void design(int mat[TAM][TAM], int i, int j, int lado){

    switch (mat[i][j]){
        case 0:
            cout<<" "; // Caminho livre
            break;
        case 1:
            printf("\033[0;44m");// fundo do texto azul
            cout<<" ";
            printf("\033[0m");// todo texto normal
            break;
        case 2:
            cout<<"@"; // Player
            break;
        case 3:
            cout<<"O"; // Bloco com gravidade
            break;
        case 4:
            cout<<"/"; // Alavanca
            break;
        case 5:
            cout<<"S"; // Saida
            break;
        case 6:
            if(portaFechada(6, lado)){
                cout<<"=";  // Porta
            } 
            else{
                cout << ":"; // Porta
            }
            break;
        case 7: 
            if(portaFechada(7, lado)){
                cout<<"|"; // Porta
            }else{
                cout<<";"; // Porta
            }
            break;
        default:
            cout<<" ";
            break;
    }
}

void gerar_matriz(int mat[TAM][TAM], int lado){

    for(int i=0; i<TAM; i++){
        for(int j=0; j<TAM; j++){
            design(mat, i, j, lado);
        }
        cout<<endl;
    }
}

// aplica a gravidade sobre os blocos. Uma porta aberta nao
// sustenta nada, entao o bloco continua caindo por ela; uma porta fechada
// sustenta, entao o bloco para em cima dela.
void aplicaGravidade(int mat[TAM][TAM], int lado){
    bool mudou;
    do{
        mudou = false;
        for(int i=TAM-2; i>=1; i--){
            for(int j=1; j<TAM-1; j++){
                if(mat[i][j]==3){
                    int abaixo = mat[i+1][j];
                    bool podeCair = (abaixo==0) || ((abaixo==6||abaixo==7) && !portaFechada(abaixo,lado));
                    if(podeCair){
                        mat[i][j]=0;
                        mat[i+1][j]=3;
                        mudou = true;
                    }
                }
            }
        }
    } while(mudou);
}

// resolve o esmagamento causado por portas que se fecham depois de girar.
// tipoPorta guarda em que celulas existem portas e de que tipo, mesmo
// quando a celula esta temporariamente mostrando um bloco (3) ou o
// jogador (2)
void resolveEsmagamento(int mat[TAM][TAM], int tipoPorta[TAM][TAM], int lado, int px, int py, int sobre, bool &jogadorMorreu){

    jogadorMorreu = false;

    for(int i=0; i<TAM; i++){
        for(int j=0; j<TAM; j++){
            if(tipoPorta[i][j] != 0 && portaFechada(tipoPorta[i][j], lado)){
                if(mat[i][j] == 3){
                    mat[i][j] = tipoPorta[i][j];
                }
            }
        }
    }
    if(sobre == 6 || sobre == 7){
        if(tipoPorta[px][py] == sobre){
            if(portaFechada(sobre, lado)){
                jogadorMorreu = true;
            }
        }
    }
}


void rotacionar_cima(int mat[TAM][TAM], int lado){
    gerar_matriz(mat, lado);
}

void rotacionar_esquerda(int &px, int &py, int &lado, int sobre, int mat[TAM][TAM], int mat_aux[TAM][TAM], int tipoPorta[TAM][TAM], int tipoPorta_aux[TAM][TAM], bool &jogadorMorreu){

    for(int i=0; i<TAM; i++){
        for(int k=0, j=TAM-1; j>=0; j--, k++){
            mat_aux[k][i] = mat[i][j];
            tipoPorta_aux[k][i] = tipoPorta[i][j];
        }
    }

    for(int i=0; i<TAM; i++){
        for(int j=0; j<TAM; j++){
            mat[i][j] = mat_aux[i][j];
            tipoPorta[i][j] = tipoPorta_aux[i][j];
            if(mat[i][j]==2){
                px = i;
                py = j;
            }
        }
    }
    lado_atual(lado, -1);
    resolveEsmagamento(mat, tipoPorta, lado, px, py, sobre, jogadorMorreu);
    
    if(!jogadorMorreu){
        aplicaGravidade(mat, lado);
    }
}

void rotacionar_direita(int &px, int &py, int &lado, int sobre, int mat[TAM][TAM], int mat_aux[TAM][TAM], int tipoPorta[TAM][TAM], int tipoPorta_aux[TAM][TAM], bool &jogadorMorreu){

    for(int k=TAM-1, i=0; i<TAM; i++, k--){
        for(int j=0; j<TAM; j++){
            mat_aux[j][k] = mat[i][j];
            tipoPorta_aux[j][k] = tipoPorta[i][j];
        }
    }

    for(int i=0; i<TAM; i++){
        for(int j=0; j<TAM; j++){
            mat[i][j] = mat_aux[i][j];
            tipoPorta[i][j] = tipoPorta_aux[i][j];
            if(mat[i][j] == 2){
                px = i;
                py = j;
            }
        }
    }

    lado_atual(lado, 1);
    resolveEsmagamento(mat, tipoPorta, lado, px, py, sobre, jogadorMorreu);
    if(!jogadorMorreu){
        aplicaGravidade(mat, lado);
    }
}

void blocos(int tecla, int &px, int &py, int &sobre, int &lado, int mat[TAM][TAM], int mat_aux[TAM][TAM], int tipoPorta[TAM][TAM], int tipoPorta_aux[TAM][TAM], bool &jogadorMorreu){

    switch(mat[px][py]){

        case 0:
            sobre = 0;
            mat[px][py] = 2;
            break;

        case 1:
            sobre = 0;
            switch(tecla){
                case 'w':
                    px++;
                    break;
                case 's':
                    px--;
                    break;
                case 'a':
                    py++;
                    break;
                case 'd':
                    py--;
                    break;
            }
            mat[px][py] = 2;
            break;

        case 3:
            sobre = 0;
            switch(tecla){
                case 'w':
                    px++;
                    break;
                case 's':
                    px--;
                    break;
                case 'a':
                    py++;
                    break;
                case 'd':
                    py--;
                    break;
            }
            mat[px][py] = 2;
            break;

        case 4:
            sobre = 4;
            mat[px][py] = 2;

            switch(tecla){
                case 'q':
                    rotacionar_esquerda(px, py, lado, sobre, mat, mat_aux, tipoPorta, tipoPorta_aux, jogadorMorreu);
                    break;
                case 'e':
                    rotacionar_direita(px, py, lado, sobre, mat, mat_aux, tipoPorta, tipoPorta_aux, jogadorMorreu);
                    break;
            }
            break;

        case 5:
            sobre = 5;
            mat[px][py] = 2;
            break;

        case 6:
            if(portaFechada(6, lado)){
                switch(tecla){
                    case 'w': 
                        px++;
                        break;
                    case 's':
                        px--;
                        break;
                    case 'a':
                        py++;
                        break;
                    case 'd':
                        py--;
                        break;
                }
                mat[px][py] = 2;
            } else {
                sobre = 6;
                mat[px][py] = 2;
            }
            break;

        case 7:
            if(portaFechada(7, lado)){
                switch(tecla){
                    case 'w':
                        px++;
                        break;
                    case 's':
                        px--;
                        break;
                    case 'a':
                        py++;
                        break;
                    case 'd':
                        py--; 
                        break;
                }
                mat[px][py] = 2;
            } else {
                sobre = 7;
                mat[px][py] = 2;
            }
            break;
    }
}

// devolve 1 se o jogador venceu, 0 se saiu pelo ESC
int controle(int &px, int &py, int &lado,int mat[TAM][TAM], int mat_aux[TAM][TAM], int tipoPorta[TAM][TAM], int tipoPorta_aux[TAM][TAM], int numeroMapa, int &movimentos, int &rotacoes, int &sobre){

    char x;
    bool jogadorMorreu;
    cout << "\033[2J\033[H";

    while (true){
        printf("\033[?25l"); // codigo para o desativar o cursor no console
        cout << "Mapa: " << numeroMapa << "   Orientacao: " << (lado*90) << " graus"
             << "   Movimentos: " << movimentos << "   Rotacoes: " << rotacoes << endl << endl;
        rotacionar_cima(mat, lado);
        cout << endl << "WASD move | Q/E gira (sobre a alavanca) | R reinicia | ESC volta ao menu" << endl;

        x = getch(); // pega a tecla

        if(x == 27){ // ESC
            return 0;
        }

        if(x == 'r' || x == 'R'){
            return 2; // sinal para o main recarregar o mesmo mapa
        }

        switch (x){

            case 'w':
                mat[px][py] = sobre;
                sobre = 0; 
                px--; // Adiciona na nova posição
                blocos(x, px, py, sobre, lado, mat, mat_aux, tipoPorta, tipoPorta_aux, jogadorMorreu);
                movimentos++;
                break;

            case 's':
                mat[px][py] = sobre;
                sobre = 0;
                px++; // adiciona na nova posição
                blocos(x, px, py, sobre, lado, mat, mat_aux, tipoPorta, tipoPorta_aux, jogadorMorreu);
                movimentos++;
                break;

            case 'a':
                mat[px][py] = sobre;
                sobre = 0;
                py--; // adiciona na nova posição
                blocos(x, px, py, sobre, lado, mat, mat_aux, tipoPorta, tipoPorta_aux, jogadorMorreu);
                movimentos++;
                break;

            case 'd':
                mat[px][py] = sobre;
                sobre = 0;
                py++; // adiciona na nova posição
                blocos(x, px, py, sobre, lado, mat, mat_aux, tipoPorta, tipoPorta_aux, jogadorMorreu);
                movimentos++;
                break;

            case 'q':
            case 'e':
                if(sobre==4){
                    mat[px][py]=4;
                }
                blocos(x, px, py, sobre, lado, mat, mat_aux, tipoPorta, tipoPorta_aux, jogadorMorreu);
                if(x=='q' || x=='e') rotacoes++;
                if(jogadorMorreu){
                    cout << "\033[2J\033[H";
                    rotacionar_cima(mat, lado);
                    cout << endl << "Voce foi esmagado por uma porta! Pressione uma tecla..." << endl;
                    getch();
                    return 2;
                }
                break;

            default: break;
        }

        if(sobre == 5){
            cout << "\033[2J\033[H";
            rotacionar_cima(mat, lado);
            cout << endl << "Parabens! Voce escapou do labirinto!" << endl;
            cout << "Movimentos: " << movimentos << "   Rotacoes: " << rotacoes << endl;
            cout << "Pressione uma tecla..." << endl;
            getch();
            return 1;
        }

        SetCursorPos(0, 0);// coloca o cursor na posição inicial
    }
}


void zeraMapa(int mat[TAM][TAM], int tipoPorta[TAM][TAM]){
    for(int i=0; i<TAM; i++){
        for(int j=0; j<TAM; j++){
            mat[i][j]=0; tipoPorta[i][j]=0;
        }
    }
}

void colocaBordas(int mat[TAM][TAM]){
    for(int i=0; i<TAM; i++){ 
        mat[i][0]=1; mat[i][TAM-1]=1;
    }
        
    for(int j=0; j<TAM; j++){ mat[0][j]=1;
        mat[TAM-1][j]=1;
    }
}

void identificaPortas(int mat[TAM][TAM], int tipoPorta[TAM][TAM]){
    for(int i=0; i<TAM; i++){
        for(int j=0; j<TAM; j++){
            if(mat[i][j] == 6 || mat[i][j] == 7){
                tipoPorta[i][j] = mat[i][j];
            }else{
                tipoPorta[i][j] = 0;
            }
        }
    }
}


// mapa 1: a solucao depende da queda de um bloco para abrir passagem.

void carregaMapa1(int mat[TAM][TAM], int tipoPorta[TAM][TAM]){

// Resolucao: ddddssaqeaaassddddssdeeaaawwddddddddd

    int mapa[TAM][TAM] = {
        {1,1,1,1,1,1,1,1,1,1,1},
        {1,2,0,0,0,0,1,0,0,0,1},
        {1,1,1,1,1,0,1,0,1,0,1},
        {1,0,3,0,4,0,1,0,1,0,1},
        {1,0,1,1,1,1,1,0,1,0,1},
        {1,0,0,0,0,0,0,0,1,0,1},
        {1,0,1,1,1,0,1,1,1,0,1},
        {1,0,0,0,1,0,4,0,0,0,1},
        {1,1,1,6,1,1,1,1,1,0,1},
        {5,0,0,0,0,0,7,0,0,3,1},
        {1,1,1,1,1,1,1,1,1,1,1}
    };

    // Limpa as matrizes
    for(int i=0; i<TAM; i++){
        for(int j=0; j<TAM; j++){
            mat[i][j] = mapa[i][j];
            tipoPorta[i][j] = 0;
        }
    }
    
    identificaPortas(mat, tipoPorta);
}


void carregaMapa2(int mat[TAM][TAM], int tipoPorta[TAM][TAM]){

// Resolucao: dddssqeeqewwwaassssaawedddwwd

    int mapa[TAM][TAM] = {
        {1,1,1,1,1,1,1,1,1,1,1},
        {1,2,0,0,0,1,0,0,0,0,1},
        {1,1,1,1,0,1,1,1,1,0,1},
        {1,0,3,0,4,0,1,0,0,0,1},
        {1,0,1,1,1,1,1,0,1,1,1},
        {1,0,0,0,0,0,1,0,0,0,1},
        {1,1,1,1,1,0,1,1,1,0,1},
        {1,0,0,0,4,0,0,0,1,0,1},
        {1,7,1,1,1,1,1,0,1,0,1},
        {5,0,0,0,0,0,6,0,0,7,1},
        {1,1,1,1,1,1,1,1,1,1,1}
    };

    // Copia o mapa para a matriz principal
    for(int i=0; i<TAM; i++){
        for(int j=0; j<TAM; j++){
            mat[i][j] = mapa[i][j];
            tipoPorta[i][j] = 0;
        }
    }
    
    identificaPortas(mat, tipoPorta);

}


void carregaMapa3(int mat[TAM][TAM], int tipoPorta[TAM][TAM]){

//resolucao: ssaassaassaaqwwwwwwdd

    int mapa[TAM][TAM] = {
        {1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,1,0,0,2,1},
        {1,1,1,1,1,0,1,1,1,0,1},
        {1,0,3,0,4,0,1,0,0,0,1},
        {1,0,1,1,1,1,1,0,1,1,1},
        {1,0,0,0,0,0,0,0,1,0,1},
        {1,1,1,1,1,0,1,1,1,0,1},
        {1,0,0,4,0,0,6,0,0,0,1},
        {1,0,1,1,1,1,1,1,1,0,1},
        {1,0,0,0,7,0,0,3,0,5,1},
        {1,1,1,1,1,1,1,1,1,1,1}
    };

    // Copia o mapa para as matrizes
    for(int i=0; i<TAM; i++){
        for(int j=0; j<TAM; j++){
            mat[i][j] = mapa[i][j];
            tipoPorta[i][j] = 0;
        }
    }
    
    identificaPortas(mat, tipoPorta);

}


void carregaMapa(int numero, int mat[TAM][TAM], int tipoPorta[TAM][TAM]){

    switch(numero){

        case 1:
            carregaMapa1(mat, tipoPorta);
            break;

        case 2:
            carregaMapa2(mat, tipoPorta);
            break;

        case 3:
            carregaMapa3(mat, tipoPorta);
            break;

        default:
            carregaMapa1(mat, tipoPorta);
            break;
    }
}



void telaSobre(){

    cout << "\033[2J\033[H";

    cout << "\033[1;34m";
    cout << "==================================================" << endl;
    cout << "              LABIRINTO GIRATORIO                 " << endl;
    cout << "==================================================" << endl;
    cout << "\033[0m";

    cout << endl;

    cout << "\033[1;33m";
    cout << "                 SOBRE O JOGO                   " << endl;
    cout << "\033[0m";

    cout << "--------------------------------------------------" << endl;
    cout << "  Aluno:       Nicolas Kremer" << endl;
    cout << "  Mes/Ano:      Setembro/2026" << endl;
    cout << "  Disciplina:   Algoritimos e Programacao II" << endl;
    cout << "  Professor:    Thiago Felski" << endl;

    cout << endl;

    cout << "\033[1;33m";
    cout << "                   OBJETIVO                     " << endl;
    cout << "\033[0m";

    cout << "--------------------------------------------------" << endl;
    cout << "  Encontre a saida do labirinto utilizando" << endl;
    cout << "  movimentos, gravidade e rotacoes do cenario." << endl;

    cout << endl;

    cout << "\033[1;33m";
    cout << "               MECANICA DO JOGO                " << endl;
    cout << "\033[0m";

    cout << "--------------------------------------------------" << endl;
    cout << "  * O cenario pode ser girado em 90 graus." << endl;
    cout << "  * A gravidade sempre aponta para baixo." << endl;
    cout << "  * Ao girar, apenas o cenario muda de posicao." << endl;
    cout << "  * Blocos soltos (O) caem ate encontrar apoio." << endl;
    cout << "  * As portas possuem diferentes orientacoes." << endl;
    cout << "  * Uma porta fechada pode esmagar o jogador." << endl;

    cout << endl;

    cout << "\033[1;33m";
    cout << "                  CONTROLES                   " << endl;
    cout << "\033[0m";

    cout << "--------------------------------------------------" << endl;
    cout << "  W    Mover para cima" << endl;
    cout << "  S    Mover para baixo" << endl;
    cout << "  A    Mover para esquerda" << endl;
    cout << "  D    Mover para direita" << endl;
    cout << "  Q    Girar para esquerda" << endl;
    cout << "  E    Girar para direita" << endl;
    cout << "  R    Reiniciar o mapa" << endl;
    cout << "  ESC  Voltar ao menu" << endl;

    cout << endl;

    cout << "\033[1;34m";
    cout << "==================================================" << endl;
    cout << "       Pressione qualquer tecla para voltar       " << endl;
    cout << "==================================================" << endl;
    cout << "\033[0m";

    cin.ignore(1000, '\n');
    getch();
}


int menuPrincipal(bool jogoEmAndamento){

    cout << "\033[2J\033[H";

    cout << "\033[1;34m";
    cout << "==================================================" << endl;
    cout << "              LABIRINTO GIRATORIO                 " << endl;
    cout << "==================================================" << endl;
    cout << "\033[0m";
    cout<<endl;
    cout << "   1  -  Novo jogo" << endl;
    cout << "   2  -  Sobre" << endl;
    cout << "   3  -  Fim" << endl;
    
    if(jogoEmAndamento){
        cout << "   4  -  Continuar" << endl;
    }
    
    cout << endl;
    cout << "Escolha uma opcao: ";

    int opcao;
    cin >> opcao;
    
    if(opcao == 4 && !jogoEmAndamento){
        return 0;
    }
    return opcao;
}


int escolheMapa(){

    cout << "\033[2J\033[H";

    cout << "\033[1;34m";
    cout << "==================================================" << endl;
    cout << "                    NOVO JOGO                     " << endl;
    cout << "==================================================" << endl;
    cout << "\033[0m";

    cout << endl;

    cout << "   1  -  Escolher o mapa" << endl;
    cout << "   2  -  Mapa aleatorio" << endl;

    cout << endl;
    cout << "Escolha uma opcao: ";

    int opcao;

    cin >> opcao;

    if(opcao == 2){
        return (rand() % 3) + 1;
    }

    int numero;

    cout << endl;
    cout << "Numero do mapa (1 a 3): ";

    cin >> numero;

    if(numero < 1 || numero > 3){
        numero = 1;
    }

    return numero;
}

int main(){

    int sobre=0, lado=0, px, py, movimentos=0, rotacoes=0, numeroMapa=1;
    int mat_aux[TAM][TAM], tipoPorta[TAM][TAM], tipoPorta_aux[TAM][TAM];
    int mat[TAM][TAM];
    bool jogoEmAndamento = false;
    int opcao, resultado;

    srand((unsigned)time(NULL));

    do{
        opcao = menuPrincipal(jogoEmAndamento);
        switch(opcao){
            case 1:
                numeroMapa = escolheMapa();
                carregaMapa(numeroMapa, mat, tipoPorta);
                pos_inicial(px, py, mat);
                lado = 0; movimentos = 0; rotacoes = 0;
                jogoEmAndamento = true;
                resultado = controle(px, py, lado, mat, mat_aux, tipoPorta, tipoPorta_aux, numeroMapa, movimentos, rotacoes, sobre);
                while(resultado == 2){
                    carregaMapa(numeroMapa, mat, tipoPorta);
                    pos_inicial(px, py, mat);
                    lado = 0; movimentos = 0; rotacoes = 0;
                    resultado = controle(px, py, lado, mat, mat_aux, tipoPorta, tipoPorta_aux, numeroMapa, movimentos, rotacoes, sobre);
                }
                if(resultado == 1) jogoEmAndamento = false;
                break;
            case 4:
                    if(jogoEmAndamento){
                        resultado = controle(px, py, lado, mat, mat_aux, tipoPorta, tipoPorta_aux, numeroMapa, movimentos, rotacoes, sobre);
                        if(resultado == 1){
                            jogoEmAndamento = false;
                        }
                    }
                break;
            case 2:
                telaSobre();
                break;
            default:
                break;
        }
    } while(opcao != 3);

    cout << "\033[2J\033[H";
    return 0;
}