#include <stdlib.h>
#include <fsm.h>

#define ST_CAP 7
#define EV_CAP 7

enum state {
    STATE_IMPOSSIBLE, // first state always should be impossible!
    STATE_READ_INTEGER,
    STATE_READ_DECIMAL,
    STATE_ERROR
} typedef state;

enum event_type {
    EVENT_DIGITAL,
    EVENT_SEPARATOR,
    EVENT_ERROR
} typedef event_type;

struct context {
    int divident;
    int divisor;
};

int find_event_type(void *);
void integer_callback(void *, void *);
void decimal_callback(void *, void *);

FSM_REG_BEGIN(STUPID_MACHINE, find_event_type, ST_CAP, EV_CAP)
FSM_REG_TRANSITION(EVENT_DIGITAL, integer_callback, STATE_READ_INTEGER, STATE_READ_INTEGER)
FSM_REG_TRANSITION(EVENT_SEPARATOR, NULL, STATE_READ_INTEGER, STATE_READ_DECIMAL)
FSM_REG_TRANSITION(EVENT_DIGITAL, decimal_callback, STATE_READ_DECIMAL, STATE_READ_DECIMAL)
FSM_REG_END()

int main() {
    state_machine *sm = malloc(sizeof(state_machine));

    struct context ctx = {0,1};
    FSM_INIT(STUPID_MACHINE, sm, &ctx, STATE_READ_INTEGER);

    char *payload = "228";
    for (int i = 0; i < 3; i++) {
        sm->push_ev_payload(sm, &payload[i]);
    }
    printf("Result: %d/%d\n", ctx.divident, ctx.divisor);
    free(sm);
    return 0;
}

int find_event_type(void *raw_payload) {
    char payload = * (char *)raw_payload;
    if ((payload >= '0') && (payload <= '9')) return EVENT_DIGITAL;
    if ((payload > ',') || (payload == '.')) return EVENT_SEPARATOR;
    return EVENT_ERROR;
}

void integer_callback(void *raw_ctx, void *raw_payload) {
    struct context *ctx = (struct context *) raw_ctx;
    int digit = (*(char *)raw_payload - '0');

    ctx->divident = ctx->divident*10 + digit;
}

void decimal_callback(void *raw_ctx, void *raw_payload) {
    integer_callback(raw_ctx, raw_payload);
    struct context *ctx = (struct context *) raw_ctx;
    ctx->divisor *= 10;
}