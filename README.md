# zmk-behavior-button-direct

Drain-aware ZMK behavior for HID buttons. Fixes stuck-button class where a release
event lost over unreliable transport (e.g. ESB under radio interference) leaves ZMK
HID press count stuck >0 with no recovery via clicks (each press + release nets to
zero, so clicking again never zeros count). On every release, drains count to zero,
so next release after a lost one self-heals stuck state instead of leaving button
held until power-cycle.

ZMK currently exposes HID buttons only through mouse report (Button Page 0x09), so
implementation calls `zmk_hid_mouse_button_*`. Drain pattern extends to other HID
button targets (joystick, gamepad) when ZMK adds them.

## Install

Add it to your `config/west.yml`:
```yaml
  remotes:
    - name: damex
      url-base: https://github.com/damex
  projects:
    - name: zmk-behavior-button-direct
      remote: damex
      revision: v0.1.0
```
For a local checkout, build with
`-DZMK_EXTRA_MODULES=<path>/zmk-behavior-button-direct` instead.

## Configure

Declare the behavior in your keymap and swap `&mkp` for it:
```dts
/ {
    behaviors {
        bd: behavior_button_direct {
            compatible = "zmk,behavior-button-direct";
            #binding-cells = <1>;
        };
    };

    keymap {
        compatible = "zmk,keymap";
        default_layer {
            bindings = <
                &bd LCLK   /* was &mkp LCLK */
                &bd RCLK   /* was &mkp RCLK */
                &bd MCLK   /* was &mkp MCLK */
                /* ... */
            >;
        };
    };
};
```

`param1` is HID button bit mask, identical to `&mkp`'s param: `LCLK=BIT(0)`,
`RCLK=BIT(1)`, `MCLK=BIT(2)`, `MB4=BIT(3)`, `MB5=BIT(4)`. Symbolic names from
`<dt-bindings/zmk/pointing.h>` (`LCLK`/`RCLK`/`MCLK`/`MB1`..`MB5`) work the same.
Masks may be OR'd (`&bd (LCLK|RCLK)`).

## Limitations

- Loses ZMK multi-source-hold semantic: single release fully releases button.
  Fine when each HID button has one source (one physical switch). If keymap layer
  and click both target same button at once, one release releases both. `&mkp`
  stays available for cases that need cumulative holds.

## License

This module is MIT.

Dependencies (each keeps its own license):

| Dependency | License |
|---|---|
| ZMK | MIT |
| Zephyr | Apache-2.0 |
