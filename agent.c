#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "agent.h"

static const int dR[4] = {-1, 1, 0, 0};
static const int dC[4] = { 0, 0,-1, 1};

agent newAgent(enviroment E){
    agent A;
    A.score = 0;
    A.temFlecha = true;
    A.comOuro = false;
    A.onde = &E.grid[0][0];
    A.h = E.h;
    A.w = E.w;

    int i, j;
    A.visited = malloc(E.h*sizeof(bool*));
    A.cheiro = malloc(E.h*sizeof(bool*));
    A.vento = malloc(E.h*sizeof(bool*));
    A.buracoConhecido = malloc(E.h*sizeof(bool*));
    for (i=0; i<E.h; i++){
        A.visited[i] = malloc(E.w*sizeof(bool));
        A.cheiro[i] = malloc(E.w*sizeof(bool));
        A.vento[i] = malloc(E.w*sizeof(bool));
        A.buracoConhecido[i] = malloc(E.w*sizeof(bool));
        for (j=0; j<E.w; j++){
            A.visited[i][j] = false;
            A.cheiro[i][j] = false;
            A.vento[i][j] = false;
            A.buracoConhecido[i][j] = false;
        }
    }
    return A;
}

void delAgent(agent* A){
    int i;
    for (i=0; i<A->h; i++){
        free(A->visited[i]);
        free(A->cheiro[i]);
        free(A->vento[i]);
        free(A->buracoConhecido[i]);
    }
    free(A->visited);
    free(A->cheiro);
    free(A->vento);
    free(A->buracoConhecido);
}

void sense(agent* A){
/*
    Registra, na base de conhecimento do agente, as sensações do local
    atual. Isto é o único jeito pelo qual o agente aprende algo sobre o
    ambiente: nunca lemos E.grid[i][j].buraco/monstro diretamente aqui.
*/
    int i = A->onde->row, j = A->onde->col;
    A->visited[i][j] = true;
    A->cheiro[i][j] = A->onde->S.cheiro;
    A->vento[i][j] = A->onde->S.vento;
    if (A->onde->buraco){
        A->buracoConhecido[i][j] = true;
    }
}

bool isSafe(agent A, int i, int j){
/*
    Um local (i,j) é seguro se existe, entre seus vizinhos já
    visitados, evidência suficiente para descartar tanto buraco quanto
    monstro: um vizinho sem vento descarta buraco em todos os SEUS
    vizinhos (logo também em (i,j)); um vizinho sem cheiro descarta
    monstro pelo mesmo motivo. As duas evidências podem vir de
    vizinhos diferentes.
*/
    bool semBuracoPossivel = false;
    bool semMonstroPossivel = false;
    int k;
    for (k=0; k<4; k++){
        int ni = i+dR[k], nj = j+dC[k];
        if (ni<0 || ni>=A.h || nj<0 || nj>=A.w) continue;
        if (!A.visited[ni][nj]) continue;
        if (!A.vento[ni][nj]) semBuracoPossivel = true;
        if (!A.cheiro[ni][nj]) semMonstroPossivel = true;
    }
    return semBuracoPossivel && semMonstroPossivel;
}

static int riskScore(agent A, int i, int j){
/*
    Quando não há nenhum local seguro ainda não visitado, o agente
    precisa arriscar. Esta função ordena os candidatos por risco:
    0 = seguro; risco de monstro é preferível ao risco de buraco, pois
    um monstro pode ser abatido com a flecha (custo -10), enquanto cair
    num buraco é sempre muito pior (-1000).
*/
    bool semBuracoPossivel = false;
    bool semMonstroPossivel = false;
    int k;
    for (k=0; k<4; k++){
        int ni = i+dR[k], nj = j+dC[k];
        if (ni<0 || ni>=A.h || nj<0 || nj>=A.w) continue;
        if (!A.visited[ni][nj]) continue;
        if (!A.vento[ni][nj]) semBuracoPossivel = true;
        if (!A.cheiro[ni][nj]) semMonstroPossivel = true;
    }
    if (semBuracoPossivel && semMonstroPossivel) return 0;
    if (semBuracoPossivel) return A.temFlecha ? 1 : 3;
    if (semMonstroPossivel) return 5;                     
    return A.temFlecha ? 2 : 4;                            
}

