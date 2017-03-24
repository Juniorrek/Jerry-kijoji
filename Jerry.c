#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

#define MAXIJ 30
#define LIVRE 0
#define PAREDE 1
#define VISITADA 2
#define BECO 3
#define LARGURA_TELA 720
#define ALTURA_TELA 720

typedef struct Ra{
    int linha, coluna, linhaAnterior, colunaAnterior;
}Rat;

typedef struct Bloco{
    int ij;
    struct Bloco *proximo;
}Pilha;

void gotoXY(int x, int y){
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(hStdOut, coord);
}

void inicializaLabirinto(int labirinto[MAXIJ][MAXIJ]){
    int linha, coluna;

    srand(time(NULL));
    for(linha=0; linha<MAXIJ; linha++){
        for(coluna=0; coluna<MAXIJ; coluna++){
            if((linha==0)||(linha==MAXIJ-1)||(coluna==0)||(coluna==MAXIJ-1))//parede superior, inferior, esquerda e direita
                labirinto[linha][coluna]=PAREDE;
            else{
                if(rand()%3==0)//0=parede, 1 ou 2=livre
                    labirinto[linha][coluna]=PAREDE;//parede aleatorias
                else
                    labirinto[linha][coluna]=LIVRE;//livre
            }
        }
    }
    //SAIDA
    linha=rand()%4;//parede 0superior, 1inferior, 2esquerda e 3direita
    coluna=rand()%27+2;//posição na parede x123456789x rand 0-27 -> +1, ficar de olho no +1, menos na 01 e 10 pq bug...30 pos, quero 27 pra nao quero os cantos e os primeiros, ando dois
    if(linha==0)
        labirinto[0][coluna]=LIVRE;
    else if(linha==1)
        labirinto[MAXIJ-1][coluna]=LIVRE;
    else if(linha==2)
        labirinto[coluna][0]=LIVRE;
    else
        labirinto[coluna][MAXIJ-1]=LIVRE;
    labirinto[1][1]=VISITADA;//limpa o espaço do rato

    for(linha=0; linha<MAXIJ; linha++){//desenha o labirinto
        printf(" ");
        for(coluna=0; coluna<MAXIJ; coluna++){
            if(labirinto[linha][coluna]==PAREDE)
                printf("%c", 219);
            else
                printf(" ");
        }
        printf("\n");
    }
}

void inicializaPilha(Pilha **P){
    *P=NULL;
}

Pilha *criarBloco(){
    return ((Pilha*)malloc(sizeof(Pilha)));
}

void empilhar(int ij, Pilha **P){
    Pilha *novo, *auxiliar;

    novo=criarBloco();
    novo->proximo=NULL;
    novo->ij=ij;
    if(*P==NULL)//bug1
        *P=novo;
    else{
        auxiliar=*P;
        while(auxiliar->proximo!=NULL)
            auxiliar=auxiliar->proximo;
        auxiliar->proximo=novo;
    }
}

Pilha *desempilhar(Pilha **P){
    Pilha *auxiliar1, *auxiliar2;

    auxiliar1=*P;
    if(auxiliar1->proximo==NULL)//só tem um entao vai desempilhar e zera pilha
        *P=NULL;
    else{
        for(auxiliar1=*P; auxiliar1->proximo!=NULL; auxiliar1=auxiliar1->proximo){
            auxiliar2=auxiliar1;
        }
        auxiliar2->proximo=NULL;
    }
    free(auxiliar1);
    return auxiliar2;
}

