#include "app/state_approach.h"
#include "app/drive.h"
#include "app/timer.h"
#include "common/assert_handler.h"

#define approach_STATE_TIMEOUT (5000u)

static void state_approach_run(const struct state_approach_data *data)
{
    switch (data->state) {
    case approach_STATE_FORWARD:
        drive_set(DRIVE_DIR_FORWARD, DRIVE_SPEED_FAST);
        break;
    case approach_STATE_LEFT:
        drive_set(DRIVE_DIR_ARCTURN_WIDE_LEFT, DRIVE_SPEED_FAST);
        break;
    case approach_STATE_RIGHT:
        drive_set(DRIVE_DIR_ARCTURN_WIDE_RIGHT, DRIVE_SPEED_FAST);
        break;
    }
    timer_start(data->common->timer, approach_STATE_TIMEOUT);
}

static approach_state_e next_approach_state(const struct object *object)
{
    if (object_at_front(object)) {
        return approach_STATE_FORWARD;
    } else if (object_at_left(object)) {
        return approach_STATE_LEFT;
    } else if (object_at_right(object)) {
        return approach_STATE_RIGHT;
    } else {
        ASSERT(0);
    }
    return approach_STATE_FORWARD;
}

// No blocking code (e.g. busy wait) allowed in this function
void state_approach_enter(struct state_approach_data *data, state_e from, state_event_e event)
{
    const approach_state_e prev_approach_state = data->state;
    data->state = next_approach_state(&data->common->object);

    switch (from) {
    case STATE_SEARCH:
        switch (event) {
        case STATE_EVENT_object:
            state_approach_run(data);
            break;
        case STATE_EVENT_TIMEOUT:
        case STATE_EVENT_LINE:
        case STATE_EVENT_FINISHED:
        case STATE_EVENT_COMMAND:
        case STATE_EVENT_NONE:
            ASSERT(0);
            break;
        }
        break;
    case STATE_approach:
        switch (event) {
        case STATE_EVENT_object:
            if (prev_approach_state != data->state) {
                state_approach_run(data);
            }
            break;
        case STATE_EVENT_TIMEOUT:
            // TODO: Consider adding a breakout strategy instead of asserting
            ASSERT(0);
            break;
        case STATE_EVENT_LINE:
        case STATE_EVENT_FINISHED:
        case STATE_EVENT_COMMAND:
        case STATE_EVENT_NONE:
            ASSERT(0);
            break;
        }
        break;
    case STATE_RETREAT:
        // Should always go via search state
        ASSERT(0);
        break;
    case STATE_WAIT:
    case STATE_MANUAL:
        ASSERT(0);
        break;
    }
}

void state_approach_init(struct state_approach_data *data)
{
    data->state = approach_STATE_FORWARD;
}
