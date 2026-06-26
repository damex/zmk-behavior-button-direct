// Copyright 2026 Roman Kuzmitskii (@damex)
// SPDX-License-Identifier: MIT

/*
 * Drain-aware HID button behavior. Stuck-count rationale and usage in README.
 * ZMK explicit_button_counts (app/src/hid.c) clears HID bit only at count 0,
 * so lost release leaves it stuck above 0. Tracks own press count, force-drains
 * to 0 on release, so next click recovers.
 * ZMK clamps release at 0, so drain stays safe.
 */
#define DT_DRV_COMPAT zmk_behavior_button_direct

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <drivers/behavior.h>
#include <dt-bindings/zmk/pointing.h>
#include <zmk/behavior.h>
#include <zmk/endpoints.h>
#include <zmk/hid.h>

LOG_MODULE_REGISTER(behavior_button_direct, CONFIG_ZMK_BEHAVIOR_BUTTON_DIRECT_LOG_LEVEL);

static uint16_t button_counts[ZMK_HID_MOUSE_NUM_BUTTONS];

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);
    const uint32_t buttons_mask = binding->param1;
    for (uint16_t bit = 0; bit < ZMK_HID_MOUSE_NUM_BUTTONS; bit++) {
        if ((buttons_mask & BIT(bit)) == 0) {
            continue;
        }
        zmk_hid_mouse_button_press(bit);
        button_counts[bit]++;
    }
    zmk_endpoint_send_mouse_report();
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    ARG_UNUSED(event);
    const uint32_t buttons_mask = binding->param1;
    for (uint16_t bit = 0; bit < ZMK_HID_MOUSE_NUM_BUTTONS; bit++) {
        if ((buttons_mask & BIT(bit)) == 0) {
            continue;
        }
        for (uint16_t press = 0; press < button_counts[bit]; press++) {
            zmk_hid_mouse_button_release(bit);
        }
        button_counts[bit] = 0;
    }
    zmk_endpoint_send_mouse_report();
    return ZMK_BEHAVIOR_OPAQUE;
}

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
static const struct behavior_parameter_value_metadata button_values[] = {
    {.display_name = "MB1", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = MB1},
    {.display_name = "MB2", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = MB2},
    {.display_name = "MB3", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = MB3},
    {.display_name = "MB4", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = MB4},
    {.display_name = "MB5", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE, .value = MB5},
};
BUILD_ASSERT(ARRAY_SIZE(button_values) == ZMK_HID_MOUSE_NUM_BUTTONS,
             "metadata must list every mouse button");

static const struct behavior_parameter_metadata_set button_metadata_set[] = {{
    .param1_values = button_values,
    .param1_values_len = ARRAY_SIZE(button_values),
}};

static const struct behavior_parameter_metadata button_metadata = {
    .sets_len = ARRAY_SIZE(button_metadata_set),
    .sets = button_metadata_set,
};
#endif

static const struct behavior_driver_api button_direct_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &button_metadata,
#endif
};

#define BUTTON_DIRECT_INST(instance)                                                                \
    BEHAVIOR_DT_INST_DEFINE(instance, NULL, NULL, NULL, NULL, POST_KERNEL,                           \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &button_direct_api);

DT_INST_FOREACH_STATUS_OKAY(BUTTON_DIRECT_INST)