int andaRato(Rat *rato, Pilha **P, int labirinto[MAXIJ][MAXIJ]){//-1 fim, 0cima 1direita 2baixo 3esquerda
    int random, ij;
    Pilha *auxiliar;

    rato->linhaAnterior=rato->linha;
    rato->colunaAnterior=rato->coluna;
    if((rato->linha==0)||(rato->linha==MAXIJ-1)||(rato->coluna==0)||(rato->coluna==MAXIJ-1))//ganhou
        return -1;
    if((labirinto[rato->linha-1][rato->coluna]==LIVRE)||(labirinto[rato->linha][rato->coluna+1]==LIVRE)||(labirinto[rato->linha+1][rato->coluna]==LIVRE)||(labirinto[rato->linha][rato->coluna-1]==LIVRE)){
        random=rand()%4;//0cima 1direita 2baixo 3esquerda
        if((random==0)&&(labirinto[rato->linha-1][rato->coluna]==LIVRE)){
            ij=(rato->linha*100)+(rato->coluna+1); //ij pra onde o rato vai, esse ij inicia no 11 e nao no 00, entao se soma 1 de i e j e depois diminui oq precisa....
            labirinto[rato->linha-1][rato->coluna]=VISITADA; //visitado para onde o rato vai
            rato->linha--; //rato vai
            empilhar(ij, P); //empilha posiçao do rato
            return 0;
        }else if((random==1)&&(labirinto[rato->linha][rato->coluna+1]==LIVRE)){
            ij=((rato->linha+1)*100)+(rato->coluna+2);
            labirinto[rato->linha][rato->coluna+1]=VISITADA;
            rato->coluna++;
            empilhar(ij, P);
            return 1;
        }else if((random==2)&&(labirinto[rato->linha+1][rato->coluna]==LIVRE)){
            ij=((rato->linha+2)*100)+(rato->coluna+1);
            labirinto[rato->linha+1][rato->coluna]=VISITADA;
            rato->linha++;
            empilhar(ij, P);
            return 2;
        }else if((random==3)&&(labirinto[rato->linha][rato->coluna-1]==LIVRE)){
            ij=((rato->linha+1)*100)+rato->coluna;
            labirinto[rato->linha][rato->coluna-1]=VISITADA;
            rato->coluna--;
            empilhar(ij, P);
            return 3;
        }
    }else if((*P)->proximo!=NULL){//nao a espaço livre, entao volta e marca beco
        labirinto[rato->linha][rato->coluna]=BECO;
        auxiliar=desempilhar(P);
        if(((auxiliar->ij/100)-1==rato->linha-1)&&((auxiliar->ij%100)-1==rato->coluna))//voltando pra cima....
            random=0;
        else if(((auxiliar->ij/100)-1==rato->linha)&&((auxiliar->ij%100)-1==rato->coluna+1))
            random=1;
        else if(((auxiliar->ij/100)-1==rato->linha+1)&&((auxiliar->ij%100)-1==rato->coluna))
            random=2;
        else
            random=3;
        rato->linha=(auxiliar->ij/100)-1;// volta do 1,1 pro 0,0
        rato->coluna=(auxiliar->ij%100)-1;
        return random;
    }else if(*P==NULL)//nasce fechado *P==NULL
        return -2;
    else if((rato->linha==1)&&(rato->coluna==1)&&(labirinto[rato->linha][rato->coluna+1]!=0)&&(labirinto[rato->linha+1][rato->coluna]!=0))//volto no inicio e nao tem mais lugar open > \/, nunca chega
        return -2;

    return 4;
}

void imprimeLabirinto(int labirinto[MAXIJ][MAXIJ], Rat rato, int labirinto2[MAXIJ][MAXIJ]){
    int linha, coluna;

    for(linha=0; linha<MAXIJ; linha++){
        for(coluna=0; coluna<MAXIJ; coluna++){
            if(labirinto[linha][coluna]!=labirinto2[linha][coluna]){
                if(labirinto[linha][coluna]==BECO){//se o rato ja tiver passado
                    gotoXY(coluna+1, linha);
                    printf("%c", 176);
                }else{// se ja tiver passado
                    gotoXY(rato.colunaAnterior+1, rato.linhaAnterior);
                    printf(".");
                }
            }
        }
    }
    gotoXY(rato.coluna+1, rato.linha);//rato
    printf("%c", 1);
}

int libera_lista(Pilha **N){
    Pilha *aux1, *aux2;
    if(*N==NULL)
        return 0;
    else{
        /*
        for(aux1=*N ; aux1!=NULL; aux2=aux2->prox){ //4 parametros bug, parou de funcionar
                if(trig==0){
                    aux2=(*N)->prox;
                    trig=1;
                }
            free(aux1);
            aux1=aux2;
        }
        */
        while((*N)->proximo!=NULL){//se tirar esse while ele fica apagando o ultimo
            for(aux1=*N;aux1->proximo!=NULL;aux1=aux1->proximo){
                aux2=aux1->proximo;
                if(aux2->proximo==NULL){
                    free(aux2);
                    aux1->proximo=NULL;
                    break;
                }
            }
        }
        free((*N)->proximo);
    }
    *N=NULL;
    return 0;
}

