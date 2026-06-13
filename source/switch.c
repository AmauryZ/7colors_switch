#include "../head/GameState.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <switch.h>

char* showKeyboard(const char* message, char* mode, size_t size);

GameState state = {.map = NULL, .size = 0};

void create_empty_game_state (GameState* state, int size){
	state->size = size;
    state->map = malloc(size * size * sizeof(Color)); //pour creer un tableau contenant les coordonnées et les valeurs de chaque case
}

void fill_map(GameState* state){
    int size=state->size; //on recup size
    for (int i = 0; i < size*size; i++) 
    {
        state->map[i] = (rand() % 7)+3; //on attribut une couleur aléatoire à chaque case
    }

    //On définit la case de départ de chaque joueur
    set_map_value(state, 0, size-1, PLAYER_1);  //PLAYER_1 en bas à gauche
    set_map_value(state, size-1, 0, PLAYER_2);  //PLAYER_2 en haut à droite
}

void set_map_value (GameState* state, int x, int y, Color value){
	state->map[y * state->size + x] = value; //on selectionne la case via ses coordonnées et on change la valeur
}

Color get_map_value (GameState* state, int x, int y){
	if (state->map == NULL || x >= state->size || y >= state->size || x < 0 || y < 0) //On vérifie que la case demandée existe
	{
		printf("[ERROR] map not big enough or not initialized %p %i access (%i %i)", state -> map, state -> size, x, y);
		return ERROR;
	}
	return state -> map[y * state -> size + x];
}

void afficher(GameState* state) { //réalise un affichage graphique dans le terminal en couleurs
    const char* colors[] = { //On définit une liste des couleurs possibles pour les cases
        "\x1b[91m", // PLAYER_1 (rouge clair)
        "\x1b[94m",  // PLAYER_2 (bleu clair)
        "\x1b[31m", // RED
        "\x1b[32m", // GREEN
        "\x1b[34m", // BLUE
        "\x1b[33m", // YELLOW
        "\x1b[35m", // MAGENTA
        "\x1b[36m", // CYAN
        "\x1b[37m" // WHITE
        
    };

    char letters[] = {'X','O','A','B','C','D','E','F','G',}; //On définit une liste des affichages possibles pour les cases. X et O réservés respectivement aux joueurs 1 et 2
    const char* reset = "\x1b[0m"; //pour reset couleur

    for (int y = 0; y < state->size; y++) {
        for (int x = 0; x < state->size; x++) { //On parcourt le plateau
            Color c = get_map_value(state, x, y); //On récupère la valeur de la case
            if(c >= 1 && c < 10) { //si elle figure parmis les valeurs possibles
                printf("%s%c %s", colors[c-1], letters[c-1], reset); //On change la couleur, on print la lettre associée et on reset la couleur
            } else {
                printf("? ");
            }
        }
        printf("\n");
    }
}


int entree(int joueur) { //fonction permettant d'entrer des coups. B retourne 1 par exemple
    PadState pad;
    padInitializeDefault(&pad);
    printf("\nPlayer %d, enter a letter (A-G): A, B, Y, X, <, ^, > \n", joueur == PLAYER_1 ? 1 : 2); //condition ? valeur si vrai : valeur si faux
    consoleUpdate(NULL);
    int val=-1;
    while (val==-1) 
    {
        padUpdate(&pad);
        u64 kUp = padGetButtonsUp(&pad);
        if (kUp & HidNpadButton_A)
            val = 0;
        else if  (kUp & HidNpadButton_B)
            val = 1;
        else if (kUp & HidNpadButton_Y)
            val = 2;
        else if (kUp & HidNpadButton_X)
            val = 3;
        else if (kUp & HidNpadButton_Left )
            val = 4;
        else if (kUp & HidNpadButton_Up )
            val = 5;
        else if (kUp & HidNpadButton_Right )
            val = 6;
        else if (kUp)
        {
            printf("Veuillez appuyer sur une touche valide\n");
            consoleUpdate(NULL);
        }
    }
    return val;
}



void tour_basique(GameState* state, int joueur, int x) { //fonction qui met à jour le monde après le coup d'un joueur
    int size = state->size;//on récup size
    int dx[4] = {1, -1, 0, 0};//On crée des variables dx et dy pour regarder les cases autour
    int dy[4] = {0, 0, 1, -1};
    int changed=1;

    while (changed)
    {
        changed=0;
        for(int j = 0; j < size; j++) {
            for(int i = 0; i < size; i++) {//On parcourt le plateau
                if(state->map[j * size + i] == x + 3) {//si la valeur de la case correspond au coup joué
                    //on vérifie si un voisin appartient au joueur
                    for(int k = 0; k < 4; k++) {
                        int ni = i + dx[k];
                        int nj = j + dy[k];

                        if(ni >= 0 && ni < size && nj >= 0 && nj < size) { //si les coordonnées de la case la font figurer sur notre plateau de jeu
                            if(state->map[nj * size + ni] == joueur) { //si la case appartient au joueur
                                state->map[j * size + i] = joueur;//on change la valeur de la case à la valeur du joueur (i.e. on conquiert la case)
                                changed = 1; 
                            }
                        }
                    }
                }
            }
        }
    }
}

