//#########################################################
//#
//# Titre : 	Utilitaires Liste Chainee et CVS LINUX Automne 21
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

extern struct noeudVM *head;
extern struct noeudVM *queue;
extern int nbVM;
extern int nbThreadAELX;
extern sem_t semH, semQ, semnbVM, semC, semnbThreadAELX;

struct noeudVM *findItem(const int no) {

    //La liste est vide
    //verrouiller pointeur de tete et pointeur de queue
    sem_wait(&semH);
    sem_wait(&semQ);

    if ((head == NULL) && (queue == NULL)) {
        // deverrouiller pointeur de tete et pointeur de queue
        sem_post(&semQ);
        sem_post(&semH);
        return NULL;
    }


    //Pointeur de navigation

    //verrouiller noeud de tete
    sem_wait(&head->semNoeud);
    struct noeudVM *ptr = head;
    sem_post(&semQ);
    sem_post(&semH);

    if (ptr->VM.noVM == no) // premier noeudVM
        return ptr; // retourner le noeud de tete verrouille

    if (ptr->suivant != NULL) {
        sem_wait(&(ptr->suivant->semNoeud)); // verrouille noeud suivant de ptr
    } else { // ptr->suivant==NULL no invalide
        sem_post(&(ptr->semNoeud)); // deverrouille noeud de tete
    }
    //Tant qu'un item suivant existe
    while (ptr->suivant != NULL) {
        struct noeudVM *optr = ptr;
        //Déplacement du pointeur de navigation
        ptr = ptr->suivant;

        sem_post(&(optr->semNoeud));

        //Est-ce l'item recherché?
        if (ptr->VM.noVM == no) {
            return ptr; // retourner le noeud verrouille
        }
        if (ptr->suivant != NULL) {
            sem_wait(&(ptr->suivant->semNoeud));
        } else { // ptr->suivant==NULL no invalide
            sem_post(&(ptr->semNoeud)); // deverrouille dernier noeud verrouille
        }
    }
    //On retourne un pointeur NULL
    return NULL;
}

struct noeudVM *findPrev(const int no) {

    //La liste est vide
    //verrouiller pointeur de tete et pointeur de queue
    sem_wait(&semH);
    sem_wait(&semQ);

    if ((head == NULL) && (queue == NULL)) {
        // deverrouiller pointeur de tete et pointeur de queue
        sem_post(&semQ);
        sem_post(&semH);
        return NULL;
    }

    //verrouiller premier noeud
    sem_wait(&(head->semNoeud));

    //Pointeur de navigation
    struct noeudVM *ptr = head;
    // deverrouiller pointeur de tete et pointeur de queue
    sem_post(&semQ);
    sem_post(&semH);

    if (ptr->suivant != NULL) {
        sem_wait(&(ptr->suivant->semNoeud)); // verrouille noeud suivant de ptr
    } else { // ptr->suivant==NULL no invalide
        sem_post(&(ptr->semNoeud)); // deverrouille noeud de tete
    }

    //Tant qu'un item suivant existe
    while (ptr->suivant != NULL) {
        //Est-ce le prédécesseur de l'item recherché?
        if (ptr->suivant->VM.noVM == no) {
            //Deverouiller le noeud suivant
            sem_post(&(ptr->suivant->semNoeud));

            //On retourne un pointeur sur l'item précédent
            //Retourne le noeud verrouille
            return ptr;
        }
        struct noeudVM *optr = ptr;
        //Déplacement du pointeur de navigation
        ptr = ptr->suivant;
        sem_post(&(optr->semNoeud));

        if (ptr->suivant != NULL) {
            sem_wait(&(ptr->suivant->semNoeud)); // verrouille noeud suivant de ptr
        } else { // ptr->suivant==NULL no invalide
            sem_post(&(ptr->semNoeud)); // deverrouille noeud courant
        }
    }
    //On retourne un pointeur NULL
    return NULL;
}

