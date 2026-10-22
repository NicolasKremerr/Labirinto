#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <ctime>

using namespace std;

const int TAM = 21; // dimensao maxima do cenario

struct Posicao {
    int lin, col;
};

struct Jogo {
    
    int cenario[TAM][TAM];
    int tipoPorta[TAM][TAM];
    
    int n;
    int sobre;
    int rotacoes;
    int orientacao;
    int movimentos;
    int numeroMapa;
    
    bool jogadorMorreu;
    
    Posicao jogador;
    string titulo;
};

struct Recorde {
    string nome;
    int mapa, movimentos, rotacoes;
};

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

template <typename T> void trocar(T &a, T &b){
    
    T aux = a;
    a = b;
    b = aux;
}

// Funcoes de apoio

bool inteiro(string texto, int &valor){

    char resto;
    stringstream crt(texto);

    if(!(crt>>valor)){return false;}
    if(crt>>resto){return false;}
    return true;
}

bool temVirgula(string texto){
    
    int tamanho = texto.size();
    
    for(int i=0; i<tamanho; i++){
        if(texto[i]==','){return true;}
    }
    return false;
}

void tiraRetorno(string &texto){
    
    int tamanho = texto.size();
    
    if(tamanho>0 && texto[tamanho-1]=='\r'){
        texto = texto.substr(0, tamanho-1);
    }
}

string nomeArquivo(int numero, string extensao){
    
    stringstream nome;
    nome<<"mapa"<<numero<<extensao;
    return nome.str();
}

int lerOpcao(){
    
    int opcao;
    
    if(!(cin>>opcao)){
        cin.clear();
        opcao = 0;
    }
    cin.ignore(1000, '\n');
    return opcao;
}

// Regras do jogo

bool portaEstaFechada(int celula, int orientacao){
    
    if(celula==6){return (orientacao==0 || orientacao==2);} 
    if(celula==7){return (orientacao==1 || orientacao==3);}
    return false;
}

void ladoAtual(int &orientacao, int valor){
    
    orientacao += valor;
    if(orientacao < 0) {orientacao += 4;}
    if(orientacao >= 4){orientacao -= 4;}
}

void posInicial(Jogo &jogo){
    
    for(int i=0; i<jogo.n; i++){
        for(int j=0; j<jogo.n; j++){
            if(jogo.cenario[i][j]==2){
                jogo.jogador.lin = i;
                jogo.jogador.col = j;
            }
        }
    }
}

void identificarPortas(Jogo &jogo){
    
    for(int i=0; i<jogo.n; i++){
        for(int j=0; j<jogo.n; j++){
            if(jogo.cenario[i][j] == 6 || jogo.cenario[i][j] == 7){
                jogo.tipoPorta[i][j] = jogo.cenario[i][j];
            }else{
                jogo.tipoPorta[i][j] = 0;
            }
        }
    }
}

void aplicaGravidade(Jogo &jogo){
    
    bool mudou;
    do{
        mudou = false;
        for(int i=jogo.n-2; i>=0; i--){
            for(int j=0; j<jogo.n; j++){
                if(jogo.cenario[i][j]==3){
                    int abaixo = jogo.cenario[i+1][j];
                    bool podeCair = (abaixo==0) || ((abaixo==6 || abaixo==7) && !portaEstaFechada(abaixo, jogo.orientacao));
                    if(podeCair){
                        if(jogo.tipoPorta[i][j] != 0){
                            jogo.cenario[i][j] = jogo.tipoPorta[i][j];
                        }else{
                            jogo.cenario[i][j] = 0;
                        }
                        jogo.cenario[i+1][j] = 3;
                        mudou = true;
                    }
                }
            }
        }
    } while(mudou);
}

void resolveEsmagamento(Jogo &jogo){

    jogo.jogadorMorreu = false;

    for(int i=0; i<jogo.n; i++){
        for(int j=0; j<jogo.n; j++){
            if(jogo.tipoPorta[i][j] != 0 && portaEstaFechada(jogo.tipoPorta[i][j], jogo.orientacao)){
                if(jogo.cenario[i][j] == 3){
                    jogo.cenario[i][j] = jogo.tipoPorta[i][j];
                }
            }
        }
    }
    if(jogo.sobre == 6 || jogo.sobre == 7){
        if(jogo.tipoPorta[jogo.jogador.lin][jogo.jogador.col] == jogo.sobre){
            if(portaEstaFechada(jogo.sobre, jogo.orientacao)){
                jogo.jogadorMorreu = true;
            }
        }
    }
}