void tour(GameState* state, int joueur, int x) { //fonction tour_basique améliorée

    typedef struct {//on définit une structure position prenant les x,y
        int x;
        int y;
    } Position;

    int size = state->size;//on récup size

    int capacity = size*size;
    int count = 0;
    Position* to_change = malloc(capacity * sizeof(Position));//On crée une liste de positions des cases du joueur

    for(int j = 0; j < size; j++) {
        for(int i = 0; i < size; i++) {//On parcourt le plateau
            if(state->map[j * size + i] == joueur) {//si la case appartient au joueur
                
                to_change[count].x = i;//On ajoute les coordonnées x et y de la case
                to_change[count].y = j;
                count++;
            }
        }
    }

    int processed = 0;

    int dx[4] = {1, -1, 0, 0};//On crée des variables dx et dy pour regarder les cases autour
    int dy[4] = {0, 0, 1, -1};

    while(processed < count) {
        int i = to_change[processed].x;//On passe par toutes les positions appartenant à notre liste
        int j = to_change[processed].y;

        

        for(int k = 0; k < 4; k++) {
            int ni = i + dx[k];
            int nj = j + dy[k];

            if(ni >= 0 && ni < size && nj >= 0 && nj < size) {//si les coordonnées de la case la font figurer sur notre plateau de jeu
                if(state->map[nj * size + ni] == x+3) {//si la valeur de la case correspond au coup joué
                    state->map[nj * size + ni] = joueur;//on change la valeur de la case à la valeur du joueur

                    to_change[count].x = ni;//on ajoute la case à la liste des positions du joueur(et donc celles à tester)
                    to_change[count].y = nj;
                    count++;
                }
            }
        }
        processed++; //on a fini de regarder les voisins. La case a été traitée
    }

    free(to_change);//on libere la liste
}

int compter_victoire(GameState* state, int joueur){//Fonction qui dit si le joueur en train de jouer a gagné. On entre la map et le joueur en train de jouer

    int count=0;
    int size=state->size;

    for(int i=0; i<size*size; i++)//On compte le nombre de cases au joueur
        if(state->map[i]==joueur)
            count++;

    if(count >= (size*size)/2)//si le compte est sup à la moitié du tableau
        return 1;//c'est win

    return 0;
} 

int choisir_mode() {

    char mode[50]; //on def notre variable mode de jeu
    PadState pad;
    padInitializeDefault(&pad);

    while(1) 
    {

        printf("\nChoose a game mode (press A to show the keyboard):\n");
        printf("Randy (random)\nRandy_Jr (improved random)\nBouboule (greedy)\nHegemonic\nHegemonic_Jr (hegemonic-greedy mix)\nBoyard (farthest cell)\nVersus (bot vs bot)\nSuper versus (versus 10 times)\nPvP\n\n");//liste des modes possibles
        consoleUpdate(NULL);

        while (1) 
        {
            
            padUpdate(&pad);
            u64 kDown = padGetButtonsDown(&pad);
            if (kDown & HidNpadButton_A)
                break;
            else if (kDown)
            {
                printf("Please press a valid key\n");
                consoleUpdate(NULL);
            }
        }
        

        if (showKeyboard("Enter a mode name", mode, sizeof(mode)) != NULL && strcmp(mode, "") != 0) 
        {
        if(strcmp(mode,"Versus")==0) return 0;//un bot contre un bot
        if(strcmp(mode,"Super versus")==0) return 1;//un bot contre un bot 500 fois
        if(strcmp(mode,"PvP")==0) return 2;//utilisateur contre utilisateur
        if(strcmp(mode,"Randy")==0) return 3;//random total
        if(strcmp(mode,"Randy_Jr")==0) return 4;//random parmi les coups possibles
        if(strcmp(mode,"Bouboule")==0) return 5;//Le plus de cases en un coup
        if(strcmp(mode,"Hegemonic")==0) return 6;//le plus de frontières en un coup
        if(strcmp(mode,"Boyard")==0) return 7;//La case la plus lointaine possible
        if(strcmp(mode,"Hegemonic_Jr")==0) return 8;//le plus de frontières en un coups, et le plus de case possibles lorque plus aucun coup ne fait augmenter les frontières
        }
        
        printf("Please enter a valid mode name\n");//si la commande ne figure pas parmis les options données
    }
}

