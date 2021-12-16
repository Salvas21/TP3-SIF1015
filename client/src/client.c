//
// Created by Piérik Landry on 2021-11-24.
//
#include "../include/client.h"

int main() {

    //init the screen
    initScreen();

    //init socket
    initSocket();

    //init color
    initColors();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);

    //init window sizes
    int height, width, start_y, start_x;
    height = 25;
    width = 50;
    start_y = start_x = 2;

    //init client&sever command input
    char command[80];
    char clientInput = 0;
    int commandLine = 0;
    int secret = 0;


    //init thread
    pthread_t tid;

    //creating client window
    WINDOW *clientWindow = createWindow(height, width, start_y, start_x, "");
    mvwprintw(clientWindow, 1, 1, "Waiting...");
    wrefresh(clientWindow);
    if (connection_status < 0) {
        mvwprintw(clientWindow, 2, 1, "Error!");
        return 0;
    } else {
        mvwprintw(clientWindow, 2, 1, "Connection established!");
        wrefresh(clientWindow);
    }

    //creating server window thread
    serverWindow = createWindow(height, width, start_y, start_x + width, "Waiting...");
    pthread_create(&tid, NULL, serverWindowThread, NULL);

    //resetting command array
    memset(command, 0, sizeof command);

    clearWindow(clientWindow, "");

    //creating input window with custom color
    WINDOW *inputWindow = createWindow(3, width - 3, height - 2, start_x + 1, "Command : ");
    wbkgd(inputWindow, COLOR_PAIR(1));
    wrefresh(inputWindow);

    //move cursor to inputWindow
    move(height - 1, start_x + 12);

    while (true) {
        clientInput = getch();

        //if input is ENTER, add command to client window
        if (clientInput == 10) {
            commandLine += 1;

            //resetting client window when all lines full
            if (commandLine > height - 5) {
                commandLine = 1;
                clearWindow(clientWindow, "");
            }

            //writing command to client window
            mvwprintw(clientWindow, commandLine, 1, "You Entered:");

            //add color to the command word
            if (secret == 1) {
                writeRainbowText(clientWindow, command, commandLine);
            } else {
                writeCommandOnWindow(clientWindow, command, commandLine);
            }

            if (executeCommand(clientWindow, command, commandLine) == -1) {
                mvwprintw(clientWindow, commandLine, 1, "%s", "Invalid command!");
            }

            wrefresh(clientWindow);

            //end program and thread when input is quit
            if (strcmp(command, "quit") == 0) {
                pthread_cancel(tid);
                break;
            }

            if (strcmp(command, "uuddlrlrba") == 0) {
                commandLine += 1;
                secret = 1;
                char konami[20] = "KONAMI CODE UNLOCK";
                writeRainbowText(clientWindow, konami, commandLine);
                wrefresh(clientWindow);
            }

            //resetting input window and variable
            memset(command, 0, sizeof command);
            move(height - 1, start_x + 12);

            clearWindow(inputWindow, "Command : ");
        } else {
            //if input is DELETE, remove last letter
            if (clientInput == 127) {
                command[strlen(command) - 1] = '\0';
            } else {
                appendChar(command, sizeof(command), clientInput);
            }

            clearWindow(inputWindow, "");

            //manually updating inputWindow
            mvwprintw(inputWindow, 1, 1, "Command : %s", command);
            wrefresh(inputWindow);

        }
    }

    // Close the connection
    close(sock);
    endwin();
    pthread_exit(NULL);
}

void initScreen() {
    initscr();
    cbreak();
    move(1, 20);
    printw("Client");
    move(1, 72);
    printw("Server");
    move(27, 35);
    printw("Try if you dare : uuddlrlrba");
}

void initColors() {
    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    start_color();
}

void initSocket() {
    // Create a stream socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // Initialise port number and address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8888);

    // Initiate a socket connection
    connection_status = connect(sock, (struct sockaddr *) &server_address, sizeof(server_address));
}

void writeCommandOnWindow(WINDOW *window, const char *text_char, int commandLine) {
    wattron(window, COLOR_PAIR(1));
    mvwprintw(window, commandLine, 14, "%s", text_char);
    wattroff(window, COLOR_PAIR(1));
}

void writeRainbowText(WINDOW *window, const char *text_char, int commandLine) {
    int color = 1;
    for (int i = 1; i < strlen(text_char) + 1; ++i) {
        if (color > 3) {
            color = 1;
        }
        wattron(window, COLOR_PAIR(color));
        mvwprintw(window, commandLine, 13 + i, "%c", text_char[i - 1]);
        wattroff(window, COLOR_PAIR(color));
        color += 1;

    }
}

int executeCommand(WINDOW *window, const char *text_char, int commandLine) {

    switch (text_char[0]) {
        case 'a':
        case 'A':
            sendDataToSocket(text_char);
            return 0;
        case 'l':
        case 'L':
            sendDataToSocket(text_char);
            return 0;
        case 'x':
        case 'X':
            sendDataToSocket(text_char);
            return 0;
        case 'e':
        case 'E':
            sendDataToSocket(text_char);
            return 0;

    }
    return -1;
}

void sendDataToSocket(const char *text_char) {
    struct info_socket_Transaction my_data;
    my_data.pid_client = getpid();
    sprintf(my_data.transaction, "%s", text_char);
    if (send(sock, &my_data, sizeof(my_data), 0) < 0) {
        puts("Did not send");
    }

}

char *appendChar(char *szString, size_t strsize, char c) {
    size_t len = strlen(szString);
    if ((len + 1) < strsize) {
        szString[len++] = c;
        szString[len] = '\0';
        return szString;
    }
    return NULL;
}

WINDOW *createWindow(int height, int width, int position_y, int position_x, const char *text_window) {
    WINDOW *window = newwin(height, width, position_y, position_x);
    refresh();
    box(window, 0, 0);
    mvwprintw(window, 1, 1, text_window);
    wrefresh(window);
    return window;
}

void clearWindow(WINDOW *window, const char *text_window) {
    werase(window);
    box(window, 0, 0);
    mvwprintw(window, 1, 1, text_window);
    wrefresh(window);
}

void *serverWindowThread() {
    werase(serverWindow);
    char message[400] = "";
    int i = 0;

    while (1) {
        if (recv(sock, message, sizeof(message), 0) < 0) {
            mvwprintw(serverWindow, i + 1, 1, "Could not read from server.");
        }
        if (strlen(message) > 0) {
            if (i > 20) {
                i = 0;
                werase(serverWindow);
                box(serverWindow, 0, 0);
                wrefresh(serverWindow);
            }
            mvwprintw(serverWindow, i + 1, 1, message);
            i = getcury(serverWindow);
            box(serverWindow, 0, 0);
            wrefresh(serverWindow);
        }
    }

    pthread_exit(NULL);
}