void inicializaLabirinto2(int labirinto[MAXIJ][MAXIJ], Rat rato){
    int linha, coluna, random, ij, sair=0;
    Pilha *P, *auxiliar;

    inicializaPilha(&P);
    srand(time(NULL));
    for(linha=0; linha<MAXIJ; linha++){
        for(coluna=0; coluna<MAXIJ; coluna++){
            labirinto[linha][coluna]=1;//fecha tudo
        }
    }
    labirinto[1][1]=0;
    empilhar(202, &P);

    do{

        if(((labirinto[rato.linha-1][rato.coluna]!=0)&&(labirinto[rato.linha-2][rato.coluna]==1)&&(labirinto[rato.linha-1][rato.coluna+1]==1)&&(labirinto[rato.linha-1][rato.coluna-1]==1)&&(rato.linha-1!=0))||//cima
           ((labirinto[rato.linha][rato.coluna+1]!=0)&&(labirinto[rato.linha][rato.coluna+2]==1)&&(labirinto[rato.linha+1][rato.coluna+1]==1)&&(labirinto[rato.linha-1][rato.coluna+1]==1)&&(rato.coluna+1!=29))||
           ((labirinto[rato.linha+1][rato.coluna]!=0)&&(labirinto[rato.linha+2][rato.coluna]==1)&&(labirinto[rato.linha+1][rato.coluna+1]==1)&&(labirinto[rato.linha+1][rato.coluna-1]==1)&&(rato.linha+1!=29))||
           ((labirinto[rato.linha][rato.coluna-1]!=0)&&(labirinto[rato.linha][rato.coluna-2]==1)&&(labirinto[rato.linha+1][rato.coluna-1]==1)&&(labirinto[rato.linha-1][rato.coluna-1]==1)&&(rato.coluna-1!=0))){
            random=rand()%4;//0cima 1direita 2baixo 3esquerda
            if((random==0)&&(labirinto[rato.linha-1][rato.coluna]!=0)&&(labirinto[rato.linha-2][rato.coluna]==1)&&(labirinto[rato.linha-1][rato.coluna+1]==1)&&(labirinto[rato.linha-1][rato.coluna-1]==1)&&(rato.linha-1!=0)){
                ij=(rato.linha*100)+(rato.coluna+1); //ij pra onde o rato vai, esse ij inicia no 11 e nao no 00, entao se soma 1 de i e j e depois diminui oq precisa....
                labirinto[rato.linha-1][rato.coluna]=0; //visitado para onde o rato vai
                rato.linha--; //rato vai
                empilhar(ij, &P); //empilha posiçao do rato
            }else if((random==1)&&(labirinto[rato.linha][rato.coluna+1]!=0)&&(labirinto[rato.linha][rato.coluna+2]==1)&&(labirinto[rato.linha+1][rato.coluna+1]==1)&&(labirinto[rato.linha-1][rato.coluna+1]==1)&&(rato.coluna+1!=29)){
                ij=((rato.linha+1)*100)+(rato.coluna+2);
                labirinto[rato.linha][rato.coluna+1]=0;
                rato.coluna++;
                empilhar(ij, &P);
            }else if((random==2)&&(labirinto[rato.linha+1][rato.coluna]!=0)&&(labirinto[rato.linha+2][rato.coluna]==1)&&(labirinto[rato.linha+1][rato.coluna+1]==1)&&(labirinto[rato.linha+1][rato.coluna-1]==1)&&(rato.linha+1!=29)){
                ij=((rato.linha+2)*100)+(rato.coluna+1);
                labirinto[rato.linha+1][rato.coluna]=0;
                rato.linha++;
                empilhar(ij, &P);
            }else if((random==3)&&(labirinto[rato.linha][rato.coluna-1]!=0)&&(labirinto[rato.linha][rato.coluna-2]==1)&&(labirinto[rato.linha-1][rato.coluna-1]==1)&&(labirinto[rato.linha+1][rato.coluna-1]==1)&&(rato.coluna-1!=0)){
                ij=((rato.linha+1)*100)+rato.coluna;
                labirinto[rato.linha][rato.coluna-1]=0;
                rato.coluna--;
                empilhar(ij, &P);
            }
        }else if((*P).proximo!=NULL){//nao a espaço livre, entao volta e marca beco
            labirinto[rato.linha][rato.coluna]=0;
            auxiliar=desempilhar(&P);
            rato.linha=(auxiliar->ij/100)-1;// volta do 1,1 pro 0,0
            rato.coluna=(auxiliar->ij%100)-1;
        }else// *P==NULL
            sair=1;
    }while(sair!=1);

    //SAIDA
    linha=rand()%4;//parede 0superior, 1inferior, 2esquerda e 3direita
    coluna=rand()%28+1;//posição na parede x123456789x rand 0-27 -> +1, ficar de olho no +1
    if(linha==0)
        labirinto[0][coluna]=0;
    else if(linha==1)
        labirinto[MAXIJ-1][coluna]=0;
    else if(linha==2)
        labirinto[coluna][0]=0;
    else
        labirinto[coluna][MAXIJ-1]=0;
    labirinto[1][1]=2;//limpa o espaço do rato

    for(linha=0; linha<MAXIJ; linha++){//desenha o labirinto
        printf(" ");
        for(coluna=0; coluna<MAXIJ; coluna++){
            if(labirinto[linha][coluna]==1)
                printf("%c", 219);
            else
                printf(" ");
        }
        printf("\n");
    }
}

