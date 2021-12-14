//#########################################################
//#
//# Titre : 	UTILITAIRES (MAIN) TP1 LINUX Automne 21
//#				SIF-1015 - Système d'exploitation
//#				Université du Québec à Trois-Rivières
//#            VERSION CONCURRENTE
//#
//# Auteur : 	Francois Meunier
//#	Date :		Novembre 2021
//#
//# Langage : 	ANSI C on LINUX 
//#
//#######################################

#include "../include/gestionListeChaineeVMS.h"
#include "../include/gestionVMS.h"

//Pointeur de tête de liste
struct noeud* head;
//Pointeur de queue de liste pour ajout rapide
struct noeud* queue;
// nombre de VM actives
int nbVM;

//nombre de threads actifs A E L X
int nbThreadAELX;


// Creer 2 semaphores pour pointeur de tete et pointeur de queue
// Semaphore acces a nbVM et nbThreadALX
sem_t semH, semQ, semnbVM, semC, semnbThreadAELX;


int main(int argc, char* argv[]){

    int sockServer , sockClient , c , read_size;
    struct sockaddr_in server , client;

	head = NULL;
	queue = NULL;
	nbVM = 0;
	nbThreadAELX = 0;
	
	sem_init(&semH, 0, 1);
	sem_init(&semQ, 0, 1);
	sem_init(&semnbVM, 0, 1);
	sem_init(&semC, 0, 1);
	sem_init(&semnbThreadAELX, 0, 1);
	
//    mkfifo(SERVER_FIFO_NAME, 0777);

    sockServer = socket(AF_INET, SOCK_STREAM, 0);
    if (sockServer == -1)
    {
        perror("Could not create socket");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
    if(bind(sockServer,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("bind failed. Error");
        return 1;
    }

    listen(sockServer, 5);
    pthread_t tid[1000];
    int nbThread = 0;
    int i;
    while (1) {
        puts("Hi");
        c = sizeof(struct sockaddr_in);
        sockClient = accept(sockServer, (struct sockaddr *)&client, (socklen_t*)&c);
        puts("Connection accepted");
        struct paramReadTrans *ptr = (struct paramReadTrans*) malloc(sizeof(struct paramReadTrans));
        ptr->socket = sockClient;
        pthread_create(&tid[nbThread++], NULL, readTrans, ptr);
    }

    for(i=0; i<nbThread;i++) {
        pthread_join(tid[i], NULL);
    }
//    readTrans();

    exit(0);
}