void rotacionarEsquerda(Jogo &jogo){
    
    int aux[TAM][TAM];
    int auxPorta[TAM][TAM];

    for(int i=0; i<jogo.n; i++){
        for(int k=0, j=jogo.n-1; j>=0; j--, k++){
            aux[k][i] = jogo.cenario[i][j];
            auxPorta[k][i] = jogo.tipoPorta[i][j];
        }
    }

    for(int i=0; i<jogo.n; i++){
        for(int j=0; j<jogo.n; j++){
            jogo.cenario[i][j] = aux[i][j];
            jogo.tipoPorta[i][j] = auxPorta[i][j];
            if(jogo.cenario[i][j]==2){
                jogo.jogador.lin = i;
                jogo.jogador.col = j;
            }
        }
    }
    ladoAtual(jogo.orientacao, -1);
    resolveEsmagamento(jogo);
    
    if(!jogo.jogadorMorreu){
        aplicaGravidade(jogo);
    }
}

void rotacionarDireita(Jogo &jogo){
    
    int aux[TAM][TAM];
    int auxPorta[TAM][TAM];

    for(int k=jogo.n-1, i=0; i<jogo.n; i++, k--){
        for(int j=0; j<jogo.n; j++){
            aux[j][k] = jogo.cenario[i][j];
            auxPorta[j][k] = jogo.tipoPorta[i][j];
        }
    }

    for(int i=0; i<jogo.n; i++){
        for(int j=0; j<jogo.n; j++){
            jogo.cenario[i][j] = aux[i][j];
            jogo.tipoPorta[i][j] = auxPorta[i][j];
            if(jogo.cenario[i][j] == 2){
                jogo.jogador.lin = i;
                jogo.jogador.col = j;
            }
        }
    }
    
    ladoAtual(jogo.orientacao, 1);
    resolveEsmagamento(jogo);
    
    if(!jogo.jogadorMorreu){
        aplicaGravidade(jogo);
    }
}

void moveJogador(Jogo &jogo, char tecla){
    
    int novaLin = jogo.jogador.lin;
    int novaCol = jogo.jogador.col;
    
    if(tecla=='w'){novaLin--;}
    if(tecla=='s'){novaLin++;}
    if(tecla=='a'){novaCol--;}
    if(tecla=='d'){novaCol++;}
    
    if(novaLin<0 || novaLin>=jogo.n || novaCol<0 || novaCol>=jogo.n){return;}
    
    int destino = jogo.cenario[novaLin][novaCol];
    bool podeAndar = (destino==0 || destino==4 || destino==5);
    
    if(destino==6 || destino==7){
        podeAndar = !portaEstaFechada(destino, jogo.orientacao);
    }
    
    if(podeAndar){
        jogo.cenario[jogo.jogador.lin][jogo.jogador.col] = jogo.sobre; // devolve o que estava embaixo
        jogo.sobre = destino;
        jogo.cenario[novaLin][novaCol] = 2;
        jogo.jogador.lin = novaLin;
        jogo.jogador.col = novaCol;
        jogo.movimentos++;
    }
}

// Devolve: 0 = a partida continua, 1 = chegou na saida, 2 = o jogador morreu
int processaTecla(Jogo &jogo, char tecla){
    
    if(tecla>='A' && tecla<='Z'){tecla = tecla + 32;} // maiuscula vira minuscula
    
    switch(tecla){
        case 'w':
        case 's':
        case 'a':
        case 'd':
            moveJogador(jogo, tecla);
            break;
        case 'q':
            if(jogo.sobre == 4){
                rotacionarEsquerda(jogo);
                jogo.rotacoes++;
            }
            break;
        case 'e':
            if(jogo.sobre == 4){
                rotacionarDireita(jogo);
                jogo.rotacoes++;
            }
            break;
        default: break;
    }
    
    if(jogo.jogadorMorreu){return 2;}
    if(jogo.sobre == 5){return 1;}
    return 0;
}

bool mapaEstavel(Jogo jogo){
    
    Jogo copia = jogo;
    aplicaGravidade(copia);
    
    for(int i=0; i<jogo.n; i++){
        for(int j=0; j<jogo.n; j++){
            if(jogo.cenario[i][j] != copia.cenario[i][j]){return false;}
        }
    }
    return true;
}

// Mapas (arquivos)

bool verificarMapa(string caminho, Jogo &jogo, int erro[14]){

    string aux;
    int valor=0, colunas=0, player=0, saida=0, total=0;

    for(int i=0; i<14; i++){erro[i]=0;}

    ifstream arquivo(caminho);
    if(!arquivo.is_open()){erro[0]++; return false;}
    if(!getline(arquivo, aux)){erro[1]++; return false;}
    if(!inteiro(aux, jogo.n)){erro[2]++; return false;}
    if(jogo.n<11 || jogo.n>TAM){erro[3]++; return false;}
    if(!getline(arquivo, jogo.titulo)){erro[4]++; return false;}
    tiraRetorno(jogo.titulo);
    if(jogo.titulo=="" || temVirgula(jogo.titulo)){erro[5]++;}

    for(int x=0; x<jogo.n; x++){
        if(!getline(arquivo, aux)){erro[6]++; break;}
        stringstream linha(aux);
        colunas = 0;
        for(int y=0; y<jogo.n; y++){
            if(!getline(linha, aux, ',')){break;}
            colunas++;
            if(!inteiro(aux, valor)){erro[7]++;}
            else if(valor<0 || valor>7){erro[8]++;}
            else{
                jogo.cenario[x][y] = valor;
                if(valor==2){player++;}
                if(valor==5){saida++;}
            }
        }
        if(colunas<jogo.n){erro[9]++;}
        else if(getline(linha, aux, ',')){erro[10]++;}
    }

    if(getline(arquivo, aux) && aux!=""){erro[11]++;}
    if(player!=1){erro[12]++;}
    if(saida==0){erro[13]++;}
    arquivo.close();

    for(int i=0; i<14; i++){total += erro[i];}
    return total==0;
}

