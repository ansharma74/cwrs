#ifndef LXNM_TIMER_H
#define LXNM_TIMER_H

#define LXNM_TIMER_DELAY 1000

typedef struct {
	gint id;
	void (*func)(void);
} LXNMTimerTask;

gint lxnm_timer_add(void *func);
void lxnm_timer_remove(gint id);
void lxnm_timer_init();

#endif