static int bfsAteVizinhoDoAlvo(agent A, enviroment E, int gr, int gc, place** caminho){
/*
    Busca em largura restrita a células já visitadas (e sem buraco
    conhecido), terminando ao alcançar (gr,gc). Preenche 'caminho' com
    a sequência de células a percorrer (do primeiro passo até o alvo
    incluso) e retorna o número de passos, ou -1 se inalcançável.
*/
    int h = E.h, w = E.w, n = h*w;
    int sr = A.onde->row, sc = A.onde->col;
    int startIdx = sr*w+sc, goalIdx = gr*w+gc;
    int idx, k, i;

    int* parent = malloc(n*sizeof(int));
    bool* seen = malloc(n*sizeof(bool));
    int* fila = malloc(n*sizeof(int));
    for (idx=0; idx<n; idx++){ parent[idx] = -1; seen[idx] = false; }

    int head=0, tail=0;
    fila[tail++] = startIdx;
    seen[startIdx] = true;
    bool achou = (startIdx==goalIdx);

    while (head<tail && !achou){
        int cur = fila[head++];
        int r = cur/w, c = cur%w;
        for (k=0; k<4 && !achou; k++){
            int nr = r+dR[k], nc = c+dC[k];
            if (nr<0 || nr>=h || nc<0 || nc>=w) continue;
            int nidx = nr*w+nc;
            if (seen[nidx]) continue;
            bool ehAlvo = (nidx==goalIdx);
            if (!ehAlvo && (!A.visited[nr][nc] || A.buracoConhecido[nr][nc])) continue;
            seen[nidx] = true;
            parent[nidx] = cur;
            fila[tail++] = nidx;
            if (ehAlvo) achou = true;
        }
    }

    int passos = -1;
    if (achou && startIdx!=goalIdx){
        int* pilha = malloc(n*sizeof(int));
        int cnt = 0;
        int cur = goalIdx;
        while (cur!=startIdx){
            pilha[cnt++] = cur;
            cur = parent[cur];
        }
        for (i=0; i<cnt; i++){
            int pIdx = pilha[cnt-1-i];
            caminho[i] = &E.grid[pIdx/w][pIdx%w];
        }
        passos = cnt;
        free(pilha);
    } else if (achou){
        passos = 0;
    }

    free(parent); free(seen); free(fila);
    return passos;
}

int runEpisode(enviroment E, int maxMoves, bool verbose, bool* sucesso){
    agent A = newAgent(E);
    int moves = 0;
    int k, r, c, s;
    *sucesso = false;
    place** caminho = malloc(E.h*E.w*sizeof(place*));

    while (moves < maxMoves){
        sense(&A);
        int i = A.onde->row, j = A.onde->col;

        if (verbose){
            printSimulation(A, E);
            printf("Score atual: %d\n", A.score);
            if (A.vento[i][j]) printf("Aqui está batendo um vento estranho.\n");
            if (A.cheiro[i][j]) printf("Aqui há um cheiro monstruoso.\n");
            for (k=0; k<4; k++){
                int ni = i+dR[k], nj = j+dC[k];
                if (ni<0 || ni>=E.h || nj<0 || nj>=E.w) continue;
                if (A.visited[ni][nj]) continue;
                printf("  Posição (%d,%d) inferida como %s.\n",
                    ni, nj, isSafe(A,ni,nj) ? "SEGURA" : "arriscada");
            }
        }

        if (A.comOuro && i==E.h-1 && j==E.w-1){
            *sucesso = true;
            break;
        }

        // Procura, entre as células de fronteira (não visitadas, mas
        // adjacentes a alguma célula já visitada), a de menor risco;
        // em caso de empate, a mais próxima da posição atual.
        int bestI=-1, bestJ=-1, bestRisk=999, bestDist=999999;
        for (r=0; r<E.h; r++){
            for (c=0; c<E.w; c++){
                if (A.visited[r][c]) continue;
                bool fronteira = false;
                for (k=0; k<4; k++){
                    int nr = r+dR[k], nc = c+dC[k];
                    if (nr>=0 && nr<E.h && nc>=0 && nc<E.w && A.visited[nr][nc]){
                        fronteira = true;
                        break;
                    }
                }
                if (!fronteira) continue;
                int risco = riskScore(A, r, c);
                int dist = abs(r-i) + abs(c-j);
                if (risco<bestRisk || (risco==bestRisk && dist<bestDist)){
                    bestRisk = risco; bestDist = dist; bestI = r; bestJ = c;
                }
            }
        }

        if (bestI==-1) break;

        int passos = bfsAteVizinhoDoAlvo(A, E, bestI, bestJ, caminho);
        if (passos<=0) break;

        for (s=0; s<passos && moves<maxMoves; s++){
            move(&A, E, caminho[s]);
            moves++;
        }
    }

    free(caminho);
    int scoreFinal = A.score;
    delAgent(&A);
    return scoreFinal;
}

bool move(agent* A, enviroment E, place* target){
/*
    Movimenta o agente para a posição place, desde que esta seja uma vizinha de
    cleaner.ondeCleaner e que a bateria de C não esteja vazia.
*/
    if (isNeighbor(*A->onde,*target)){
        A->onde = target;
        A->score--;
        if (A->onde->monstro){
            if (A->temFlecha){
                printf("Matou o monstro. \n");
                A->score -= 10;
                A->onde->monstro = false;
            }
            else{
                printf("Pego pelo monstro.\n");
                A->score -= 1000;
            }
        }
        if (A->onde->buraco){
            printf("Caiu no buraco. \n");
            A->score -= 1000;
        }
        if (A->onde->ouro){
            printf("O ouro está aqui! \n");
            A->comOuro = true;
        }
        if (A->comOuro && A->onde->row==E.h-1 && A->onde->col==E.w-1){
            printf("Escapou com o ouro! Parabéns. \n");
        }
        return true;
    }
    return false;
}

void printSimulation(agent A, enviroment E){
    int i, j;
    for (i=0; i<E.h; i++){
        for (j=0; j<E.w; j++){
            if (&E.grid[i][j]==A.onde){
                printf("O ");
            }
            else{
                printf("_ ");
            }
        }
        printf("\n");
    }
}