string mensagemErro(int erro[14]){
    
    if(erro[0] >0){return "Arquivo do mapa nao encontrado";}
    if(erro[1] >0){return "Arquivo vazio";}
    if(erro[2] >0){return "Dimensao ausente ou nao numerica";}
    if(erro[3] >0){return "dimensao fora do intervalo permitido";}
    if(erro[4] >0){return "Titulo ausente";}
    if(erro[5] >0){return "Titulo ausente";}
    if(erro[6] >0){return "O arquivo tem menos linhas do que a dimensao informada";}
    if(erro[7] >0){return "Valor nao numerico no cenario";}
    if(erro[8] >0){return "Valor fora do intervalo de 0 a 7";}
    if(erro[9] >0){return "Linha com menos colunas do que a dimensao";}
    if(erro[10]>0){return "Linha com mais colunas do que a dimensao";}
    if(erro[11]>0){return "Existe conteudo depois do cenario";}
    if(erro[12]>0){return "O mapa deve ter exatamente 1 jogador";}
    if(erro[13]>0){return "O mapa nao tem saida";}
    return "";
}

bool carregarMapa(int numero, Jogo &jogo, string &motivo){
    
    int erro[14];
    
    if(!verificarMapa(nomeArquivo(numero, ".csv"), jogo, erro)){
        motivo = mensagemErro(erro);
        return false;
    }
    
    jogo.numeroMapa = numero;
    jogo.orientacao = 0;
    jogo.movimentos = 0;
    jogo.rotacoes = 0;
    jogo.sobre = 0;
    jogo.jogadorMorreu = false;
    identificarPortas(jogo);
    posInicial(jogo);
    
    if(!mapaEstavel(jogo)){
        motivo = "Mapa instavel: existe bloco no ar";
        return false;
    }
    
    motivo = "";
    return true;
}

int contarMapas(){
    
    int quantidade = 0;
    bool existe = true;
    
    while(existe){
        ifstream arquivo(nomeArquivo(quantidade+1, ".csv"));
        if(arquivo.is_open()){
            quantidade++;
            arquivo.close();
        }else{
            existe = false;
        }
    }
    return quantidade;
}

bool mapaValido(int numero){
    
    Jogo teste;
    string motivo;
    
    return carregarMapa(numero, teste, motivo);
}

// Solucoes (arquivos)

bool carregarSolucao(int numero, string &solucao){
    
    ifstream arquivo(nomeArquivo(numero, ".sol"));
    
    if(!arquivo.is_open()){return false;}
    if(!getline(arquivo, solucao)){solucao = "";}
    tiraRetorno(solucao);
    arquivo.close();
    return true;
}


int teclaInvalida(string solucao){
    
    int tamanho = solucao.size();
    
    for(int i=0; i<tamanho; i++){
        char tecla = solucao[i];
        if(tecla>='A' && tecla<='Z'){tecla = tecla + 32;}
        if(tecla!='w' && tecla!='a' && tecla!='s' && tecla!='d' && tecla!='q' && tecla!='e'){
            return i+1;
        }
    }
    return 0;
}


int executarSolucao(Jogo &jogo, string solucao, int &posicao){
    
    int resultado = 0;
    int tamanho = solucao.size();
    
    posicao = teclaInvalida(solucao);
    if(posicao > 0){return 3;}
    
    while(posicao<tamanho && resultado==0){
        resultado = processaTecla(jogo, solucao[posicao]);
        posicao++;
    }
    return resultado;
}


bool lerValores(string linha, int valores[], int quantidade){
    
    stringstream crt(linha);
    string pedaco;
    int contados = 0, valor = 0;
    
    while(getline(crt, pedaco, ',')){
        if(contados>=quantidade){return false;}
        if(!inteiro(pedaco, valor)){return false;}
        valores[contados] = valor;
        contados++;
    }
    return contados==quantidade;
}

