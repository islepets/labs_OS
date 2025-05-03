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
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    newTask->name = strdup(name);
    if (newTask->name == NULL) {
        fprintf(stderr, "String duplication failed\n");
        free(newTask);  
        exit(1);  
    }
    
    newTask->priority = priority;
    newTask->burst = burst;

    insert(&taskList, newTask);
}

void schedule() {
    struct node *current = taskList;
    while (current != NULL) {
        Task *task = current->task;
        run(task, task->burst);
        current = current->next;
    }
}