int choisir_bot() {//si le mode versus est selectionné. Meme principe que choisir_mode()

    char mode[50];
    PadState pad;
    padInitializeDefault(&pad);

    while(1) {

        printf("\nChoose a bot (press A to show the keyboard):\n");
        printf("Randy (random)\nRandy_Jr (improved random)\nBouboule (greedy)\nHegemonic\nHegemonic_Jr (hegemonic-greedy mix)\nBoyard (farthest cell)\n\n");
        consoleUpdate(NULL);

        while (1) 
        {
            padUpdate(&pad);
            u64 kDown = padGetButtonsDown(&pad);
            if (kDown & HidNpadButton_A)
                break;
            else if (kDown)
            {
                printf("Please press a valid key\n");
                consoleUpdate(NULL);
            }
        }

        if (showKeyboard("Enter a bot name", mode, sizeof(mode)) != NULL && strcmp(mode, "") != 0) 
        {
        if(strcmp(mode,"Randy")==0) return 0;
        if(strcmp(mode,"Randy_Jr")==0) return 1;
        if(strcmp(mode,"Bouboule")==0) return 2;
        if(strcmp(mode,"Hegemonic")==0) return 3;
        if(strcmp(mode,"Boyard")==0) return 4;
        if(strcmp(mode,"Hegemonic_Jr")==0) return 5;
        }

        printf("Invalid bot\n");
    }
}

int randy(GameState* state, int joueur){
    return (rand() % 7);//On retourne un nombre random entre 0 et 6
}

int randy_jr(GameState* state, int joueur) {
    int size = state->size;//on récup size
    int L[7];//On crée une liste de taille 7 dont les indices correspondent au valeurs possibles des cases 
    int n = 0;//nombre d'éléments dans L

    for(int i=0; i<size;i++){
        for(int j=0; j<size;j++){//On parcourt le plateau

            if(get_map_value(state, i, j)==joueur){//si la case appartient au joueur

                if(i<size-1){//si la case est pas sur le bord droit du plateau
                    int v=get_map_value(state, i+1, j);//on regarde la case à droite de notre case selectionnée
                    int present=0;
                    for(int k=0; k<n; k++) //On regarde si la valeur de la case figure déjà parmi les valeurs comptabilisées dans la liste
                        if(L[k]==v)
                            present=1;
                            
                    if(!present && v!=PLAYER_1 && v!=PLAYER_2) 
                        L[n++]=v;//si pas présent on ajoute à la position n et on incrémente n
                }

                if(i>0){//si la case est pas sur le bord gauche du plateau
                    int v=get_map_value(state, i-1, j);
                    int present=0;
                    for(int k=0; k<n; k++) 
                        if(L[k]==v) 
                            present=1;
                            
                    if(!present && v!=PLAYER_1 && v!=PLAYER_2) 
                        L[n++]=v;
                }

                if(j<size-1)//si la case n'est pas sur le bord inférieur du plateau
                {
                    int v=get_map_value(state, i, j+1);
                    int present=0;
                    for(int k=0; k<n; k++) 
                        if(L[k]==v) 
                            present=1;
                            
                    if(!present && v!=PLAYER_1 && v!=PLAYER_2) 
                        L[n++]=v;
                }

                if(j>0){//si la case est pas sur le bord supérieur du plateau
                    int v=get_map_value(state, i, j-1);
                    int present=0;
                    for(int k=0; k<n; k++) 
                        if(L[k]==v) 
                            present=1;
                            
                    if(!present && v!=PLAYER_1 && v!=PLAYER_2) 
                        L[n++]=v;
                }

            }
        }
    }
        
    return L[rand()%n]-3;//on renvoit une VA parmis L (comprise entre 0 et 6)
}

