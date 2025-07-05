#include <dt-bindings/zmk/behavior.h>

/ {
    behaviors {
        compatible = "zmk,behaviors";

        ht_auto_off: hold_tap_auto_off {
            compatible = "zmk,behavior-hold-tap-auto-off";
            label = "HT_AUTO_OFF";
            #binding-cells = <2>;
            bindings = <&kp>, <&mo>;
            child-layers = <0>;
            tapping-term-ms = <280>;
        };
    };
};