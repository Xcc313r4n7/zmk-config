#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>
#include <zmk/matrix.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_mto_config {
    uint8_t layer;
};

static int behavior_mto_init(const struct device *dev) { return 0; };

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_mto_config *cfg = dev->config;
    LOG_DBG("layer %d", cfg->layer);
    return zmk_keymap_layer_to(cfg->layer);
}

static const struct zmk_behavior_api behavior_mto_api = {
    .binding_pressed = on_keymap_binding_pressed,
};

#define MTO_INST(n)                                                                                \
    static struct behavior_mto_config behavior_mto_config_##n = {.layer = DT_INST_PROP(n, layer)};  \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_mto_init, NULL, NULL, &behavior_mto_config_##n,             \
                          APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_mto_api);

DT_INST_FOREACH_STATUS_OKAY(MTO_INST)