int Bouboule(GameState* state, int joueur)
{
    int size = state->size;
    int bestColor = 0; 
    int maxCaptured = -1;
    typedef struct { 
        int x; 
        int y; 
    } Position;
    
    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1}; //On définit une liste de variation d'une case autour de notre case séléctionnée

    for(int color = 0; color < 7; color++)//pour chaque couleur
    {
        
        Color* tmpMap = malloc(size * size * sizeof(Color));
        memcpy(tmpMap, state->map, size * size * sizeof(Color));// On crée une copie temporaire de la map pour simuler le coup (on copie state->map dans tmpMap)

        
        Position* to_change = malloc(size * size * sizeof(Position));//On crée une liste de positions
        int count = 0;

        for(int j = 0; j < size; j++)
        {
            for(int i = 0; i < size; i++)
            {
                if(tmpMap[j*size + i] == joueur)
                {
                    to_change[count].x = i;
                    to_change[count].y = j;//On y ajoute toute les cases actuelles du joueur
                    count++;
                }
            }
        }

        int processed = 0;


        while(processed < count) //tant qu'on n'a pas traité toutes les cases de la liste
        { 
            int i = to_change[processed].x;
            int j = to_change[processed].y;//On prend les coordonnées de chaque élément de la liste

            for (int k = 0; k < 4; k++)
            {
                int ni = i + dx[k];//On regarde pour chaque case autour de notre case sélectionnée
                int nj = j + dy[k];

                if(ni >= 0 && ni < size && nj >= 0 && nj < size)//si la case se situe bien dans dans notre plateau de jeu
                { 
                    if(tmpMap[nj*size + ni] == color+3)//si la case est bien de la couleur simulée
                    {
                        tmpMap[nj*size + ni] = joueur;//on change la valeur de la case à la valeur du joueur (sur notre carte temporaire pour ne pas tomber dans une boucle infinie)
                        to_change[count].x = ni;//On ajoute les coordonnées de notre case dans notre liste de cases à modifier 
                        to_change[count].y = nj;
                        count++;
                    }
                }
            }
        processed++;
        }

        int captured = 0;
        for(int i = 0; i < size*size; i++)//on compte le nombre de cases totales capturées
        {
            if(tmpMap[i] == joueur)
            {
                captured++;
            }
        }
        

        if(captured > maxCaptured)//si le compte est plus grand que le compte max enregistré jusqu'ici, on remplace celui-ci et on retient la couleur
        {
            maxCaptured = captured;
            bestColor = color;
        }

        free(tmpMap); //on libère nos malloc
        free(to_change);

    }
    
    return (bestColor);//On renvoie la couleur retenue. Si glouton ne peut plus faire progresser son territoire, il va jouer la lettre A (couleur rouge) par défaut
}

int Hegemonic(GameState* state, int joueur)
{
    int size = state->size;
    int bestColor = 0;
    int maxFront = 0;//On recup les variables et on crée celles dont on a besoin

    for(int color = 0; color < 7; color++)//Pour chaque couleur
    {
        Color* tmpMap = malloc(size*size*sizeof(Color));
        memcpy(tmpMap, state->map, size*size*sizeof(Color));//On réalise une copie de la carte

        typedef struct { 
            int x;
            int y; 
        } Position;

        Position* to_change = malloc(size*size*sizeof(Position));//On crée une liste des position à vérifier
        Position* front_list = malloc(size*size*sizeof(Position)); //On crée une liste de positions repérées des fronitères pour ne pas compter des cases frontalières plusieurs fois
        int count = 0;//Variable de compte du nombre de case dans to_change
        int front_count = 0; //Variable de compte du nombre de cases frontalières rencontrées

        // Ajouter toutes les cases du joueur
        for(int j=0; j<size; j++)
        {
            for(int i=0; i<size; i++)
            {
                if(tmpMap[j*size + i] == joueur)//On ajoute à cette liste toutes les cases du joueur
                {
                    to_change[count].x = i;
                    to_change[count].y = j;
                    count++;
                }
            }
        }

        int dx[4] = {1,-1,0,0};//On définit des variations de position
        int dy[4] = {0,0,1,-1};

        int processed = 0;

        while(processed < count){
            int i = to_change[processed].x;//Pour chaque case de notre liste to_change
            int j = to_change[processed].y;

            for(int k=0;k<4;k++)
            {
                int ni = i+dx[k];//On regarde les 4 cases autour
                int nj = j+dy[k];
                if(ni>=0 && ni<size && nj>=0 && nj<size)//Si elles sont bien sur le plateau
                {
                    if(tmpMap[nj*size + ni] == color+3)//Si elle correspondent à la couleur qu'on simule
                    {
                        for(int l=0;l<4;l++)//On regarde les 4 cases autour de notre case alentour (i.e les frontières de la couleur conquise)
                        {
                            int nni = ni+dx[l];
                            int nnj = nj+dy[l];
                            if (nni>=0 && nni<size && nnj>=0 && nnj<size)//Si elle est bien sur le plateau
                            {
                                if(tmpMap[nnj*size + nni] != joueur && tmpMap[nnj*size + nni] != color+3)//Si ce n'est ni une case du joueur ni une case de la couleur simulée
                                {
                                    int deja_visitee = 0; //on vérifie si la fronitère n'a pas déjà été visitée
                                    
                                    for (int m=0; m < front_count; m++)
                                    {
                                        if (front_list[m].x==nni && front_list[m].y==nnj)
                                        {
                                            deja_visitee=1;
                                            break;
                                        }
                                    }

                                    if (!deja_visitee) //si c'est bien le cas
                                    {
                                        front_list[front_count].x = nni; //on l'ajoute dans la liste des frontières visitées
                                        front_list[front_count].y = nnj;
                                        front_count++;//On augmente le compteur de frontière
                                    }
                                    
                                }
                            }
                        }
                    tmpMap[nj*size+ni]=joueur;//On en change la couleur
                    to_change[count].x=ni;//On la rajoute à nos cases à vérifier
                    to_change[count].y=nj;
                    count++;
                    }
                }
            }

            processed++;
        }

        if(front_count > maxFront)
        {
            maxFront = front_count;
            bestColor = color;
        }

        free(tmpMap);
        free(to_change);
        free(front_list);
    }
    printf("%i", maxFront);
    return bestColor; //si maxFront vaut 0, l'IA va jouer la lettre A (couleur rouge) par défaut
}

