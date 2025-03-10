#include "myshell.h"

int main() {
    char command[MAX_COMMAND_LEN];
    char *arguments[MAX_ARG];
    char cwd[1024];
    FILE *batchFile = NULL;

    // Пытаемся открыть файл для пакетной обработки
    batchFile = fopen("batch.txt", "r");
    if (batchFile == NULL) {
        printf("batch.txt не найден. Переход в интерактивный режим.\n");
    }

    while (1) {
        // Проверяем завершившиеся процессы
        while (1) {
            pid_t pid = waitpid(-1, NULL, WNOHANG);
            if (pid == 0) {
                break; // Нет завершившихся процессов
            } else if (pid < 0) {
                if (errno == ECHILD) {
                    break; // Нет дочерних процессов
                } else {
                    perror("waitpid");
                    break;
                }
            } else {
                printf("Завершен процесс %d.\n", pid);
            }
        }

        // Получаем текущую рабочую директорию
        if (batchFile) {
            if (fgets(command, MAX_COMMAND_LEN, batchFile) == NULL) {
                break; // Достигнут конец файла
            }
        } else {
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("myshell [%s]> ", cwd);
            } else {
                perror("getcwd");
                printf("myshell> ");
            }

            if (fgets(command, MAX_COMMAND_LEN, stdin) == NULL) {
                printf("\n");
                break; 
            }
        }

        command[strcspn(command, "\n")] = 0; // Убираем символ новой строки

        // Разбиваем команду на токены (аргументы)
        char* token = strtok(command, " ");
        int i = 0;
        while(token != NULL && i < MAX_ARG - 1) {
            arguments[i++] = token; 
            token = strtok(NULL, " "); 
        }
        arguments[i] = NULL; 

        if(arguments[0] == NULL) {
            continue; // Если команда пуста
        }

        // Проверяем, нужно ли выполнять в фоновом режиме
        int background = 0;
        if (i > 0 && strcmp(arguments[i - 1], "&") == 0) {
            background = 1;
            arguments[--i] = NULL; // Убираем амперсанд из аргументов
        }

        // Перенаправление ввода/вывода
        int input_fd = -1, output_fd = -1;
        char *infile = NULL, *outfile = NULL;
        for (int j = 0; j < i; j++) {
            if (strcmp(arguments[j], "<") == 0) {
                infile = arguments[j + 1];
                j++; // Пропускаем следующий аргумент
            } else if (strcmp(arguments[j], ">") == 0) {
                outfile = arguments[j + 1];
                j++; // Пропускаем следующий аргумент
            } else if (strcmp(arguments[j], ">>") == 0) {
                outfile = arguments[j + 1];
                j++; // Пропускаем следующий аргумент
            }
        }

        // Сохраняем стандартные дескрипторы
        int saved_stdin = dup(STDIN_FILENO);
        int saved_stdout = dup(STDOUT_FILENO);

        // Открытие файлов для перенаправления
        if (infile) {
            input_fd = open(infile, O_RDONLY);
            if (input_fd < 0) {
                perror("open input file");
                continue;
            }
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }

        if (outfile) {
            arguments[i - 1] = NULL;

            int flags = O_WRONLY | O_CREAT | ((strcmp(arguments[i - 2], ">>") == 0) ? O_APPEND : O_TRUNC);

            arguments[i - 2] = NULL;  
            output_fd = open(outfile, flags, 0644);
            if (output_fd < 0) {
                perror("open output file");
                dup2(saved_stdin, STDIN_FILENO);
                continue;
            }
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }       

        // Обработка встроенных команд
        if (strcmp(arguments[0], "echo") == 0) {
            my_echo(arguments);
            dup2(saved_stdout, STDOUT_FILENO);
            continue;
        }

        if (strcmp(arguments[0], "clr") == 0) {
            my_clear();
            dup2(saved_stdout, STDOUT_FILENO);
            continue;
        }

        if (strcmp(arguments[0], "cd") == 0) {
            my_cd(arguments[1]);
            dup2(saved_stdout, STDOUT_FILENO);
            continue;
        }

        if (strcmp(arguments[0], "help") == 0) {
            my_help();
            dup2(saved_stdout, STDOUT_FILENO);
            continue;
        }

        if (strcmp(arguments[0], "quit") == 0) {
            my_quit();
            break; 
        }

        if (strcmp(arguments[0], "environ") == 0) {
            my_environ();
            dup2(saved_stdout, STDOUT_FILENO);
            continue;
        }

        if (strcmp(arguments[0], "dir") == 0) {
            my_dir();
            dup2(saved_stdout, STDOUT_FILENO);
            continue;
        }
        
        if (strcmp(arguments[0], "pause") == 0) {
            my_pause();
            dup2(saved_stdout, STDOUT_FILENO);
            continue;
        }

        // Создаем новый процесс
        pid_t pid = fork();
        if (pid == 0) {
            // В дочернем процессе
            execvp(arguments[0], arguments);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("fork");
        } else {
            // В родительском процессе
            if (background) {
                // Если фоновый процесс, не ждем завершения
                printf("[%d]\n", pid);
            } else {
                // Если не фоновый процесс, ждем завершения
                int status;
                waitpid(pid, &status, 0);
            }
        }

        // Восстанавливаем стандартные дескрипторы
        dup2(saved_stdin, STDIN_FILENO);
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdin);
        close(saved_stdout);
    }   

    // Закрываем файл, если он был открыт
    if (batchFile) {
        fclose(batchFile);
    }

    return 0;
}