bool salvarJogo(const Jogo &jogo){
    
    ofstream arquivo("jogo_salvo.csv");
    
    if(!arquivo.is_open()){return false;}
    
    arquivo<<jogo.numeroMapa<<","<<jogo.n<<","<<jogo.orientacao<<",";
    arquivo<<jogo.jogador.lin<<","<<jogo.jogador.col<<","<<jogo.movimentos<<",";
    arquivo<<jogo.rotacoes<<","<<jogo.sobre<<endl;
    arquivo<<jogo.titulo<<endl;
    
    for(int i=0; i<jogo.n; i++){
        for(int j=0; j<jogo.n; j++){
            arquivo<<jogo.cenario[i][j];
            if(j<jogo.n-1){arquivo<<",";}
        }
        arquivo<<endl;
    }
    
    for(int i=0; i<jogo.n; i++){
        for(int j=0; j<jogo.n; j++){
            arquivo<<jogo.tipoPorta[i][j];
            if(j<jogo.n-1){arquivo<<",";}
        }
        arquivo<<endl;
    }
    
    arquivo.close();
    return true;
}

bool carregarJogoSalvo(Jogo &jogo){
    
    Jogo lido;
    string aux;
    int cab[8];
    int linha[TAM];
    int jogadores = 0;
    
    ifstream arquivo("jogo_salvo.csv");
    
    if(!arquivo.is_open()){return false;}
    if(!getline(arquivo, aux)){return false;}
    if(!lerValores(aux, cab, 8)){return false;}
    
    lido.numeroMapa = cab[0];
    lido.n = cab[1];
    lido.orientacao = cab[2];
    lido.jogador.lin = cab[3];
    lido.jogador.col = cab[4];
    lido.movimentos = cab[5];
    lido.rotacoes = cab[6];
    lido.sobre = cab[7];
    lido.jogadorMorreu = false;
    
    if(lido.numeroMapa<1){return false;}
    if(lido.n<11 || lido.n>TAM){return false;}
    if(lido.orientacao<0 || lido.orientacao>3){return false;}
    if(lido.jogador.lin<0 || lido.jogador.lin>=lido.n){return false;}
    if(lido.jogador.col<0 || lido.jogador.col>=lido.n){return false;}
    if(lido.movimentos<0 || lido.rotacoes<0){return false;}
    if(lido.sobre!=0 && lido.sobre!=4 && lido.sobre!=5 && lido.sobre!=6 && lido.sobre!=7){return false;}
    
    if(!getline(arquivo, lido.titulo)){return false;}
    tiraRetorno(lido.titulo);
    if(lido.titulo==""){return false;}
    
    for(int i=0; i<lido.n; i++){
        if(!getline(arquivo, aux)){return false;}
        if(!lerValores(aux, linha, lido.n)){return false;}
        for(int j=0; j<lido.n; j++){
            if(linha[j]<0 || linha[j]>7){return false;}
            if(linha[j]==2){jogadores++;}
            lido.cenario[i][j] = linha[j];
        }
    }
    
    for(int i=0; i<lido.n; i++){
        if(!getline(arquivo, aux)){return false;}
        if(!lerValores(aux, linha, lido.n)){return false;}
        for(int j=0; j<lido.n; j++){
            if(linha[j]!=0 && linha[j]!=6 && linha[j]!=7){return false;}
            lido.tipoPorta[i][j] = linha[j];
        }
    }
    
    if(jogadores!=1){return false;}
    if(lido.cenario[lido.jogador.lin][lido.jogador.col]!=2){return false;}
    
    arquivo.close();
    jogo = lido;
    return true;
}

bool existeJogoSalvo(){
    
    Jogo teste;
    
    return carregarJogoSalvo(teste);
}

void apagarJogoSalvo(){
    
    remove("jogo_salvo.csv");
}

// Ranking

bool registrarRecorde(Recorde recorde){
    
    ofstream arquivo("recordes.csv", ios_base::app);
    
    if(!arquivo.is_open()){return false;}
    
    arquivo<<recorde.nome<<","<<recorde.mapa<<","<<recorde.movimentos<<","<<recorde.rotacoes<<endl;
    arquivo.close();
    return true;
}


int carregarRecordes(Recorde recordes[], int mapa){
    
    int quantidade = 0;
    int valores[3]; // mapa, movimentos, rotacoes
    string aux, nome, texto;
    
    ifstream arquivo("recordes.csv");
    
    if(!arquivo.is_open()){return 0;}
    
    while(getline(arquivo, aux) && quantidade<200){
        stringstream linha(aux);
        bool valido = true;
        
        if(!getline(linha, nome, ',')){valido = false;}
        for(int i=0; i<3; i++){
            if(!getline(linha, texto, ',')){valido = false;}
            else if(!inteiro(texto, valores[i])){valido = false;}
        }
        
        if(valido && valores[0]==mapa){
            recordes[quantidade].nome = nome;
            recordes[quantidade].mapa = valores[0];
            recordes[quantidade].movimentos = valores[1];
            recordes[quantidade].rotacoes = valores[2];
            quantidade++;
        }
    }
    
    arquivo.close();
    return quantidade;
}