int Boyard(GameState* state, int joueur)
{
    int size = state->size; //On récup les variables dont on a besoin et on crée celles qu'on utilisera
    int bestColor = 0;
    double maxDistance = 0.0;

    typedef struct {
         int x; 
         int y; 
    } Position; //On définit une structure position

    Position* playerCells = malloc(size*size*sizeof(Position));//on crée une liste de positions
    int playerCount = 0;//Taille de notre liste

    for(int j = 0; j < size; j++){
        for(int i = 0; i < size; i++){
            if(get_map_value(state, i, j) == joueur){//On ajoute à notre liste toutes les positions des cases du joueur
                playerCells[playerCount].x = i;
                playerCells[playerCount].y = j;
                playerCount++;
            }
        }
    }
    
    int dx[4] = {1,-1,0,0};//On définit des variations de position autour d'une case séléctionnée
    int dy[4] = {0,0,1,-1};

    int adjacent[7] = {0}; //on crée un tableau initialisé à 0 pour dire quelles couleurs sont adjacentes au joueur

    for(int p = 0; p < playerCount; p++) //pour chaque case du joueur
    {
        int i = playerCells[p].x;
        int j = playerCells[p].y;

        for(int k = 0; k < 4; k++){ //on regarde ses voisins
            int ni = i + dx[k];
            int nj = j + dy[k];

            if(ni >= 0 && ni < size && nj >= 0 && nj < size)
            {
                int col = get_map_value(state, ni, nj)-3;
                if(col >= 0 && col < 7) //si c'est bien une couleur (hors joueurs)
                    adjacent[col] = 1; //on l'ajoute à la liste des couleurs adjacentes au joueur
            }
        }
    }

    for(int color = 0; color < 7; color++){ //Pour chaque couleur

        if(!adjacent[color])
            continue;

        Color* tmpMap = malloc(size*size*sizeof(Color));
        memcpy(tmpMap, state->map, size*size*sizeof(Color));//On crée une copie de la carte (tmpMap est une copie de state->map)

        Position* to_change = malloc(size*size*sizeof(Position));//On crée une liste des positions du joueur après avoir joué le coup
        int count = 0;//taille de la liste

        for(int i = 0; i < playerCount; i++){
            to_change[count] = playerCells[i];//On y ajoute toute les cases originelles du joueur
            count++;
        }

        int processed = 0;
        //////////////////////////////////
        //on simule la conquête de la case
        while(processed < count){
            int i = to_change[processed].x;//Pour chaque case dans notre liste to_change
            int j = to_change[processed].y;

            for(int k = 0; k < 4; k++){
                int ni = i + dx[k];//Pour chaque case autour de la case choisie
                int nj = j + dy[k];

                if(ni >= 0 && ni < size && nj >= 0 && nj < size){//si la case se trouve dans le plateau de jeu
                    if(tmpMap[nj*size + ni] == color+3)//si la case a la couleur jouée
                    {
                        tmpMap[nj*size + ni] = joueur;//On change la couleur de la case à celle du joueur et on rajoute sa position dans la liste to_change
                        to_change[count].x = ni;
                        to_change[count].y = nj;
                        count++;
                        
                    }
                }
            }
            processed++;
        }
        //////////////////////////////////

        // Calculer la distance maximale
        double farthest = 0.0; //On définit une variable 'case la plus lointaine'
        for(int j = 0; j < count; j++)//pour toute les cases dans notre liste de cases après conquête
        {
            double dist = 0;
            for(int p = 0; p < playerCount; p++)//Pour chaque case appartenant au joueur dès le début
            {
                int diffx = to_change[j].x - playerCells[p].x;
                int diffy = to_change[j].y - playerCells[p].y;
                dist += sqrt(diffx*diffx + diffy*diffy);
            }
            dist/=playerCount;//On calcule la distance moyenne de notre case capturée par rapport à toutes nos cases originelles
            if(dist > farthest) 
            {
                farthest = dist; //On garde la distance max au sein de la couleur
            }
        }

        //printf("couleur : %c, distance : %lf\n", "ABCDEFG"[color], farthest);

        if(farthest > maxDistance){
            maxDistance = farthest;//On garde la couleur avec la plus grande distance
            bestColor = color;
            
        }

        free(tmpMap);
        free(to_change);//On free les malloc
    }

    free(playerCells);
    printf("\nmaxdistance : %lf\n", maxDistance);
    return bestColor; //joue la lettre A (couleur rouge) s'il ne peut plus faire progresser son territoire 
}

