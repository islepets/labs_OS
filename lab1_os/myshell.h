#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>

#define MAX_COMMAND_LEN 1024  // Максимальная длина команды
#define MAX_ARG 64            // Максимальное количество аргументов
#define MAX_PATH_LEN 2048     // Максимальная длина пути

extern char **environ;       // Указатель на переменные окружения
extern char command[MAX_COMMAND_LEN]; // Массив для хранения введенной команды
extern char* arguments[MAX_ARG];    // Массив для хранения аргументов команды
extern pid_t pid;                   // Идентификатор процесса
extern int status;                  // Статус завершения процесса
extern char cwd[MAX_PATH_LEN];      // Массив для хранения текущего рабочего каталога

/*
    @my_cd

    Осуществляет перемещение по каталогам
*/
void my_cd(char* path);

/*
    @my_echo

    Выводит содержимое в консоль/терминал
*/
void my_echo(char** argv);

/*
    @my_clear

    Очищает консоль/терминал
*/
void my_clear();

/*
    @my_help

    Выводит краткую информацию об использовании команд оболочки
*/
void my_help();

/*
    @my_quit

    Выход из оболочки
*/
void my_quit();

/*
    @my_environ

    Вывод всех переменных среды
*/
void my_environ();

/*
    @my_dir

    Вывод содержимого каталога
*/
void my_dir();

/*
    @my_pause

    Приостанавливает операции оболочки до нажатия Enter
*/
void my_pause();