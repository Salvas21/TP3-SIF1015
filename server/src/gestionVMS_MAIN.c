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
	head = NULL;
	queue = NULL;
	nbVM = 0;
	nbThreadAELX = 0;
	
	sem_init(&semH, 0, 1);
	sem_init(&semQ, 0, 1);
	sem_init(&semnbVM, 0, 1);
	sem_init(&semC, 0, 1);
	sem_init(&semnbThreadAELX, 0, 1);
	
    mkfifo(SERVER_FIFO_NAME, 0777);

    readTrans();
    unlink(SERVER_FIFO_NAME);

    exit(0);
}