bool vemDepois(const Recorde &a, const Recorde &b){
    
    if(a.movimentos != b.movimentos){return a.movimentos > b.movimentos;}
    return a.rotacoes > b.rotacoes;
}

void ordenarRecordes(Recorde recordes[], int quantidade){
    
    for(int i=0; i<quantidade-1; i++){
        for(int j=0; j<quantidade-1-i; j++){
            if(vemDepois(recordes[j], recordes[j+1])){
                trocar(recordes[j], recordes[j+1]);
            }
        }
    }
}

void exibirRanking(Recorde recordes[], int quantidade){
    
    if(quantidade==0){
        cout<<"Ainda nao ha recordes para este mapa. Seja o primeiro!"<<endl;
    }else{
        for(int i=0; i<quantidade && i<5; i++){
            cout<<"   "<<(i+1)<<"o  "<<recordes[i].nome<<"\t";
            cout<<recordes[i].movimentos<<" movimentos\t";
            cout<<recordes[i].rotacoes<<" rotacoes"<<endl;
        }
    }
}

// Desenho

void desenharCelula(int celula, int orientacao){
    
    switch (celula){
        case 0:
            cout<<" "; // Caminho livre
            break;
        case 1:
            printf("\033[0;44m"); // fundo do texto azul
            cout<<" ";
            printf("\033[0m"); // todo texto normal
            break;
        case 2:
            cout<<"@"; // Jogador
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
            if(portaEstaFechada(6, orientacao)){cout<<"=";}  // Porta
            else{cout << ":";} // Porta
            break;
        case 7: 
            if(portaEstaFechada(7, orientacao)){cout<<"|";} // Porta
            else{cout<<";";} // Porta
            break;
        default:
            cout<<" ";
            break;
    }
}

void desenharCenario(const Jogo &jogo){

    for(int i=0; i<jogo.n; i++){
        cout<<"\t";
        for(int j=0; j<jogo.n; j++){
            desenharCelula(jogo.cenario[i][j], jogo.orientacao);
        }
        cout<<endl;
    }
}

void desenharJogo(const Jogo &jogo){
    
    cout<<"Mapa: "<<jogo.numeroMapa<<" - "<<jogo.titulo<<"\tOrientacao: "<<(jogo.orientacao*90)<<" graus"<<endl<<endl;
    desenharCenario(jogo);
    cout<<"\nMovimentos: "<<jogo.movimentos<<"\tRotacoes: "<<jogo.rotacoes;
}

string textoResultado(int resultado, int posicao, const Jogo &jogo){
    
    stringstream texto;
    
    switch(resultado){
        case 0:
            texto<<"As teclas acabaram e o jogador nao chegou a saida";
            break;
        case 1:
            texto<<"Solucao valida! Teclas: "<<posicao<<", movimentos: "<<jogo.movimentos<<", rotacoes: "<<jogo.rotacoes;
            break;
        case 2:
            texto<<"Derrota na tecla "<<posicao;
            break;
        case 3:
            texto<<"Tecla invalida no arquivo, na posicao "<<posicao;
            break;
        case 4:
            texto<<"Reproducao interrompida";
            break;
        default: break;
    }
    return texto.str();
}


int reproduzirSolucao(Jogo &jogo, string solucao, int &posicao){
    
    char tecla;
    int resultado = 0;
    int tamanho = solucao.size();
    
    posicao = teclaInvalida(solucao);
    if(posicao > 0){return 3;}
    
    cout<<"\033[2J\033[H";
    desenharJogo(jogo);
    cout<<endl<<endl<<"Solucao: "<<solucao<<endl;
    cout<<"Pressione uma tecla para executar a proxima (ESC para parar)"<<endl;
    
    while(posicao<tamanho && resultado==0){
        tecla = getch();
        if(tecla==27){return 4;}
        
        resultado = processaTecla(jogo, solucao[posicao]);
        posicao++;
        
        cout<<"\033[2J\033[H";
        desenharJogo(jogo);
        cout<<endl<<endl<<"Tecla "<<posicao<<" de "<<tamanho<<": "<<solucao[posicao-1]<<endl;
        if(posicao<tamanho && resultado==0){
            cout<<"Pressione uma tecla para executar a proxima (ESC para parar)"<<endl;
        }
    }
    return resultado;
}