int Hegemonic_Jr(GameState* state, int joueur)
{
    int size = state->size;
    int bestColor = 0;
    int maxFront = 0;//On recup les variables et on crée celles dont on a besoin

    for(int color = 0; color < 7; color++)//Pour chaque couleur
    {
        Color* tmpMap = malloc(size*size*sizeof(Color));
        memcpy(tmpMap, state->map, size*size*sizeof(Color));//On réalise une copie de la carte

        typedef struct { 
            int x;
             int y; 
        } Position;

        Position* to_change = malloc(size*size*sizeof(Position));//On crée une liste des positions à vérifier
        Position* front_list = malloc(size*size*sizeof(Position)); //On crée une liste de positions repérées des fronitères pour ne pas compter des cases frontalières plusieurs fois
        int count = 0;//Variable de compte du nombre de cases dans to_change
        int front_count = 0; //Variable de compte du nombre de cases frontalières rencontrées

        // Ajouter toutes les cases du joueur
        for(int j=0; j<size; j++)
        {
            for(int i=0; i<size; i++)
            {
                if(tmpMap[j*size + i] == joueur)//On ajoute à cette liste toutes les cases du joueur
                {
                    to_change[count].x = i;
                    to_change[count].y = j;
                    count++;
                }
            }
        }

        int dx[4] = {1,-1,0,0};//On définit des variation de position
        int dy[4] = {0,0,1,-1};

        int processed = 0;

        while(processed < count){
            int i = to_change[processed].x;//Pour chaque case de notre liste to_change
            int j = to_change[processed].y;

            for(int k=0;k<4;k++)
            {
                int ni = i+dx[k];//On regarde les 4 cases autour
                int nj = j+dy[k];
                if(ni>=0 && ni<size && nj>=0 && nj<size)//Si elles sont bien sur le plateau
                {
                    if(tmpMap[nj*size + ni] == color+3)//Si elle correspondent à la couleur qu'on simule
                    {
                        for(int l=0;l<4;l++)
                        {
                            int nni = ni+dx[l];//On regarde les 4 cases autour de notre case alentour
                            int nnj = nj+dy[l];
                            if (nni>=0 && nni<size && nnj>=0 && nnj<size)//Si elle est bien sur le plateau
                            {
                                if(tmpMap[nnj*size + nni] != joueur && tmpMap[nnj*size + nni] != color+3)//Si ce n'est ni une case du joueur ni une case de la couleur simulée
                                {
                                    int deja_visitee = 0;
                                    
                                    for (int m=0; m < front_count; m++)
                                    {
                                        if (front_list[m].x==nni && front_list[m].y==nnj)
                                        {
                                            deja_visitee=1;
                                            break;
                                        }
                                    }

                                    if (!deja_visitee)
                                    {
                                        front_list[front_count].x = nni;
                                        front_list[front_count].y = nnj;
                                        front_count++;//On augmente le compteur de frontière
                                    }
                                    
                                }
                            }
                        }
                    tmpMap[nj*size+ni]=joueur;//On en change la couleur
                    to_change[count].x=ni;//On la rajoute à nos cases à vérifier
                    to_change[count].y=nj;
                    count++;
                    }
                }
            }

            processed++;
        }

        if(front_count > maxFront)
        {
            maxFront = front_count;
            bestColor = color;
        }

        free(tmpMap);
        free(to_change);
        free(front_list);
    }

    printf("%i", maxFront);

    if(maxFront==0) //s'il n'est plus possible de faire progresser son territoire, on joue comme un glouton.
                    //S'il n'est encore pas possible de faire progresser son territoire avec glouton, l'IA va jouer la lettre A (couleur rouge) par défaut
    {
        bestColor=Bouboule(state,joueur); 
    }

    return bestColor;
}


int jouer_bot(int bot, GameState* state, int joueur) {//pour faire jouer les bots si le mode versus  est selectionné

    switch(bot) {

        case 0: return randy(state,joueur);
        case 1: return randy_jr(state,joueur);
        case 2: return Bouboule(state,joueur);
        case 3: return Hegemonic(state,joueur);
        case 4: return Boyard(state,joueur);
        case 5: return Hegemonic_Jr(state,joueur);

    }

    return 0;
}

