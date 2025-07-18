#include "app/input_history.h"
#include "common/ring_buffer.h"

static bool input_equal(const struct input *a, const struct input *b)
{
    return a->line == b->line && a->object.position == b->object.position
        && a->object.range == b->object.range;
}

void input_history_save(struct ring_buffer *history, const struct input *input)
{
    // Skip if no input detected
    if (input->object.position == object_POS_NONE && input->line == LINE_NONE) {
        return;
    }

    // Skip if identical input detected
    if (ring_buffer_count(history)) {
        struct input last_input;
        ring_buffer_peek_head(history, &last_input, 0);
        if (input_equal(input, &last_input)) {
            return;
        }
    }

    ring_buffer_put(history, input);
}

struct object input_history_last_directed_object(const struct ring_buffer *history)
{
    for (uint8_t offset = 0; offset < ring_buffer_count(history); offset++) {
        struct input input;
        ring_buffer_peek_head(history, &input, offset);
        if (object_at_left(&input.object) || object_at_right(&input.object)) {
            return input.object;
        }
    }
    const struct object object_none = { object_POS_NONE, object_RANGE_NONE };
    return object_none;
}
