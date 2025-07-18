#ifndef STATE_approach_H
#define STATE_approach_H

#include "app/state_common.h"

// Drive towards detected enemy

typedef enum
{
    approach_STATE_FORWARD,
    approach_STATE_LEFT,
    approach_STATE_RIGHT
} approach_state_e;

struct state_approach_data
{
    const struct state_common_data *common;
    approach_state_e state;
};

void state_approach_init(struct state_approach_data *data);
void state_approach_enter(struct state_approach_data *data, state_e from, state_event_e event);

#endif // STATE_approach_H