void addItem(struct paramA *param) {
    int socket = param->clientSock;
    free(param);

    char msg[400] = "";
    char temp[400] = "";

    sem_wait(&semnbThreadAELX);
    nbThreadAELX++;
    sem_post(&semnbThreadAELX);

    //Création de l'enregistrement en mémoire
    struct noeudVM *ni = (struct noeudVM *) malloc(sizeof(struct noeudVM));
//printf("\n noVM=%d busy=%d adr ni=%p", ni->VM.noVM, ni->VM.busy, ni);
//printf("\n noVM=%d busy=%d adrQ deb=%p", ni->VM.noVM, ni->VM.busy,queue);

    //Affectation des valeurs des champs
    ni->VM.noVM = ++nbVM;
    //printf("\n noVM=%d", ni->VM.noVM);
    ni->VM.busy = 0;
    //printf("\n busy=%d", ni->VM.busy);
    ni->VM.ptrDebutVM = (unsigned short *) malloc(sizeof(unsigned short) * 65536);
//printf("\n noVM=%d busy=%d adrptr VM=%p", ni->VM.noVM, ni->VM.busy, ni->VM.ptrDebutVM);
//printf("\n noVM=%d busy=%d adrQ=%p", ni->VM.noVM, ni->VM.busy, queue);	

    // Semaphore pour le nouveau noeud
    sem_init(&(ni->semNoeud), 0, 1);

    //verrouiller pointeur de tete et pointeur de queue
    sem_wait(&semH);
    sem_wait(&semQ);

    if ((head == NULL) && (queue == NULL)) {//liste vide
        ni->suivant = NULL;
        queue = head = ni;
        // deverrouiller pointeur de tete et pointeur de queue
        sem_post(&semQ);
        sem_post(&semH);
        sem_wait(&semnbThreadAELX);
        nbThreadAELX--;
        sem_post(&semnbThreadAELX);

        sprintf(temp, "Added VM\n");
        strcat(msg, temp);
        writeSocket(msg, socket);

        pthread_exit(0);
    }

    sem_wait(&(queue->semNoeud));    // verrouillage du noeud de queue

    // deverrouiller pointeur de tete
    sem_post(&semH);


    struct noeudVM *tptr = queue;
    ni->suivant = NULL;
    queue = ni;
//printf("\n noVM=%d busy=%d adrQ=%p", ni->VM.noVM, ni->VM.busy, queue);	
    tptr->suivant = ni;
//printf("\n noVM=%d busy=%d adr Queue=%p", ni->VM.noVM, ni->VM.busy,queue);

    sem_post(&(tptr->semNoeud));

    sprintf(temp, "Added VM\n");
    strcat(msg, temp);
    writeSocket(msg, socket);

    // deverrouiller pointeur de queue (avant)
    sem_post(&semQ);
    sem_wait(&semnbThreadAELX);
    nbThreadAELX--;
    sem_post(&semnbThreadAELX);
}

