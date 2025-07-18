#include "app/object.h"
#include "drivers/vl53l0x.h"
#include "common/assert_handler.h"
#include "common/trace.h"

#define RANGE_DETECT_THRESHOLD (600u) // mm
#define INVALID_RANGE (UINT16_MAX)
#define RANGE_CLOSE (100u) // mm
#define RANGE_MID (200u) // mm
#define RANGE_FAR (300u) // mm

struct object object_get(void)
{
    struct object object = { object_POS_NONE, object_RANGE_NONE };
    vl53l0x_ranges_t ranges;
    bool fresh_values = false;
    vl53l0x_result_e result = vl53l0x_read_range_multiple(ranges, &fresh_values);
    if (result) {
        TRACE("read range failed %u", result);
        return object;
    }

    const uint16_t range_front = ranges[VL53L0X_IDX_FRONT];
    const uint16_t range_front_left = ranges[VL53L0X_IDX_FRONT_LEFT];
    const uint16_t range_front_right = ranges[VL53L0X_IDX_FRONT_RIGHT];
#if 0 // Skip left and right (badly mounted on the robot)
    const uint16_t range_left = ranges[VL53L0X_IDX_LEFT];
    const uint16_t range_right = ranges[VL53L0X_IDX_RIGHT];
#endif

    const bool front = range_front < RANGE_DETECT_THRESHOLD;
    const bool front_left = range_front_left < RANGE_DETECT_THRESHOLD;
    const bool front_right = range_front_right < RANGE_DETECT_THRESHOLD;
#if 0 // Skip left and right (badly mounted on the robot)
    const bool left = range_left < RANGE_DETECT_THRESHOLD;
    const bool right = range_right < RANGE_DETECT_THRESHOLD;
#endif

    uint16_t range = INVALID_RANGE;
#if 0 // Skip left and right (badly mounted on the robot)
    if (left) {
        if (front_right || right) {
            object.position = object_POS_IMPOSSIBLE;
        } else {
            object.position = object_POS_LEFT;
            range = range_left;
        }
    } else if (right) {
        if (front_left || left) {
            object.position = object_POS_IMPOSSIBLE;
        } else {
            object.position = object_POS_RIGHT;
            range = range_right;
        }
    }
#endif

    if (front_left && front && front_right) {
        object.position = object_POS_FRONT_ALL;
        // Average
        range = ((((range_front_left + range_front) / 2) + range_front_right) / 2);
    } else if (front_left && front_right) {
        object.position = object_POS_IMPOSSIBLE;
    } else if (front_left) {
        if (front) {
            object.position = object_POS_FRONT_AND_FRONT_LEFT;
            // Average
            range = (range_front_left + range_front) / 2;
        } else {
            object.position = object_POS_FRONT_LEFT;
            range = range_front_left;
        }
    } else if (front_right) {
        if (front) {
            object.position = object_POS_FRONT_AND_FRONT_RIGHT;
            // Average
            range = (range_front_right + range_front) / 2;
        } else {
            object.position = object_POS_FRONT_RIGHT;
            range = range_front_right;
        }
    } else if (front) {
        object.position = object_POS_FRONT;
        range = range_front;
    } else {
        object.position = object_POS_NONE;
    }

    if (range == INVALID_RANGE) {
        return object;
    }

    if (range < RANGE_CLOSE) {
        object.range = object_RANGE_CLOSE;
    } else if (range < RANGE_MID) {
        object.range = object_RANGE_MID;
    } else {
        object.range = object_RANGE_FAR;
    }

    return object;
}

bool object_detected(const struct object *object)
{
    return object->position != object_POS_NONE && object->position != object_POS_IMPOSSIBLE;
}

bool object_at_left(const struct object *object)
{
    return object->position == object_POS_LEFT || object->position == object_POS_FRONT_LEFT
        || object->position == object_POS_FRONT_AND_FRONT_LEFT;
}

bool object_at_right(const struct object *object)
{
    return object->position == object_POS_RIGHT || object->position == object_POS_FRONT_RIGHT
        || object->position == object_POS_FRONT_AND_FRONT_RIGHT;
}

bool object_at_front(const struct object *object)
{
    return object->position == object_POS_FRONT || object->position == object_POS_FRONT_ALL;
}

static bool initialized = false;
void object_init(void)
{
    ASSERT(!initialized);
    vl53l0x_result_e result = vl53l0x_init();
    if (result) {
        TRACE("Failed to initialize vl53l0x %u", result);
        return;
    }
    initialized = true;
}
