#include <zmk/behavior.h>
#include <zmk/keymap.h>
#include <zmk/behavior_helpers.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_ht_auto_off_config {
    struct zmk_behavior_binding a_binding;
    struct zmk_behavior_binding b_binding;
    uint8_t child_layers[10];
    uint8_t child_layers_len;
    int32_t tapping_term_ms;
};

struct behavior_ht_auto_off_data {
    bool is_hold;
    int64_t release_at;
};

static int on_ht_auto_off_binding_pressed(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_ht_auto_off_config *cfg = dev->config;
    struct behavior_ht_auto_off_data *data = dev->data;

    data->release_at = event.timestamp + cfg->tapping_term_ms;
    data->is_hold = false;
    
    
    int err = k_work_reschedule(&dev->dwork, K_MSEC(cfg->tapping_term_ms));
    __ASSERT(err >= 0, "Failed to schedule hold-tap timer work: %d", err);

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_ht_auto_off_binding_released(struct zmk_behavior_binding *binding, struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_ht_auto_off_config *cfg = dev->config;
    struct behavior_ht_auto_off_data *data = dev->data;

    // Cancel the timer
    k_work_cancel_delayable(&dev->dwork);

    if (data->is_hold) {
        zmk_behavior_binding_released(&cfg->b_binding, event);
        
        for (int i = 0; i < cfg->child_layers_len; i++) {
            zmk_keymap_layer_off(cfg->child_layers[i]);
        }
    } else {
        zmk_behavior_binding_pressed(&cfg->a_binding, event);
        zmk_behavior_binding_released(&cfg->a_binding, event);
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static void ht_auto_off_timer_work_handler(struct k_work *work) {
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);
    struct zmk_behavior_binding_event event = { .timestamp = k_uptime_get() };
    const struct device *dev = CONTAINER_OF(dwork, struct device, dwork);
    const struct behavior_ht_auto_off_config *cfg = dev->config;
    struct behavior_ht_auto_off_data *data = dev->data;

    data->is_hold = true;
    zmk_behavior_binding_pressed(&cfg->b_binding, event);
}

static int behavior_ht_auto_off_init(const struct device *dev) {
    struct behavior_ht_auto_off_data *data = dev->data;
    k_work_init_delayable(&dev->dwork, ht_auto_off_timer_work_handler);
    return 0;
}

static const struct zmk_behavior_api behavior_ht_auto_off_api = {
    .binding_pressed = on_ht_auto_off_binding_pressed,
    .binding_released = on_ht_auto_off_binding_released,
};

#define ZMK_BHV_HT_AUTO_OFF_CHILD_LAYERS_PROP(n)                                                     \
    .child_layers_len = DT_INST_PROP_LEN_OR(n, child_layers, 0),                                     \
    .child_layers = { DT_INST_PROP_OR(n, child_layers, {0}) },

#define HT_AUTO_OFF_INST(n)                                                                     \
    static struct behavior_ht_auto_off_data behavior_ht_auto_off_data_##n;                      \
    static struct behavior_ht_auto_off_config behavior_ht_auto_off_config_##n = {               \
        ZMK_BHV_HT_AUTO_OFF_CHILD_LAYERS_PROP(n)                                                 \
        .tapping_term_ms = DT_INST_PROP_OR(n, tapping_term_ms, 280),                             \
        .a_binding = ZMK_KEYMAP_EXTRACT_BINDING(0, DT_DRV_INST(n)),                              \
        .b_binding = ZMK_KEYMAP_EXTRACT_BINDING(1, DT_DRV_INST(n)),                              \
    };                                                                                          \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_ht_auto_off_init, NULL, &behavior_ht_auto_off_data_##n,  \
                          &behavior_ht_auto_off_config_##n, POST_KERNEL,                        \
                          CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_ht_auto_off_api);

DT_INST_FOREACH_STATUS_OKAY(HT_AUTO_OFF_INST)