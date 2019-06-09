#include <stdio.h>

// #define DEBUG

#if defined DEBUG
#define LOG(fmt, ...)    printf("DEBUG: " #fmt " \n", __VA_ARGS__);
#else
#define LOG(fmt, ...)
#endif

typedef void (*ev_callback) (void *context, void *event_payload);

struct transition {
    int next_state;
    ev_callback callback;
} typedef transition;

struct state_machine {
    int state;
    int state_cap;
    void *context;
    transition *transitions; // manual indexation
    int (*find_ev_type) (void *);
    int (*push_ev) (struct state_machine *sm, int event);
    int (*push_ev_payload) (struct state_machine *sm, void *payload);
} typedef state_machine;

int push_ev(struct state_machine *sm, int event) {
    transition tr = sm->transitions[sm->state*sm->state_cap + event];
    if (tr.next_state == 0) return -1;

    LOG("push event: %d", event);
    sm->state = tr.next_state;
    if (tr.callback != NULL) {   
        tr.callback(sm->context, NULL);
    }
    return tr.next_state;
}

int push_ev_payload(struct state_machine *sm, void *ev_payload) {
    if (sm->find_ev_type == NULL) return -1;

    int event = sm->find_ev_type(ev_payload);
    LOG("push payloaded event: %d", event);
    if (event < 0) return -1;

    transition tr = sm->transitions[sm->state*sm->state_cap + event];
    if (tr.next_state == 0) return -1;

    sm->state = tr.next_state;
    if (tr.callback != NULL) {   
        tr.callback(sm->context, ev_payload);
    }
    return tr.next_state;
}

#define FSM_REG_BEGIN(name, _find_ev_type, _state_cap, event_cap) \
static transition name##_transitions[_state_cap*event_cap]; \
\
void name##_init(state_machine *sm, void *context, int start_state) { \
    sm->context = context; \
    sm->state = start_state; \
    sm->state_cap = _state_cap; \
    sm->push_ev = push_ev; \
    sm->find_ev_type = _find_ev_type; \
    sm->push_ev_payload = push_ev_payload; \
    sm->transitions = name##_transitions; \
    static int called; \
    if (called) return; \
    called = 1; \
    transition *tr;

#define FSM_REG_TRANSITION(event, _callback, _curr_state, _next_state) \
    LOG("Register %s->%s", #_curr_state, #_next_state);\
    tr = &(sm->transitions[_curr_state*sm->state_cap+event]); \
    tr->next_state = _next_state; \
    tr->callback = _callback;

#define FSM_REG_END() }

#define FSM_INIT(name, sm, context, state) name##_init(sm, context, state)
