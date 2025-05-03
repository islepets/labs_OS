#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "schedulers.h"
#include "task.h"
#include "cpu.h"

struct node *taskList = NULL;

void add(char *name, int priority, int burst) {
    Task *newTask = malloc(sizeof(Task));
    if (newTask == NULL) {
        perror("Failed to allocate memory for new task");
        exit(EXIT_FAILURE);
    }
    newTask->name = strdup(name);
    if (newTask->name == NULL) {
        perror("Failed to allocate memory for task name");
        free(newTask);
        exit(EXIT_FAILURE);
    }
    newTask->priority = priority;
    newTask->burst = burst;

    insert(&taskList, newTask);
}

void schedule() {
    struct node *current = taskList;

    while (current != NULL) {
        Task *task = current->task;
        int timeSlice = (task->burst > QUANTUM) ? QUANTUM : task->burst;
        run(task, timeSlice);
        task->burst -= timeSlice;

        if (task->burst <= 0) {
            delete(&taskList, task);
        }
        current = current->next;
        if (current == NULL) {
            current = taskList;
        }
    }
}