int jouer_coup(int mode, int joueur, int bot1, int bot2, GameState* state)
{
    int coup;
    switch(mode){

            case 2: //mode JcJ

                coup=entree(joueur);
                break;

            case 3: //mode RANDY

                if(joueur==PLAYER_1)
                    coup=entree(joueur);
                
                else
                {
                    coup=randy(state,joueur);
                    printf("\nMove played: %c\n","ABCDEFG"[coup]);
                    
                }

                break;

            case 4: //mode RANDY_JR

                if(joueur==PLAYER_1)
                    coup=entree(joueur);
                else{
                    coup=randy_jr(state,joueur);
                    printf("\nMove played: %c\n","ABCDEFG"[coup]);
                    
                }

                break;

            case 5: //mode BOUBOULE

                if(joueur==PLAYER_1)
                    coup=entree(joueur);
                else{
                    coup=Bouboule(state,joueur);
                    printf("\nMove played: %c\n","ABCDEFG"[coup]);
                    
                }

                break;

            case 6: //Hegemonic mode

                if(joueur==PLAYER_1)
                    coup=entree(joueur);
                else{
                    coup=Hegemonic(state,joueur);
                    printf("\nMove played: %c\n","ABCDEFG"[coup]);
                    
                }

                break;

            case 7: //mode BOYARD

                if(joueur==PLAYER_1)
                    coup=entree(joueur);
                else{
                    coup=Boyard(state,joueur);
                    printf("\nMove played: %c\n","ABCDEFG"[coup]);
                    
                }

                break;
                
            case 8: //Hegemonic_Jr mode

                if(joueur==PLAYER_1)
                    coup=entree(joueur);
                else{
                    coup=Hegemonic_Jr(state,joueur);
                    printf("\nMove played: %c\n","ABCDEFG"[coup]);
                    
                }

                break;

            default: //dans les autres cas i.e. MODE_VERSUS et MODE_SUPERVERSUS

                if(joueur==PLAYER_1)
                {
                    coup=jouer_bot(bot1,state,joueur);
                    printf("\nMove played: %c\n","ABCDEFG"[coup]);
                    
                }
                else
                {
                    coup=jouer_bot(bot2,state,joueur);
                    printf("\nMove played: %c\n","ABCDEFG"[coup]);
                    
                }
                break;
                
    }
    
    consoleUpdate(NULL);
    return coup;
}

int simu_coup_IA (GameState* state, int size, char* argv[])
{

    const char* colors[] = { //On définit une liste des couleurs possibles pour les cases
        "\x1b[91m", // PLAYER_1 (rouge clair)
        "\x1b[94m",  // PLAYER_2 (bleu clair)
        "\x1b[31m", // RED
        "\x1b[32m", // GREEN
        "\x1b[34m", // BLUE
        "\x1b[33m", // YELLOW
        "\x1b[35m", // MAGENTA
        "\x1b[36m", // CYAN
        "\x1b[37m" // WHITE   
        };

    const char* reset = "\x1b[0m"; //pour reset couleur

    int numero_joueur = atoi(argv[4]);
    int verif=0;
    char* nom_bot = argv[3];

    if (numero_joueur < 0 || numero_joueur > 6)
    {
        fprintf(stderr, "Erreur : Une couleur (et donc le numero de l'IA) doit etre comprise entre 0 et 6.\n");
        consoleUpdate(NULL);
        return -1;
    } 

    printf("Le bot joue la lettre %s%c %s\n", colors[numero_joueur+2], "ABCDEFG"[numero_joueur], reset);
    consoleUpdate(NULL);
    for (int j=0; j<size; j++)
    {
        for (int i=0; i<size; i++)
        {
            int etat_couleur = atoi(argv[5+j*size+i])+3; //on fait +3 pour enlever J1, J2 et EMPTY puisque désormais le joueur peut être une des 7 couleurs
            if (etat_couleur < 3 || etat_couleur > 9)
            {
                fprintf(stderr, "Erreur : les couleurs doivent etre comprises entre 0 et 6.\n");
                consoleUpdate(NULL);
                return -1;
            }
            set_map_value(state, i, j, etat_couleur);

            if (etat_couleur-3==numero_joueur)
                verif = 1;
        }
    }

    if (verif==0)
    {
        fprintf(stderr, "Erreur : Le numero du joueur doit etre dans la liste des etats.\n");
        consoleUpdate(NULL);
        return -1;
    }

    afficher(state);

    if(strcmp(nom_bot,"Randy")==0)
        return randy(state,numero_joueur+3);
    else if(strcmp(nom_bot,"Randy_Jr")==0) 
        return randy_jr(state,numero_joueur+3);
    else if(strcmp(nom_bot,"Bouboule")==0)
        return Bouboule(state,numero_joueur+3);
    else if(strcmp(nom_bot,"Hegemonic")==0) 
        return Hegemonic(state,numero_joueur+3);
    else if(strcmp(nom_bot,"Boyard")==0) 
        return Boyard(state,numero_joueur+3);
    else if(strcmp(nom_bot,"Hegemonic_Jr")==0) 
        return Hegemonic_Jr(state,numero_joueur+3);

    else 
    {
        fprintf(stderr, "Bot inconnu\n");
        consoleUpdate(NULL);
        return -1;
    }
}


