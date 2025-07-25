#include "app/state_search.h"
#include "app/drive.h"
#include "app/timer.h"
#include "app/input_history.h"
#include "common/assert_handler.h"

#define SEARCH_STATE_ROTATE_TIMEOUT (400u)
#define SEARCH_STATE_FORWARD_TIMEOUT (3000u)

static void state_search_run(struct state_search_data *data)
{
    switch (data->state) {
    case SEARCH_STATE_ROTATE:
    {
        // Rotate to where object was last seen
        const struct object last_object =
            input_history_last_directed_object(data->common->input_history);
        if (object_at_right(&last_object)) {
            drive_set(DRIVE_DIR_ROTATE_RIGHT, DRIVE_SPEED_FAST);
        } else {
            drive_set(DRIVE_DIR_ROTATE_LEFT, DRIVE_SPEED_FAST);
        }
        timer_start(data->common->timer, SEARCH_STATE_ROTATE_TIMEOUT);
    } break;
    case SEARCH_STATE_FORWARD:
        drive_set(DRIVE_DIR_FORWARD, DRIVE_SPEED_FAST);
        timer_start(data->common->timer, SEARCH_STATE_FORWARD_TIMEOUT);
        break;
    }
}

// No blocking code (e.g. busy wait) allowed in this function
void state_search_enter(struct state_search_data *data, state_e from, state_event_e event)
{
    switch (from) {
    case STATE_WAIT:
        ASSERT(event == STATE_EVENT_COMMAND);
        state_search_run(data);
        break;
    case STATE_approach:
        // fall-through
    case STATE_RETREAT:
        switch (event) {
        case STATE_EVENT_NONE:
            ASSERT(from == STATE_approach);
            state_search_run(data);
            break;
        case STATE_EVENT_FINISHED:
            ASSERT(from == STATE_RETREAT);
            // Switch state to avoid getting stuck driving back and forth when object lost
            if (data->state == SEARCH_STATE_FORWARD) {
                data->state = SEARCH_STATE_ROTATE;
            }
            state_search_run(data);
            break;
        case STATE_EVENT_COMMAND:
        case STATE_EVENT_TIMEOUT:
        case STATE_EVENT_LINE:
        case STATE_EVENT_object:
            ASSERT(0);
            break;
        }
        break;
    case STATE_SEARCH:
        switch (event) {
        case STATE_EVENT_NONE:
            break;
        case STATE_EVENT_TIMEOUT:
            switch (data->state) {
            case SEARCH_STATE_ROTATE:
                data->state = SEARCH_STATE_FORWARD;
                break;
            case SEARCH_STATE_FORWARD:
                data->state = SEARCH_STATE_ROTATE;
                break;
            }
            state_search_run(data);
            break;
        case STATE_EVENT_FINISHED:
        case STATE_EVENT_LINE:
        case STATE_EVENT_object:
        case STATE_EVENT_COMMAND:
            ASSERT(0);
            break;
        }
        break;
    case STATE_MANUAL:
        ASSERT(0);
        break;
    }
}

void state_search_init(struct state_search_data *data)
{
    data->state = SEARCH_STATE_ROTATE;
}