void removeItem(struct paramE *param) {
    int noVM = param->noVM;
    int socket = param->clientSock;
    free(param);
    struct noeudVM *ptr;
    struct noeudVM *tptr;
    struct noeudVM *optr;

    char msg[400] = "";
    char temp[400] = "";

    // Attendre la terminaison des threads A E L et X
    while (1) {
        sem_wait(&semnbThreadAELX);

        if (nbThreadAELX == 0) {
            break;
        }
        sem_post(&semnbThreadAELX);
    }


    //Vérification sommaire (noVM>0 et liste non vide)	aucune suppression
    if ((noVM < 1) || ((head == NULL) && (queue == NULL))) {
        sem_post(&semnbThreadAELX);
        pthread_exit(0);
    }

    //Pointeur de recherche
    if (noVM == 1) {
        ptr = head;
        // suppression du premier element de la liste
        //Le noeud head reste verrouille
    } else {
        ptr = findItem(noVM - 1); //Si ptr ! NULL, noeud previous deja verrouille
    }


    //L'item a été trouvé
    if (ptr != NULL) {
        sem_post(&(ptr->semNoeud)); // deverrouillage du noeud pas necessaire d'etre verrouille
        nbVM--;

        if ((head == ptr) && (noVM == 1)) // suppression de l'element de tete
        {
            if (head == queue) // un seul element dans la liste
            {
                // Detruire le semaphore correspondant au noeud
                sem_destroy(&(ptr->semNoeud));
                free(ptr->VM.ptrDebutVM);
                free(ptr);
                queue = head = NULL;

                sprintf(temp, "Removed VM %d\n", noVM);
                strcat(msg, temp);
                writeSocket(msg, socket);

                sem_post(&semnbThreadAELX);
                pthread_exit(0);
            }
            tptr = ptr->suivant;
            head = tptr;
            // Detruire le semaphore correspondant au noeud
            sem_destroy(&(ptr->semNoeud));
            free(ptr->VM.ptrDebutVM);
            free(ptr);
        } else if (queue == ptr->suivant) // suppression de l'element de queue
        {
            queue = ptr;
            // Detruire le semaphore correspondant au queue
            sem_destroy(&(ptr->suivant->semNoeud));
            free(ptr->suivant->VM.ptrDebutVM);
            free(ptr->suivant);
            ptr->suivant = NULL;

            sprintf(temp, "Removed VM %d\n", noVM);
            strcat(msg, temp);
            writeSocket(msg, socket);

            sem_post(&semnbThreadAELX);
            pthread_exit(0);
        } else // suppression d'un element dans la liste
        {
            optr = ptr->suivant;    // optr pointe sur noeud a supprimer
            ptr->suivant = ptr->suivant->suivant;
            tptr = ptr->suivant;

            // Detruire le semaphore correspondant au noeud
            sem_destroy(&(optr->semNoeud));
            free(optr->VM.ptrDebutVM);
            free(optr);

            sprintf(temp, "Removed VM %d\n", noVM);
            strcat(msg, temp);
            writeSocket(msg, socket);
        }


        while (tptr != NULL) { // ajustement des numeros de VM
            //Ajustement du noVM
            tptr->VM.noVM--;
            //Déplacement du pointeur de navigation
            tptr = tptr->suivant;
        }
        sem_post(&semnbThreadAELX);
    } else {
        sem_post(&semnbThreadAELX);
    }

    sprintf(temp, "Removed VM %d\n", noVM);
    strcat(msg, temp);
    writeSocket(msg, socket);
}

void listItems(struct paramL *param) {
    int start = param->nstart;
    int end = param->nend;
    int socket = param->clientSock;
    free(param);
    sem_wait(&semnbThreadAELX);
    nbThreadAELX++;
    sem_post(&semnbThreadAELX);
    sem_wait(&semC);
    sem_wait(&semH);

    char msg[400] = "";
    char temp[400] = "";

    sprintf(temp, "noVM    Busy    Adresse Debut VM\n");
    strcat(msg, temp);
    sprintf(temp, "===================================\n");
    strcat(msg, temp);

    struct noeudVM *ptr = head;
    sem_wait(&(ptr->semNoeud));
    sem_post(&semH);

    while (ptr != NULL) {
        if ((ptr->VM.noVM >= start) && (ptr->VM.noVM <= end)) {
            sprintf(temp, " %d        %d        %p\n", ptr->VM.noVM, ptr->VM.busy, ptr->VM.ptrDebutVM);
            strcat(msg, temp);
        }
        if (ptr->VM.noVM > end) {
            ptr = NULL;
            sem_post(&(ptr->semNoeud));
        } else {
            struct noeudVM *optr = ptr;
            if (ptr->suivant != NULL) {
                sem_wait(&(ptr->suivant->semNoeud));
            }
            ptr = ptr->suivant;
            sem_post(&(optr->semNoeud));
        }
    }
    sprintf(temp, "===================================\n");
    strcat(msg, temp);

    writeSocket(msg, socket);

    sem_post(&semC);
    sem_wait(&semnbThreadAELX);
    nbThreadAELX--;
    sem_post(&semnbThreadAELX);
}