char* showKeyboard(const char* message, char* mode, size_t size) {
    SwkbdConfig kbd;

    Result keyrc=swkbdCreate(&kbd, 0);
    if (R_FAILED(keyrc))
        return NULL;


    swkbdConfigMakePresetDefault(&kbd);
    swkbdConfigSetGuideText(&kbd, message);

    swkbdShow(&kbd, mode, size);

    swkbdClose(&kbd);

    return mode;
}

int main(int argc, char* argv[]) {
    consoleInit(NULL);
    srand(time(NULL));

    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeDefault(&pad);
    printf("7colors game on switch!\n\n");

    
    while (appletMainLoop())
    {
        printf("\nPress A to start a new game, or press + to quit\n");

        // Update the console, sending a new frame to the display
        consoleUpdate(NULL);
        int continuer = -1;

        while (continuer == -1)
        {
            padUpdate(&pad);
            u64 kUp = padGetButtonsUp(&pad);
            if (kUp & HidNpadButton_A)
                continuer=1;

            else if (kUp & HidNpadButton_Plus)
                continuer=0;
        
            else if (kUp)
            {
                printf("Please press a valid key\n");
                consoleUpdate(NULL);
            }
        }
        
        if (continuer == 0)
            break;

        char mode_choisi[16];
        int size;

        if (showKeyboard("Enter a grid size", mode_choisi, sizeof(mode_choisi)) != NULL && strcmp(mode_choisi, "") != 0) {
            size = atoi(mode_choisi);   // conversion char* -> int
            printf("\nSize = %d\n", size);
            
        }

        else { //erreur si l'utilisateur ne rentre pas la taille de la carte en argument
            fprintf(stderr, "No size has been entered\n");
            consoleUpdate(NULL);
            return -1;
        }



        create_empty_game_state(&state, size);
        fill_map(&state);
    

        int mode=choisir_mode();

        int bot1=-1;
        int bot2=-1;

        if(mode==1||mode==0) // modes VERSUS ou SUPER_VERSUS
        {
            printf("\nChoose player 1\n");
            bot1=choisir_bot();

            printf("\nChoose player 2\n");
            bot2=choisir_bot();
        }


        int victory=0;

        int partie=1; //partie du super versus en cours
        int supround= 10; //nombre de parties à faire à la suite dans le mode versus

        int turn=0;
        int joueur;
        int vic1=0;
        int vic2=0;
        int coup;
        

        while(victory==0 && partie <= supround) //tant qu'il n'y a pas de victoire ou qu'on n'a pas joué toutes les parties du SUPERVERSUS
        {
            
            afficher(&state);
            

            joueur = (turn%2==0) ? PLAYER_1 : PLAYER_2; //PLAYER_1 commence toujours en premier
            
            coup=jouer_coup(mode, joueur, bot1, bot2, &state);
            
            tour(&state,joueur,coup);
            
            if (mode==1) // si MODE_SUPERVERSUS
            {
                printf("\nRound n%d\n", (partie));
                if (compter_victoire(&state, joueur))
                {
                    if (joueur==PLAYER_1)
                    {
                        vic1++;
                    }
                    else
                    {
                        vic2++;
                    }
                
                    partie++;
                    turn = -1; //-1 pour arriver à 0 lors de l'incrémentation de turn qui suit


                    fill_map(&state);
                    
                }
            }
            else
            {
                victory=compter_victoire(&state, joueur);
            }
            
            turn++;
            consoleUpdate(NULL);
            
        }

        if (mode==1) // si MODE_SUPERVERSUS
        {
            printf("\nPlayer 1 (%d) has won %d times and Player 2 (%d) has won %d times\n", bot1, vic1, bot2, vic2);
            /*
            Randy -> 0
            Randy_Jr -> 1;
            Bouboule -> 2;
            Hegemonic -> 3;
            Boyard -> 4;
            Hegemonic_Jr -> 5;
            */
        }

        else if (mode==0) //MODE VERSUS
        {
            printf("\nPlayer %d (%d) has won\n", joueur==PLAYER_1 ? 1 : 2, joueur==PLAYER_1 ? bot1 : bot2);
            /*
            Randy -> 0
            Randy_Jr -> 1;
            Bouboule -> 2;
            Hegemonic -> 3;
            Boyard -> 4;
            Hegemonic_Jr -> 5;
            */
        }

        else //MODE JcJ ou Joueur contre IA
        {
            printf("\nPlayer %d has won\n", joueur==PLAYER_1 ? 1 : 2);  
        }    

        free(state.map);
    }
    consoleExit(NULL);
    return 0;
}