int jogar(Jogo &jogo){
    
    char tecla;
    int retorno;
    int resultado = -1; 
    
    cout<<"\033[2J\033[H";

    while (resultado == -1){
        printf("\033[?25l"); // Desativar o cursor no console
        desenharJogo(jogo);

        tecla = getch(); // Pegar tecla

        if(tecla == 27){resultado = 0;} // Esc
        else if(tecla == 'r' || tecla == 'R'){resultado = 2;} // Recarregar o mesmo mapa
        else{
            retorno = processaTecla(jogo, tecla);
            
            if(retorno == 2){
                cout<<"\033[2J\033[H";
                desenharJogo(jogo);
                cout<<endl<<endl<<"Voce foi esmagado por uma porta! Pressione uma tecla..."<<endl;
                getch();
                resultado = 2;
            }
            if(retorno == 1){
                cout<<"\033[2J\033[H";
                desenharJogo(jogo);
                cout<<endl<<endl<<"Parabens! Voce escapou do labirinto!"<<endl;
                cout<<"Movimentos: "<<jogo.movimentos<<"   Rotacoes: "<<jogo.rotacoes<<endl;
                resultado = 1;
            }
        }
        
        if(resultado == -1){
            SetCursorPos(0, 0); // Coloca o cursor na posição inicial
            cout<<"\033[2K";
        }
    }
    
    printf("\033[?25h"); // Ativar o cursor de novo
    return resultado;
}

string pedirNome(){
    
    string nome;
    bool valido = false;
    
    while(!valido){
        cout<<"\nNome do jogador (ate 15 caracteres, sem virgulas): ";
        if(!getline(cin, nome)){
            cin.clear();
            nome = "Anonimo";
        }
        tiraRetorno(nome);
        valido = (nome!="" && nome.size()<=15 && !temVirgula(nome));
    }
    return nome;
}

void partida(Jogo &jogo){
    
    string motivo;
    Recorde recorde;
    bool carregou = true;
    int resultado = jogar(jogo);
    
    while(resultado==2 && carregou){
        carregou = carregarMapa(jogo.numeroMapa, jogo, motivo);
        if(carregou){resultado = jogar(jogo);}
    }
    
    if(!carregou){
        cout<<"\033[2J\033[H";
        cout<<"Nao foi possivel recarregar o mapa: "<<motivo<<endl;
        cout<<"Pressione uma tecla..."<<endl;
        getch();
    }
    
    if(carregou && resultado==0){
        if(!salvarJogo(jogo)){
            cout<<"\033[2J\033[H";
            cout<<"Nao foi possivel salvar o jogo. Pressione uma tecla..."<<endl;
            getch();
        }
    }
    
    if(carregou && resultado==1){
        recorde.nome = pedirNome();
        recorde.mapa = jogo.numeroMapa;
        recorde.movimentos = jogo.movimentos;
        recorde.rotacoes = jogo.rotacoes;
        if(!registrarRecorde(recorde)){
            cout<<"Nao foi possivel gravar o recorde."<<endl;
        }
        apagarJogoSalvo();
        cout<<"Pressione uma tecla..."<<endl;
        getch();
    }
}

// Telas e menus

void telaSobre(){

    cout<<"\033[2J\033[H";
    cout<<"\033[1;34m";
    cout<<"=================================================="<<endl;
    cout<<"              LABIRINTO GIRATORIO                 "<<endl;
    cout<<"=================================================="<<endl;
    cout<<"\033[0m"<<endl;
    cout<<"\033[1;33m";
    cout<<"                 SOBRE O JOGO                   "<<endl;
    cout<<"\033[0m";
    cout<<"--------------------------------------------------"<<endl;
    cout<<"  Equipe:       Nicolas Kremer"<<endl;
    cout<<"  Mes/Ano:      Outubro/2026"<<endl;
    cout<<"  Disciplina:   Algoritmos e Programacao II"<<endl;
    cout<<"  Professor:    Thiago Felski"<<endl<<endl;
    cout<<"\033[1;33m";
    cout<<"                   OBJETIVO                     "<<endl;
    cout<<"\033[0m";
    cout<<"--------------------------------------------------"<<endl;
    cout<<"  Encontre a saida do labirinto utilizando"<<endl;
    cout<<"  movimentos, gravidade e rotacoes do cenario."<<endl<<endl;
    cout<<"\033[1;33m";
    cout<<"               MECANICA DO JOGO                "<<endl;
    cout<<"\033[0m";
    cout<<"--------------------------------------------------"<<endl;
    cout<<"  * O cenario pode ser girado em 90 graus."<<endl;
    cout<<"  * A gravidade sempre aponta para baixo."<<endl;
    cout<<"  * Ao girar, apenas o cenario muda de posicao."<<endl;
    cout<<"  * Blocos soltos (O) caem ate encontrar apoio."<<endl;
    cout<<"  * As portas possuem diferentes orientacoes."<<endl;
    cout<<"  * Uma porta fechada pode esmagar o jogador."<<endl<<endl;
    cout<<"\033[1;33m";
    cout<<"                  CONTROLES                   "<<endl;
    cout<<"\033[0m";
    cout<<"--------------------------------------------------"<<endl;
    cout<<"  W    Mover para cima"<<endl;
    cout<<"  S    Mover para baixo"<<endl;
    cout<<"  A    Mover para esquerda"<<endl;
    cout<<"  D    Mover para direita"<<endl;
    cout<<"  Q    Girar para esquerda (sobre a alavanca)"<<endl;
    cout<<"  E    Girar para direita (sobre a alavanca)"<<endl;
    cout<<"  R    Reiniciar o mapa"<<endl;
    cout<<"  ESC  Voltar ao menu (o jogo e salvo)"<<endl<<endl;
    cout<<"\033[1;33m";
    cout<<"                 OPCOES DO MENU                 "<<endl;
    cout<<"\033[0m";
    cout<<"--------------------------------------------------"<<endl;
    cout<<"  Novo jogo   Escolhe um mapa (ou sorteia um)."<<endl;
    cout<<"  Continuar   Retoma o jogo salvo com ESC, mesmo"<<endl;
    cout<<"              depois de fechar o programa."<<endl;
    cout<<"  Ver solucao Mostra a solucao de um mapa ou"<<endl;
    cout<<"              verifica a solucao de todos."<<endl;
    cout<<"  Recordes    Os 5 melhores resultados de um mapa."<<endl<<endl;
    cout<<"\033[1;34m";
    cout<<"=================================================="<<endl;
    cout<<"       Pressione qualquer tecla para voltar       "<<endl;
    cout<<"=================================================="<<endl;
    cout<<"\033[0m";

    getch();
}

