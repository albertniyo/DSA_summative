#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


void init_logger(e_log *logger) {
    logger->head = NULL;
    logger->tail = NULL;
    logger->current = NULL;
    logger->count = 0;
    logger->next_id = 1;
}

static void remove_old(e_log *logger) {
    e_node *oldest;

    if (logger->head == NULL) {
        return;
    }

    oldest = logger->head;
    logger->head = oldest->next;

    if (logger->head != NULL) {
        logger->head->prev = NULL;
    } else {
        logger->tail = NULL;
    }

    if (logger->current == oldest) {
        logger->current = logger->head;
    }

    free(oldest);
    logger->count--;
}

int add_event(e_log *logger, const char *type, const char *sensor_id,
            const char *timestamp, const char *severity) {
    e_node *event;

    if (logger->count >= max_event) {
        remove_old(logger);
    }

    event = (e_node *) malloc(sizeof(e_node));
    if (event == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        return 0;
    }

    event->id = logger->next_id++;
    strncpy(event->type, type, lim - 1);
    event->type[lim - 1] = '\0';
    strncpy(event->sensor_id, sensor_id, lim - 1);
    event->sensor_id[lim - 1] = '\0';
    strncpy(event->timestamp, timestamp, lim - 1);
    event->timestamp[lim - 1] = '\0';
    strncpy(event->severity, severity, lim - 1);
    event->severity[lim - 1] = '\0';
    event->prev = NULL;
    event->next = NULL;

    if (logger->head == NULL) {
        logger->head = event;
        logger->tail = event;
        logger->current = event;
    } else {
        event->prev = logger->tail;
        logger->tail->next = event;
        logger->tail = event;
    }

    logger->count++;
    printf("event %d added %d/%d\n",
        event->id, logger->count, max_event);
    return 1;
}

void display_current(const e_log *logger) {
    e_node *e;

    if (logger->current == NULL) {
        puts("no events in the logger");
        return;
    }

    e = logger->current;
    puts("\n--- current event ---");
    printf("ID:        %d\n", e->id);
    printf("Type:      %s\n", e->type);
    printf("Sensor:    %s\n", e->sensor_id);
    printf("Timestamp: %s\n", e->timestamp);
    printf("Severity:  %s\n", e->severity);
    puts(" ");
}

void move_next(e_log *logger) {
    if (logger->current == NULL) {
        puts("no events to navigate");
        return;
    }

    if (logger->current->next == NULL) {
        puts("already at event");
        return;
    }

    logger->current = logger->current->next;
    display_current(logger);
}

void move_prev(e_log *logger) {
    if (logger->current == NULL) {
        puts("no events to navigate");
        return;
    }

    if (logger->current->prev == NULL) {
        printf("already at old event");
        return;
    }

    logger->current = logger->current->prev;
    display_current(logger);
}

static void free_all_events(e_log *logger) {
    e_node *node = logger->head;
    e_node *next;

    while (node != NULL) {
        next = node->next;
        free(node);
        node = next;
    }

    logger->head = NULL;
    logger->tail = NULL;
    logger->current = NULL;
    logger->count = 0;
}

void clear_events(e_log *logger) {
    free_all_events(logger);
    puts("all events cleared");
}

void free_logger(e_log *logger) {
    free_all_events(logger);
}

static void save_session(const e_log *logger) {
    FILE *fp;
    e_node *e;

    fp = fopen("session.log", "w");
    if (fp == NULL) {
        puts("could not save in session.log");
        return;
    }

    fprintf(fp, "Embedded Device event Logger Session\n");
    fprintf(fp, "Stored events: %d\n\n", logger->count);

    for (e = logger->head; e != NULL; e = e->next) {
        fprintf(fp, "ID=%d | Type=%s | Sensor=%s | Timestamp=%s | Severity=%s\n",
                e->id, e->type, e->sensor_id, e->timestamp, e->severity);
    }

    fclose(fp);
    puts("session saved to session.log");
}

static void read_line(char *buf, size_t size) {
    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }

    buf[strcspn(buf, "\n")] = '\0';
}

static void make_timestamp(char *buf, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    if (tm_info == NULL) {
        strncpy(buf, "unknown", size - 1);
        buf[size - 1] = '\0';
        return;
    }

    strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

static void prompt_add_event(e_log *logger) {
    char type[lim];
    char sensor_id[lim];
    char severity[lim];
    char timestamp[lim];

    printf("event type: ");
    read_line(type, sizeof(type));
    printf("device ID: ");
    read_line(sensor_id, sizeof(sensor_id));
    printf("Severity: ");
    read_line(severity, sizeof(severity));

    if (type[0] == '\0' || sensor_id[0] == '\0' || severity[0] == '\0') {
        puts("invalid input");
        return;
    }

    make_timestamp(timestamp, sizeof(timestamp));
    add_event(logger, type, sensor_id, timestamp, severity);
}

void run_event_logger() {
    e_log logger;
    char command[16];

    init_logger(&logger);
    printf("Embedded Device event Logger\n");
    puts("commands: \n- n for next \n- p for previous" 
            "\n- a to add \n- c for current \n- x to clear \n- q for quit");

    while (1) {
        printf("\n~# ");
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        switch (command[0]) {
        case 'n':
            move_next(&logger);
            break;
        case 'p':
            move_prev(&logger);
            break;
        case 'a':
            prompt_add_event(&logger);
            break;
        case 'c':
            display_current(&logger);
            break;
        case 'x':
            clear_events(&logger);
            break;
        case 'q':
            puts("saving session .....");
            save_session(&logger);
            puts("Exiting.....");
            free_logger(&logger);
            return;
        default:
            printf("invalid command, use only n, p, a, c, x, or q");
            break;
        }
    }

    free_logger(&logger);
}
