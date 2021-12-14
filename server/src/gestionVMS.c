//#########################################################
//#
//# Titre : 	Utilitaires CVS LINUX Automne 21
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
extern struct noeud* head;
//Pointeur de queue de liste pour ajout rapide
extern struct noeud* queue;

// nombre de VM actives
extern int nbVM;

// nombre de threads actifs
extern int nbThreadAELX;
//Semaphores
extern sem_t semH, semQ, semnbVM, semC, semnbThreadAELX;

int continueReadingFifo = 1;

//#######################################
//#
//# Affiche une série de retour de ligne pour "nettoyer" la console
//#
void cls(void){
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	}

//#######################################
//#
//# Affiche un messgae et quitte le programme
//#
void error(const int exitcode, const char * message){
	printf("\n-------------------------\n%s\n",message);
	exit(exitcode);
	}
	
/* Sign Extend */
uint16_t sign_extend(uint16_t x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

/* Swap */
uint16_t swap16(uint16_t x)
{
    return (x << 8) | (x >> 8);
}

/* Update Flags */
void update_flags(uint16_t reg[R_COUNT], uint16_t r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15) /* a 1 in the left-most bit indicates negative */
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

/* Read Image File */
int read_image_file(uint16_t * memory, char* image_path,uint16_t * origin)
{
    char fich[200];
    strncpy(fich, image_path, strlen(image_path)+1);
    FILE* file = fopen(fich, "rb");

    if (!file) { return 0; }
    /* the origin tells us where in memory to place the image */
   	*origin=0x3000;

    /* we know the maximum file size so we only need one fread */
    uint16_t max_read = UINT16_MAX - *origin;
    uint16_t* p = memory + *origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);
    /* swap to little endian ???? */
    while (read-- > 0)
    {
    //	printf("\n p * BIG = %x",*p);
       // *p = swap16(*p);
		// printf("\n p * LITTLE = %x",*p);
        ++p;
    }
    return 1;
}


/* Check Key */
uint16_t check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}

/* Memory Access */
void mem_write(uint16_t * memory, uint16_t address, uint16_t val)
{
    memory[address] = val;
}

uint16_t mem_read( uint16_t * memory, uint16_t address)
{
    if (address == MR_KBSR)
    {
        if (check_key())
        {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        }
        else
        {
            memory[MR_KBSR] = 0;
        }
    }
    return memory[address];
}

/* Input Buffering */
struct termios original_tio;

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

/* Handle Interrupt */
void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

void terminateReadingFifo() {
    continueReadingFifo = 0;
}

void readTrans(struct paramReadTrans* param) {
//    int server_fifo_fd;
//    server_fifo_fd = open(SERVER_FIFO_NAME, O_RDONLY);
//    if (server_fifo_fd == -1) {
//        fprintf(stderr, "Server Fifo Failure\n");
//        unlink(SERVER_FIFO_NAME);
//        exit(EXIT_FAILURE);
//    }
    char client_message[2000];
    int read_size;

	pthread_t tid[1000];
	int nbThread = 0;
	int i;

    int clientSocket = param->socket;
    free(param);
    while( (read_size = recv(clientSocket , client_message , 2000 , 0)) > 0 )
    {
        //Send the message back to client
        puts(client_message);
        write(clientSocket , client_message , strlen(client_message));
    }

    int read_res;
    struct info_FIFO_Transaction my_data;

//    signal(SIGINT, terminateReadingFifo);

    char *tok, *sp;
//    while((read_size = recv(clientSocket , &my_data ,sizeof(my_data)  , 0)) > 0) {
////        read_res = read(clientSocket, &my_data, sizeof(my_data));
//        puts(my_data.transaction);
//        read_res = 1;
//        if (read_res > 0) {
//            tok = strtok_r(my_data.transaction, " ", &sp);
//            switch(tok[0]){
//                case 'A':
//                case 'a':{
//                    pthread_create(&tid[nbThread++], NULL, addItem, NULL);
////                    writeFifo("Added VM",my_data.pid_client);
//                    break;
//                }
//                case 'E':
//                case 'e':{
//                    int noVM = atoi(strtok_r(NULL, " ", &sp));
//                    struct paramE *ptr = (struct paramE*) malloc(sizeof(struct paramE));
//                    ptr->noVM = noVM;
//                    ptr->pid = my_data.pid_client;
//                    pthread_create(&tid[nbThread++], NULL, removeItem, ptr);
//                    break;
//                }
//                case 'L':
//                case 'l':{
//                    int nstart = atoi(strtok_r(NULL, "-", &sp));
//                    int nend = atoi(strtok_r(NULL, " ", &sp));
//
//                    struct paramL *ptr = (struct paramL*) malloc(sizeof(struct paramL));
//                    ptr->nstart = nstart;
//                    ptr->nend = nend;
//                    ptr->pid = my_data.pid_client;
//
//                    pthread_create(&tid[nbThread++], NULL, listItems, ptr);
//                    break;
//                }
//                case 'X':
//                case 'x':{
//                    int noVM = atoi(strtok_r(NULL, " ", &sp));
//                    char *nomfich = strtok_r(NULL, "\n", &sp);
//
//                    struct paramX *ptr = (struct paramX*) malloc(sizeof(struct paramX));
//                    ptr->noVM = noVM;
//                    ptr->pid = my_data.pid_client;
//                    strcpy(ptr->nomfich,(const char *)nomfich);
//
//                    pthread_create(&tid[nbThread++], NULL, executeFile, ptr);
//                    break;
//                }
//            }
//        }
//        memset(&my_data,0, sizeof(my_data));
//    }

	for(i=0; i<nbThread;i++) {
		pthread_join(tid[i], NULL);
	}
    pthread_exit(0);
}