int menuPrincipal(bool temJogoSalvo){

    cout<<"\033[2J\033[H";
    cout<<"\033[1;34m";
    cout<<"=================================================="<<endl;
    cout<<"              LABIRINTO GIRATORIO                 "<<endl;
    cout<<"=================================================="<<endl;
    cout<<"\033[0m"<<endl;
    cout<<"   1  -  Novo jogo"<<endl;
    
    if(temJogoSalvo){
        cout<<"   2  -  Continuar"<<endl;
    }
    
    cout<<"   3  -  Ver solucao"<<endl;
    cout<<"   4  -  Recordes"<<endl;
    cout<<"   5  -  Sobre"<<endl;
    cout<<"   6  -  Fim"<<endl<<endl;
    cout<<"Escolha uma opcao: ";

    int opcao = lerOpcao();
    
    if(opcao==2 && !temJogoSalvo){
        return 0;
    }
    return opcao;
}

void listarMapas(){
    
    int quantidade = contarMapas();
    Jogo teste;
    string motivo;
    
    if(quantidade==0){
        cout<<"   Nenhum mapa encontrado."<<endl;
    }
    
    for(int i=1; i<=quantidade; i++){
        if(carregarMapa(i, teste, motivo)){
            cout<<"   "<<i<<"  -  "<<teste.titulo<<endl;
        }else{
            cout<<"   "<<i<<"  -  [indisponivel] "<<motivo<<endl;
        }
    }
}

// Sorteia um mapa entre os validos. Devolve 0 se nao houver nenhum
int sortearMapa(int quantidade){
    
    int validos = 0;
    
    for(int i=1; i<=quantidade; i++){
        if(mapaValido(i)){validos++;}
    }
    if(validos==0){return 0;}
    
    int sorteado = rand()%validos;
    
    for(int i=1; i<=quantidade; i++){
        if(mapaValido(i)){
            if(sorteado==0){return i;}
            sorteado--;
        }
    }
    return 0;
}

// Devolve o numero do mapa escolhido, ou 0 se o jogador desistiu
int escolheMapa(){

    cout<<"\033[2J\033[H";
    cout<<"\033[1;34m";
    cout<<"==================================================" << endl;
    cout<<"                    NOVO JOGO                     " << endl;
    cout<<"==================================================" << endl;
    cout<<"\033[0m"<<endl;
    listarMapas();
    cout<<endl;
    cout<<"   1  -  Escolher o mapa"<<endl;
    cout<<"   2  -  Mapa aleatorio"<<endl;
    cout<<"   3  -  Voltar"<<endl<<endl;
    cout<<"Escolha uma opcao: ";

    int opcao = lerOpcao();
    int quantidade = contarMapas();
    int numero = 0;

    if(opcao == 1){
        cout<< "\nNumero do mapa (1 a "<<quantidade<<"): ";
        numero = lerOpcao();
        
        if(numero<1 || numero>quantidade || !mapaValido(numero)){
            cout<<"\nMapa indisponivel! Pressione uma tecla..."<<endl;
            getch();
            numero = 0;
        }
    }
    
    if(opcao == 2){
        numero = sortearMapa(quantidade);
        
        if(numero == 0){
            cout<<"\nNenhum mapa valido! Pressione uma tecla..."<<endl;
            getch();
        }
    }

    return numero;
}

