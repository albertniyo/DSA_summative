#ifndef MAIN_H
#define MAIN_H

#define max_event 25
#define lim 64

typedef struct event {
    int id;
    char type[lim];
    char sensor_id[lim];
    char timestamp[lim];
    char severity[lim];
    struct event *prev;
    struct event *next;
} e_node;

typedef struct {
    e_node *head;
    e_node *tail;
    e_node *current;
    int count;
    int next_id;
} e_log;

void init_logger(e_log *logger);
int add_event(e_log *logger, const char *type, const char *sensor_id,
              const char *timestamp, const char *severity);
void display_current(const e_log *logger);
void move_next(e_log *logger);
void move_prev(e_log *logger);
void clear_events(e_log *logger);
void free_logger(e_log *logger);
void run_event_logger();

#endif
