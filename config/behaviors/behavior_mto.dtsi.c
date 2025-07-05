#include <dt-bindings/zmk/behavior.h>

/ {
    behaviors {
        compatible = "zmk,behaviors";

        mto: mto {
            compatible = "zmk,behavior-layer-to";
            label = "MTO";
            #binding-cells = <1>;
            bindings = <&kp>;
        };
    };
};