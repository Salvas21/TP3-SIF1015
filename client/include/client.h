//
// Created by Piérik Landry on 2021-11-24.
//

#ifndef TP2_SIF1015_CLIENT_H
#define TP2_SIF1015_CLIENT_H
#define SERVER_FIFO_NAME "/tmp/FIFO_TRANSACTIONS"
#define CLIENT_FIFO_NAME "/tmp/FIFO%d"

#include <ncurses.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include <arpa/inet.h>

struct info_socket_Transaction {
    pid_t pid_client;
    char transaction[200];
};

int sock;
int connection_status;

WINDOW *serverWindow;

char *appendChar(char *szString, size_t strsize, char c);

void initScreen();

void initColors();

void initSocket();

void writeCommandOnWindow(WINDOW *window, const char *text_char, int commandLine);

void sendDataToSocket(const char *text_char);

void writeRainbowText(WINDOW *window, const char *text_char, int commandLine);

int executeCommand(WINDOW *window, const char *text_char, int commandLine);

WINDOW *createWindow(int height, int width, int position_y, int position_x, const char *text_window);

void clearWindow(WINDOW *window, const char *text_window);

void *serverWindowThread();

#endif //TP2_SIF1015_CLIENT_H