int main(int argc, char *args[]){
    int umplay, imgpiso, imgrato, delay, udt, music, jogar, prim, random, opcao, linha, coluna, labirinto[MAXIJ][MAXIJ], labirinto2[MAXIJ][MAXIJ], lab;//0=livre, 1=parede, 2=visitada, 3=beco *4rato, rato nao existiria e seria mais um int na matriz, ai na impressao verificando as diferença nos labirinto, quando o rato andasse teria que trocar os numeros, aonde vai para rato aonde tava para visitado...*,,,,matrizes sao ponteiros, passam como ponteiros
    Rat rato;
    Pilha *P;
    SDL_Window *janela;
    SDL_Surface *superficie;
    SDL_Surface *chao;
    SDL_Surface *parede;
    SDL_Surface *visitada;
    SDL_Surface *beco;
    SDL_Surface *ratoc[3];
    SDL_Surface *ratod[3];
    SDL_Surface *ratob[3];
    SDL_Surface *ratoe[3];
    SDL_Surface *menu;
    SDL_Surface *config;
    SDL_Surface *seta1;
    SDL_Surface *seta2;
    SDL_Surface *intro;
    SDL_Surface *saida;
    SDL_Surface *ganhou;
    SDL_Surface *perdeu;
    SDL_Rect objeto;
    SDL_Event evento;
    Mix_Music *musicaMenu;
    Mix_Music *musicaIngame;
    Mix_Chunk *somIntro;

    objeto.w=24;
    objeto.h=24;
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);//Inicia funçoes de video do SDL
    janela=SDL_CreateWindow("Jerry Kijoji", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, LARGURA_TELA, ALTURA_TELA, 0);//Cria a janela
    superficie=SDL_GetWindowSurface(janela);//superficie-janela
    IMG_Init(IMG_INIT_PNG);//Inicia função para importar png
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 1024 );//inicializa mixer
    parede=IMG_Load("media/parede.png");//importa o png pra superficie
    visitada=IMG_Load("media/visitada.png");
    beco=IMG_Load("media/beco.png");
    menu=IMG_Load("media/menu.png");
    config=IMG_Load("media/config.png");
    seta1=IMG_Load("media/seta1.png");
    seta2=IMG_Load("media/seta2.png");
    intro=IMG_Load("media/intro.png");
    ganhou=IMG_Load("media/ganhou.png");
    perdeu=IMG_Load("media/perdeu.png");
    musicaMenu=Mix_LoadMUS("media/menu.wav");
    musicaIngame=Mix_LoadMUS("media/ingame.wav");
    somIntro=Mix_LoadWAV("media/intro.wav");
    SDL_Delay(1000);
    SDL_BlitScaled(intro, NULL, superficie, NULL);
    SDL_UpdateWindowSurface(janela);
    Mix_PlayChannel( -1, somIntro, 0 );
    SDL_Delay(2000);
    udt=2;//um dois tres config
    imgpiso=1;
    imgrato=1;
    umplay=0;
    do{
        opcao=5;
        jogar=2;
        music=1;
        do{//MENU/GAME
            if(udt==1)
                delay=20;
            else if(udt==2)
                delay=10;
            else if(udt==3)
                delay=1;
            if(music==1){
                Mix_HaltMusic();
                Mix_PlayMusic( musicaMenu, -1 );
                music=0;
            }
            objeto.h=20;
            objeto.w=100;
            if(jogar==2){//jogar
                SDL_BlitScaled(menu, NULL, superficie, NULL);
                objeto.y=390;
                objeto.x=140;
                SDL_BlitScaled(seta1, NULL, superficie, &objeto);
                objeto.y=390;
                objeto.x=485;
                SDL_BlitScaled(seta1, NULL, superficie, &objeto);
            }else if(jogar==1){//config
                SDL_BlitScaled(menu, NULL, superficie, NULL);
                objeto.y=500;
                objeto.x=120;
                SDL_BlitScaled(seta1, NULL, superficie, &objeto);
                objeto.y=500;
                objeto.x=500;
                SDL_BlitScaled(seta1, NULL, superficie, &objeto);
            }else{//sair
                SDL_BlitScaled(menu, NULL, superficie, NULL);
                objeto.y=610;
                objeto.x=180;
                SDL_BlitScaled(seta1, NULL, superficie, &objeto);
                objeto.y=610;
                objeto.x=445;
                SDL_BlitScaled(seta1, NULL, superficie, &objeto);
            }
            objeto.h=24;
            objeto.w=24;
            SDL_UpdateWindowSurface(janela);
            while( SDL_PollEvent( &evento ) != 0 ){
					if( evento.type == SDL_QUIT )
						opcao=-1;
					else if( evento.type == SDL_KEYDOWN ){
                        Mix_PlayChannel( -1, somIntro, 0 );
						switch( evento.key.keysym.sym ){
                        case SDLK_UP:
                            if(jogar+1<3)
                                jogar++;
                            break;
                        case SDLK_DOWN:
                            if(jogar+1>-1)
                                jogar--;
                            break;
                        case SDLK_KP_ENTER:
                            if(jogar==2){
                                opcao=1;
                                lab=1;
                            }else if(jogar==1)
                                opcao=2;
                            else
                                opcao=-1;
                            break;
						case SDLK_RETURN:
                            if(jogar==2){
                                opcao=1;
                                lab=1;
                            }else if(jogar==1)
                                opcao=2;
                            else
                                opcao=-1;
                            break;
                        case SDLK_LEFT:
							jogar=1;
							break;
                        case SDLK_RIGHT:
							jogar=2;
							break;
                        case SDLK_F2:
                            opcao=1;
                            lab=2;
						}
					}
            }
            if(opcao==1){//CARREGA GAME
                umplay=1;
                if(music==0){
                    Mix_HaltMusic();
                    Mix_PlayMusic(musicaIngame, -1 );
                    music=1;
                }
                rato.coluna=1;
                rato.linha=1;
                prim=0;
                inicializaPilha(&P);
                if(lab==1)
                    inicializaLabirinto(labirinto);
                else
                    inicializaLabirinto2(labirinto, rato);
                if(imgrato==1){
                    saida=IMG_Load("media/queijo.png");
                    ratoc[0]=IMG_Load("media/rato1c1.png");
                    ratoc[1]=IMG_Load("media/rato1c2.png");
                    ratoc[2]=IMG_Load("media/rato1c3.png");
                    ratod[0]=IMG_Load("media/rato1d1.png");
                    ratod[1]=IMG_Load("media/rato1d2.png");
                    ratod[2]=IMG_Load("media/rato1d3.png");
                    ratob[0]=IMG_Load("media/rato1b1.png");
                    ratob[1]=IMG_Load("media/rato1b2.png");
                    ratob[2]=IMG_Load("media/rato1b3.png");
                    ratoe[0]=IMG_Load("media/rato1e1.png");
                    ratoe[1]=IMG_Load("media/rato1e2.png");
                    ratoe[2]=IMG_Load("media/rato1e3.png");
                }else if(imgrato==2){
                    saida=IMG_Load("media/cenoura.png");
                    ratoc[0]=IMG_Load("media/rato2c1.png");
                    ratoc[1]=IMG_Load("media/rato2c2.png");
                    ratoc[2]=IMG_Load("media/rato2c3.png");
                    ratod[0]=IMG_Load("media/rato2d1.png");
                    ratod[1]=IMG_Load("media/rato2d2.png");
                    ratod[2]=IMG_Load("media/rato2d3.png");
                    ratob[0]=IMG_Load("media/rato2b1.png");
                    ratob[1]=IMG_Load("media/rato2b2.png");
                    ratob[2]=IMG_Load("media/rato2b3.png");
                    ratoe[0]=IMG_Load("media/rato2e1.png");
                    ratoe[1]=IMG_Load("media/rato2e2.png");
                    ratoe[2]=IMG_Load("media/rato2e3.png");
                }else if(imgrato==3){
                    saida=IMG_Load("media/coxa.png");
                    ratoc[0]=IMG_Load("media/rato3c1.png");
                    ratoc[1]=IMG_Load("media/rato3c2.png");
                    ratoc[2]=IMG_Load("media/rato3c3.png");
                    ratod[0]=IMG_Load("media/rato3d1.png");
                    ratod[1]=IMG_Load("media/rato3d2.png");
                    ratod[2]=IMG_Load("media/rato3d3.png");
                    ratob[0]=IMG_Load("media/rato3b1.png");
                    ratob[1]=IMG_Load("media/rato3b2.png");
                    ratob[2]=IMG_Load("media/rato3b3.png");
                    ratoe[0]=IMG_Load("media/rato3e1.png");
                    ratoe[1]=IMG_Load("media/rato3e2.png");
                    ratoe[2]=IMG_Load("media/rato3e3.png");
                }else{
                    saida=IMG_Load("media/rato.png");
                    ratoc[0]=IMG_Load("media/rato4c1.png");
                    ratoc[1]=IMG_Load("media/rato4c2.png");
                    ratoc[2]=IMG_Load("media/rato4c3.png");
                    ratod[0]=IMG_Load("media/rato4d1.png");
                    ratod[1]=IMG_Load("media/rato4d2.png");
                    ratod[2]=IMG_Load("media/rato4d3.png");
                    ratob[0]=IMG_Load("media/rato4b1.png");
                    ratob[1]=IMG_Load("media/rato4b2.png");
                    ratob[2]=IMG_Load("media/rato4b3.png");
                    ratoe[0]=IMG_Load("media/rato4e1.png");
                    ratoe[1]=IMG_Load("media/rato4e2.png");
                    ratoe[2]=IMG_Load("media/rato4e3.png");
                }

                if(imgpiso==1){
                    chao=IMG_Load("media/piso1.png");
                }else if(imgpiso==2){
                    chao=IMG_Load("media/piso2.png");
                }else{
                    chao=IMG_Load("media/piso3.png");
                }
                empilhar(202, &P);//posiçao inicial do rato começa na 2,2(ij...)
                for(linha=0; linha<MAXIJ; linha++){//MONTA O CHAO
                    objeto.y=linha*24;
                    for(coluna=0; coluna<MAXIJ; coluna++){
                        objeto.x=coluna*24;
                        SDL_BlitSurface(chao, NULL, superficie, &objeto);//Joga imagem na superficie na posiçao
                    }
                }
                for(linha=0; linha<MAXIJ; linha++){//COLOCAR AS PAREDES
                    objeto.y=linha*24;
                    for(coluna=0; coluna<MAXIJ; coluna++){
                        objeto.x=coluna*24;
                        if(labirinto[linha][coluna]==PAREDE)
                            SDL_BlitScaled(parede, NULL, superficie, &objeto);
                        else if((linha==0)||(linha==MAXIJ-1)||(coluna==0)||(coluna==MAXIJ-1))
                            SDL_BlitScaled(saida, NULL, superficie, &objeto);
                    }
                }
                do{//GAMELOOP
                    for(linha=0; linha<MAXIJ; linha++){//copio lab 1 no 2
                        for(coluna=0; coluna<MAXIJ; coluna++){
                            labirinto2[linha][coluna]=labirinto[linha][coluna];
                        }
                    }
                    random=andaRato(&rato, &P, labirinto);//altero lab1 retorna posiçao que foi /\ > \/ <
                    if((random==-1)||(random==-2)){//ganho ou perdeu
                        opcao=0;
                        objeto.h=79;
                        objeto.w=530;
                        objeto.x=LARGURA_TELA/6;
                        objeto.y=ALTURA_TELA/2;
                        if(random==-1)
                            SDL_BlitScaled(ganhou, NULL, superficie, &objeto);
                        else
                            SDL_BlitScaled(perdeu, NULL, superficie, &objeto);
                    }
                    if(prim==0){
                        objeto.x=24;
                        objeto.y=24;
                        objeto.h=24;
                        objeto.w=24;
                        SDL_BlitScaled(chao, NULL, superficie, &objeto);
                        SDL_BlitScaled(ratob[0], NULL, superficie, &objeto);//primeira forma do rato
                        SDL_UpdateWindowSurface(janela);
                        prim=1;
                    }else{
                        switch(random){
                        case 0:
                            for(linha=rato.linhaAnterior*24; linha>rato.linha*24; linha--){
                                random=rand()%3;
                                objeto.x=rato.colunaAnterior*24;
                                objeto.y=rato.linhaAnterior*24;
                                SDL_BlitScaled(chao, NULL, superficie, &objeto);
                                objeto.x=rato.coluna*24;
                                objeto.y=rato.linha*24;
                                SDL_BlitScaled(chao, NULL, superficie, &objeto);
                                objeto.x=rato.coluna*24;
                                objeto.y=linha;
                                switch(random){
                                case 0:
                                    SDL_BlitScaled(ratoc[0], NULL, superficie, &objeto);//desenha rato
                                    break;
                                case 1:
                                    SDL_BlitScaled(ratoc[1], NULL, superficie, &objeto);
                                    break;
                                case 2:
                                    SDL_BlitScaled(ratoc[2], NULL, superficie, &objeto);
                                    break;
                                }
                                SDL_UpdateWindowSurface(janela);
                                SDL_Delay(delay);
                            }
                            break;
                        case 1:
                            for(coluna=rato.colunaAnterior*24; coluna<rato.coluna*24; coluna++){
                                random=rand()%3;
                                objeto.x=rato.colunaAnterior*24;
                                objeto.y=rato.linhaAnterior*24;
                                SDL_BlitScaled(chao, NULL, superficie, &objeto);
                                objeto.x=rato.coluna*24;
                                objeto.y=rato.linha*24;
                                SDL_BlitScaled(chao, NULL, superficie, &objeto);
                                objeto.x=coluna;
                                objeto.y=rato.linha*24;
                                switch(random){
                                case 0:
                                    SDL_BlitScaled(ratod[0], NULL, superficie, &objeto);
                                    break;
                                case 1:
                                    SDL_BlitScaled(ratod[1], NULL, superficie, &objeto);
                                    break;
                                case 2:
                                    SDL_BlitScaled(ratod[2], NULL, superficie, &objeto);
                                    break;
                                }
                                SDL_UpdateWindowSurface(janela);
                                SDL_Delay(delay);
                            }
                            break;
                        case 2:
                            for(linha=rato.linhaAnterior*24; linha<rato.linha*24; linha++){
                                random=rand()%3;
                                objeto.x=rato.colunaAnterior*24;
                                objeto.y=rato.linhaAnterior*24;
                                SDL_BlitScaled(chao, NULL, superficie, &objeto);
                                objeto.x=rato.coluna*24;
                                objeto.y=rato.linha*24;
                                SDL_BlitScaled(chao, NULL, superficie, &objeto);
                                objeto.x=rato.coluna*24;
                                objeto.y=linha;
                                switch(random){
                                case 0:
                                    SDL_BlitScaled(ratob[0], NULL, superficie, &objeto);
                                    break;
                                case 1:
                                    SDL_BlitScaled(ratob[1], NULL, superficie, &objeto);
                                    break;
                                case 2:
                                    SDL_BlitScaled(ratob[2], NULL, superficie, &objeto);
                                    break;
                                }
                                SDL_UpdateWindowSurface(janela);
                                SDL_Delay(delay);
                            }
                            break;
                        case 3:
                            for(coluna=rato.colunaAnterior*24; coluna>rato.coluna*24; coluna--){
                                random=rand()%3;
                                objeto.x=rato.colunaAnterior*24;
                                objeto.y=rato.linhaAnterior*24;
                                SDL_BlitScaled(chao, NULL, superficie, &objeto);
                                objeto.x=rato.coluna*24;
                                objeto.y=rato.linha*24;
                                SDL_BlitScaled(chao, NULL, superficie, &objeto);
                                objeto.x=coluna;
                                objeto.y=rato.linha*24;
                                switch(random){
                                case 0:
                                    SDL_BlitScaled(ratoe[0], NULL, superficie, &objeto);
                                    break;
                                case 1:
                                    SDL_BlitScaled(ratoe[1], NULL, superficie, &objeto);
                                    break;
                                case 2:
                                    SDL_BlitScaled(ratoe[2], NULL, superficie, &objeto);
                                    break;
                                }
                                SDL_UpdateWindowSurface(janela);
                                SDL_Delay(delay);
                            }
                            break;
                        }
                    }
                    //imprimeLabirinto(labirinto, rato, labirinto2);//imprimo só os lab1 != lab2
                    for(linha=0; linha<MAXIJ; linha++){//anda o rato e joga algo atras
                        for(coluna=0; coluna<MAXIJ; coluna++){
                            if(labirinto[linha][coluna]!=labirinto2[linha][coluna]){
                                if(labirinto[linha][coluna]==3){//se o rato ja tiver passado
                                    objeto.y=linha*24;
                                    objeto.x=coluna*24;
                                    SDL_BlitScaled(chao, NULL, superficie, &objeto);
                                    SDL_BlitScaled(beco, NULL, superficie, &objeto);//desenha beco
                                }else{// se ja tiver passado
                                    objeto.y=rato.linhaAnterior*24;
                                    objeto.x=rato.colunaAnterior*24;
                                    SDL_BlitScaled(chao, NULL, superficie, &objeto);
                                    SDL_BlitScaled(visitada, NULL, superficie, &objeto);//desenha visitada
                                }
                            }
                        }
                    }
                    SDL_UpdateWindowSurface(janela);//Atualiza janela
                    while(SDL_PollEvent(&evento)!=0)
                        if(evento.type==SDL_QUIT)
                            opcao=-1;
                        else if(evento.type==SDL_KEYDOWN){
                            switch(evento.key.keysym.sym){
                            case SDLK_BACKSPACE:
                                opcao=-2;
                                break;
                            }
                        }
                }while((opcao!=0)&&(opcao!=-1)&&(opcao!=-2));
                libera_lista(&P);
                if((opcao!=-1)&&(opcao!=-2)){
                    opcao=1;
                    do{
                        while( SDL_PollEvent( &evento ) != 0 ){
                            if( evento.type == SDL_QUIT )
                                opcao=-1;
                            else if( evento.type == SDL_KEYDOWN ){
                                switch( evento.key.keysym.sym ){
                                default:
                                    opcao=0;
                                    jogar=1;
                                    prim=0;
                                    break;
                                }
                            }
                        }
                    }while((opcao!=0)&&(opcao!=-1));
                }
            }else if(opcao==2){//CONFIG
                jogar=3;
                do{
                    objeto.h=20;
                    objeto.w=100;
                    if(jogar==3){//velo
                        SDL_BlitScaled(config, NULL, superficie, NULL);
                        objeto.x=40;//1
                        objeto.y=80;
                        SDL_BlitScaled(seta1, NULL, superficie, &objeto);
                        objeto.x=590;
                        objeto.y=80;//2
                        SDL_BlitScaled(seta1, NULL, superficie, &objeto);
                    }else if(jogar==2){//animal
                        SDL_BlitScaled(config, NULL, superficie, NULL);
                        objeto.x=130;
                        objeto.y=280;
                        SDL_BlitScaled(seta1, NULL, superficie, &objeto);
                        objeto.x=500;
                        objeto.y=280;
                        SDL_BlitScaled(seta1, NULL, superficie, &objeto);
                    }else if(jogar==1){//piso
                        SDL_BlitScaled(config, NULL, superficie, NULL);
                        objeto.x=180;
                        objeto.y=450;
                        SDL_BlitScaled(seta1, NULL, superficie, &objeto);
                        objeto.x=450;
                        objeto.y=450;
                        SDL_BlitScaled(seta1, NULL, superficie, &objeto);
                    }else{
                        SDL_BlitScaled(config, NULL, superficie, NULL);
                        objeto.x=100;
                        objeto.y=620;
                        SDL_BlitScaled(seta1, NULL, superficie, &objeto);
                        objeto.x=500;
                        objeto.y=620;
                        SDL_BlitScaled(seta1, NULL, superficie, &objeto);
                    }
                    objeto.h=100;
                    objeto.w=100;
                    objeto.y=128;
                    if(udt==1){
                        objeto.x=200;
                        SDL_BlitScaled(seta2, NULL, superficie, &objeto);
                    }else if(udt==2){
                        objeto.x=300;
                        SDL_BlitScaled(seta2, NULL, superficie, &objeto);
                    }else{
                        objeto.x=400;
                        SDL_BlitScaled(seta2, NULL, superficie, &objeto);
                    }
                    objeto.y=330;
                    if(imgrato==1){
                        objeto.x=200;
                        SDL_BlitScaled(seta2, NULL, superficie, &objeto);
                    }else if(imgrato==2){
                        objeto.x=275;
                        SDL_BlitScaled(seta2, NULL, superficie, &objeto);
                    }else if(imgrato==3){
                        objeto.x=360;
                        SDL_BlitScaled(seta2, NULL, superficie, &objeto);
                    }else{
                        objeto.x=430;
                        SDL_BlitScaled(seta2, NULL, superficie, &objeto);
                    }
                    objeto.y=492;
                    if(imgpiso==1){
                        objeto.x=240;
                        SDL_BlitScaled(seta2, NULL, superficie, &objeto);
                    }else if(imgpiso==2){
                        objeto.x=310;
                        SDL_BlitScaled(seta2, NULL, superficie, &objeto);
                    }else{
                        objeto.x=380;
                        SDL_BlitScaled(seta2, NULL, superficie, &objeto);
                    }
                    SDL_UpdateWindowSurface(janela);
                    while( SDL_PollEvent( &evento ) != 0 ){
                        if( evento.type == SDL_QUIT )
                            opcao=-1;
                        else if( evento.type == SDL_KEYDOWN ){
                            Mix_PlayChannel( -1, somIntro, 0 );
                            switch( evento.key.keysym.sym ){
                            case SDLK_UP:
                                if(jogar+1<4)
                                    jogar++;
                                break;
                            case SDLK_DOWN:
                                if(jogar-1>-1)
                                    jogar--;
                                break;
                            case SDLK_KP_ENTER:
                                if(jogar==0)
                                    opcao=5;
                                break;
                            case SDLK_RETURN:
                                if(jogar==0)
                                    opcao=5;
                                break;
                            case SDLK_LEFT:
                                if(jogar==3){
                                    if(udt-1>0)
                                        udt--;
                                }else if(jogar==2){
                                    if(imgrato-1>0)
                                        imgrato--;
                                }else if(jogar==1){
                                    if(imgpiso-1>0)
                                        imgpiso--;
                                }
                                break;
                            case SDLK_RIGHT:
                                if(jogar==3){
                                    if(udt+1<4)
                                        udt++;
                                }else if(jogar==2){
                                    if(imgrato+1<5)
                                        imgrato++;
                                }else if(jogar==1){
                                    if(imgpiso+1<4)
                                        imgpiso++;
                                }
                                break;
                            }
                        }
                    }
                }while((opcao!=5)&&(opcao!=-1));
                jogar=2;
            }
        }while((opcao!=0)&&(opcao!=-1));
        if(opcao==-1)
            opcao=-2;
    }while((opcao!=-2)&&(opcao!=-1));
    SDL_FreeSurface(parede);
    SDL_FreeSurface(visitada);
    SDL_FreeSurface(beco);
    SDL_FreeSurface(menu);
    SDL_FreeSurface(config);
    SDL_FreeSurface(seta1);
    SDL_FreeSurface(seta2);
    SDL_FreeSurface(intro);
    SDL_FreeSurface(ganhou);
    SDL_FreeSurface(perdeu);
    Mix_FreeMusic(musicaIngame);
    Mix_FreeMusic(musicaMenu);
    Mix_FreeChunk(somIntro);
    if(umplay==1){
        for(linha=0; linha<3; linha++){
            SDL_FreeSurface(ratoc[linha]);
            SDL_FreeSurface(ratod[linha]);
            SDL_FreeSurface(ratob[linha]);
            SDL_FreeSurface(ratoe[linha]);
        }
        SDL_FreeSurface(saida);
        SDL_FreeSurface(chao);
    }
    SDL_DestroyWindow(janela);//finaliza a janela e a superficie dela
    IMG_Quit();
    SDL_Quit();
    Mix_Quit();
    gotoXY(0, 30);
    return 0;
}