void novoJogo(){
    
    Jogo jogo;
    string motivo;
    int numero;
    
    if(existeJogoSalvo()){
        cout<<"\033[2J\033[H";
        cout<<"Ja existe um jogo salvo. Deseja substitui-lo?"<<endl<<endl;
        cout<<"   1  -  Sim"<<endl;
        cout<<"   2  -  Nao"<<endl<<endl;
        cout<<"Escolha uma opcao: ";
        if(lerOpcao()!=1){return;}
    }
    
    numero = escolheMapa();
    
    if(numero != 0){
        if(carregarMapa(numero, jogo, motivo)){
            partida(jogo);
        }else{
            cout<<"\nNao foi possivel carregar o mapa: "<<motivo<<endl;
            cout<<"Pressione uma tecla..."<<endl;
            getch();
        }
    }
}

void continuar(){
    
    Jogo jogo;
    
    if(carregarJogoSalvo(jogo)){
        partida(jogo);
    }else{
        cout<<"\033[2J\033[H";
        cout<<"Nao ha jogo salvo valido. Pressione uma tecla..."<<endl;
        getch();
    }
}

void verificarTodos(){
    
    Jogo jogo;
    string motivo, solucao;
    int posicao, resultado;
    int quantidade = contarMapas();
    
    cout<<"\033[2J\033[H";
    cout<<"\033[1;34m";
    cout<<"==================================================" << endl;
    cout<<"              VERIFICAR TODOS OS MAPAS            " << endl;
    cout<<"==================================================" << endl;
    cout<<"\033[0m"<<endl;
    
    for(int i=1; i<=quantidade; i++){
        cout<<"Mapa "<<i;
        if(!carregarMapa(i, jogo, motivo)){
            cout<<" - Indisponivel: "<<motivo<<endl;
        }else if(!carregarSolucao(i, solucao)){
            cout<<" - "<<jogo.titulo<<": arquivo "<<nomeArquivo(i, ".sol")<<" inexistente"<<endl;
        }else{
            resultado = executarSolucao(jogo, solucao, posicao);
            cout<<" - "<<jogo.titulo<<": "<<textoResultado(resultado, posicao, jogo)<<endl;
        }
    }
    
    cout<<endl<<"Pressione uma tecla..."<<endl;
    getch();
}

void verSolucao(){
    
    Jogo jogo;
    string motivo, solucao;
    int posicao, resultado, numero;

    cout<<"\033[2J\033[H";
    cout<<"\033[1;34m";
    cout<<"==================================================" << endl;
    cout<<"                   VER SOLUCAO                    " << endl;
    cout<<"==================================================" << endl;
    cout<<"\033[0m"<<endl;
    cout<<"   1  -  Reproduzir a solucao de um mapa"<<endl;
    cout<<"   2  -  Verificar todos os mapas"<<endl;
    cout<<"   3  -  Voltar"<<endl<<endl;
    cout<<"Escolha uma opcao: ";

    int opcao = lerOpcao();
    
    if(opcao == 1){
        cout<<endl;
        listarMapas();
        cout<<"\nNumero do mapa: ";
        numero = lerOpcao();
        
        if(numero<1 || numero>contarMapas()){
            cout<<"\nMapa inexistente!"<<endl;
        }else if(!carregarMapa(numero, jogo, motivo)){
            cout<<"\nMapa indisponivel: "<<motivo<<endl;
        }else if(!carregarSolucao(numero, solucao)){
            cout<<"\nArquivo "<<nomeArquivo(numero, ".sol")<<" inexistente!"<<endl;
        }else{
            resultado = reproduzirSolucao(jogo, solucao, posicao);
            cout<<endl<<textoResultado(resultado, posicao, jogo)<<endl;
        }
        
        cout<<"Pressione uma tecla..."<<endl;
        getch();
    }
    
    if(opcao == 2){
        verificarTodos();
    }
}

void telaRecordes(){
    
    Recorde recordes[200];
    int quantidade, numero;
    
    cout<<"\033[2J\033[H";
    cout<<"\033[1;34m";
    cout<<"==================================================" << endl;
    cout<<"                     RECORDES                     " << endl;
    cout<<"==================================================" << endl;
    cout<<"\033[0m"<<endl;
    cout<<"Numero do mapa (1 a "<<contarMapas()<<"): ";
    numero = lerOpcao();
    
    cout<<endl;
    if(numero<1 || numero>contarMapas()){
        cout<<"Mapa inexistente!"<<endl;
    }else{
        quantidade = carregarRecordes(recordes, numero);
        ordenarRecordes(recordes, quantidade);
        exibirRanking(recordes, quantidade);
    }
    
    cout<<endl<<"Pressione uma tecla..."<<endl;
    getch();
}

int main(){
    
    int opcao;

    srand((unsigned)time(NULL));
    do{
        opcao = menuPrincipal(existeJogoSalvo());
        switch(opcao){
            case 1:
                novoJogo();
                break;
            case 2:
                continuar();
                break;
            case 3:
                verSolucao();
                break;
            case 4:
                telaRecordes();
                break;
            case 5:
                telaSobre();
                break;
            default: break;
        }
    }while(opcao!=6);

    cout<<"\033[2J\033[H";
    return 0;
}