void executeFile(struct paramX *param) {
    char sourcefname[100];
    int noVM;
    char msg[400] = "";
    char temp[400] = "";
    int socket = param->clientSock;

    noVM = param->noVM;
    strcpy(sourcefname, (const char *) param->nomfich);
    free(param);

    sem_wait(&semnbThreadAELX);
    nbThreadAELX++;
    sem_post(&semnbThreadAELX);

/* Memory Storage */
/* 65536 locations */
    uint16_t *memory;
    uint16_t origin;
    uint16_t PC_START;

/* Register Storage */
    uint16_t reg[R_COUNT];

    // si ptr non NULL, noeud verrouille
    struct noeudVM *ptr = findItem(noVM);

    if (ptr == NULL) {
        sem_wait(&semC);

        sprintf(temp, "Virtual Machine unavailable\n");
        strcat(msg, temp);
        writeSocket(msg, socket);

        sem_post(&semC);
        //return(0);
        pthread_exit(0);
    }
    memory = ptr->VM.ptrDebutVM;

    if (!read_image_file(memory, sourcefname, &origin)) {
        sem_wait(&semC);
        sprintf(temp, "Failed to load image: %s\n", sourcefname);
        strcat(msg, temp);
        writeSocket(msg, socket);

        sem_post(&semC);
        //return(0);
        // liberer le noeud verrouille
        sem_post(&ptr->semNoeud);
        pthread_exit(0);
    }

    while (ptr->VM.busy != 0) { // wait for the VM
    }
    // Acquiring access to the VM
    ptr->VM.busy = 1;

    /* Setup */
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    /* set the PC to starting position */
    /* at  ptr->VM.ptrDebutVM + 0x3000 is the default  */
    //enum { PC_START = origin };
    PC_START = origin;
    reg[R_PC] = PC_START;

    //Verrouiller la console
    sem_wait(&semC);

    int running = 1;
    while (running) {
        /* FETCH */
        uint16_t instr = mem_read(memory, reg[R_PC]++);
// printf("\n instr = %x", instr);
        uint16_t op = instr >> 12;

// printf("\n exe op = %x", op);

        switch (op) {
            case OP_ADD:
                /* ADD */
            {
                /* destination register (DR) */
                uint16_t r0 = (instr >> 9) & 0x7;
                /* first operand (SR1) */
                uint16_t r1 = (instr >> 6) & 0x7;
                /* whether we are in immediate mode */
                uint16_t imm_flag = (instr >> 5) & 0x1;

                if (imm_flag) {
                    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                    reg[r0] = reg[r1] + imm5;
                } else {
                    uint16_t r2 = instr & 0x7;
                    reg[r0] = reg[r1] + reg[r2];

                    sprintf(temp, " add reg[r0] (sum) = %d\n", reg[r0]);
                    strcat(msg, temp);
                    //printf("\t add reg[r1] (sum avant) = %d", reg[r1]);
                    //printf("\t add reg[r2] (valeur ajoutee) = %d", reg[r2]);
                }

                update_flags(reg, r0);
            }

                break;
            case OP_AND:
                /* AND */
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t imm_flag = (instr >> 5) & 0x1;

                if (imm_flag) {
                    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                    reg[r0] = reg[r1] & imm5;
                } else {
                    uint16_t r2 = instr & 0x7;
                    reg[r0] = reg[r1] & reg[r2];
                }
                update_flags(reg, r0);
            }

                break;
            case OP_NOT:
                /* NOT */
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;

                reg[r0] = ~reg[r1];
                update_flags(reg, r0);
            }

                break;
            case OP_BR:
                /* BR */
            {
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                uint16_t cond_flag = (instr >> 9) & 0x7;
                if (cond_flag & reg[R_COND]) {
                    reg[R_PC] += pc_offset;
                }
            }

                break;
            case OP_JMP:
                /* JMP */
            {
                /* Also handles RET */
                uint16_t r1 = (instr >> 6) & 0x7;
                reg[R_PC] = reg[r1];
            }

                break;
            case OP_JSR:
                /* JSR */
            {
                uint16_t long_flag = (instr >> 11) & 1;
                reg[R_R7] = reg[R_PC];
                if (long_flag) {
                    uint16_t long_pc_offset = sign_extend(instr & 0x7FF, 11);
                    reg[R_PC] += long_pc_offset;  /* JSR */
                } else {
                    uint16_t r1 = (instr >> 6) & 0x7;
                    reg[R_PC] = reg[r1]; /* JSRR */
                }
                break;
            }

                break;
            case OP_LD:
                /* LD */
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                reg[r0] = mem_read(memory, reg[R_PC] + pc_offset);
                update_flags(reg, r0);
            }

                break;
            case OP_LDI:
                /* LDI */
            {
                /* destination register (DR) */
                uint16_t r0 = (instr >> 9) & 0x7;
                /* PCoffset 9*/
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                /* add pc_offset to the current PC, look at that memory location to get the final address */
                reg[r0] = mem_read(memory, mem_read(memory, reg[R_PC] + pc_offset));
                update_flags(reg, r0);
            }

                break;
            case OP_LDR:
                /* LDR */
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t offset = sign_extend(instr & 0x3F, 6);
                reg[r0] = mem_read(memory, reg[r1] + offset);
                update_flags(reg, r0);
            }

                break;
            case OP_LEA:
                /* LEA */
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                reg[r0] = reg[R_PC] + pc_offset;
                update_flags(reg, r0);
            }

                break;
            case OP_ST:
                /* ST */
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                mem_write(memory, reg[R_PC] + pc_offset, reg[r0]);
            }

                break;
            case OP_STI:
                /* STI */
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                mem_write(memory, mem_read(memory, reg[R_PC] + pc_offset), reg[r0]);
            }

                break;
            case OP_STR:
                /* STR */
            {
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                uint16_t offset = sign_extend(instr & 0x3F, 6);
                mem_write(memory, reg[r1] + offset, reg[r0]);
            }

                break;
            case OP_TRAP:
                /* TRAP */
                switch (instr & 0xFF) {
                    case TRAP_GETC:
                        /* TRAP GETC */
                        /* read a single ASCII char */
                        reg[R_R0] = (uint16_t) getchar();

                        break;
                    case TRAP_OUT:
                        /* TRAP OUT */
                        putc((char) reg[R_R0], stdout);
                        fflush(stdout);

                        break;
                    case TRAP_PUTS:
                        /* TRAP PUTS */
                    {
                        /* one char per word */
                        uint16_t *c = memory + reg[R_R0];
                        while (*c) {
                            putc((char) *c, stdout);
                            ++c;
                        }
                        fflush(stdout);
                    }

                        break;
                    case TRAP_IN:
                        /* TRAP IN */
                    {
                        printf("Enter a character: ");
                        char c = getchar();
                        putc(c, stdout);
                        reg[R_R0] = (uint16_t) c;
                    }

                        break;
                    case TRAP_PUTSP:
                        /* TRAP PUTSP */
                    {
                        /* one char per byte (two bytes per word)
                           here we need to swap back to
                           big endian format */
                        uint16_t *c = memory + reg[R_R0];
                        while (*c) {
                            char char1 = (*c) & 0xFF;
                            putc(char1, stdout);
                            char char2 = (*c) >> 8;
                            if (char2) putc(char2, stdout);
                            ++c;
                        }
                        fflush(stdout);
                    }

                        break;
                    case TRAP_HALT:
                        /* TRAP HALT */
//                        puts("\n HALT");
                        sprintf(temp, " HALT\n");
                        strcat(msg, temp);

                        fflush(stdout);
                        running = 0;

                        break;
                }

                break;
            case OP_RES:
            case OP_RTI:
            default:
                /* BAD OPCODE */
                abort();

                break;
        }
    }
    writeSocket(msg, socket);

    ptr->VM.busy = 0;
    restore_input_buffering();
    sem_post(&ptr->semNoeud);
    sem_post(&semC);
    sem_wait(&semnbThreadAELX);
    nbThreadAELX--;
    sem_post(&semnbThreadAELX);
    pthread_exit(NULL);
}

void writeSocket(char *text, int socket) {
    char message[400] = "";
    strcpy(message, text);
    write(socket, message, sizeof(message) + 1);
}