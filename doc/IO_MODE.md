# IO Modes
Activate an IO mode by
```
v4l2-ctl -c io_mode=<IO mode number>
```
The IO mode remains set until it is deactivated with 
```
v4l2-ctl -c io_mode=0
```
Following you will find timing diagrams to illustrate the specific behavior of each mode.

In all modes you can activate a trigger mode as described in [Trigger Modes](TRIGGER_MODE.md). 

## Disabled (0)
In this mode the flash signal is deactivated and the trigger signal is active high.

![Disabled](../doc/plantuml/iom_disabled.svg)

## Flash active high (1)
In this mode the flash signal is active high and the trigger signal is active high.

![Flash active high](../doc/plantuml/iom_flash_active_high.svg)

## Flash active low (2)
In this mode the flash signal is active low and the trigger signal is active high.

![Flash active low](../doc/plantuml/iom_flash_active_low.svg)

## Trigger active low (3)
In this mode the flash signal is deactivated and the trigger signal is active low.

![Trigger active low](../doc/plantuml/iom_trigger_active_low.svg)

## Trigger active low and Flash active high (4)
In this mode the flash signal is active high and the trigger signal is active low.

![Trigger active low and Flash active high](../doc/plantuml/iom_trigger_low_flash_high.svg)

## Trigger and Flash active low (5)
In this mode the flash signal is active low and the trigger signal is active low.

![Trigger and Flash active low](../doc/plantuml/iom_trigger_flash_active_low.svg)

## Support for flash signal
In the table below you can find, which camera supports which flash signal in IO Mode (1)

| cameras | rev | STREAM       | TRIGGER      |
| ------  | --- | -------------| ------------ |
| IMX178  |  02 | xvs H 110 ns |  H exp. time |
| IMX183  |  15 | xvs H 110 ns |  H exp. time |
| IMX226  |  16 | xvs H 110 ns |  H exp. time |
| IMX250  |  09 |     H 177 us |  H exp. time |
| IMX252  |  12 |     H 150 us |  H exp. time |
| IMX264  |  05 |     H 270 us |  H exp. time |
| IMX265  |  05 |           no |  H exp. time |
| IMX273  |  16 |     H 113 us |  H exp. time |
| IMX290  |  02 |         n.a. |         n.a. |
| IMX296  |  43 |     H 3.5 ms |  H exp. time |
| IMX297  |  43 |     H 3.5 ms |  H exp. time |
| IMX327  |  02 |         n.a. |         n.a. |
| IMX335  |  02 |     L 7.5 us |         n.a. |
| IMX392  |  08 |       143 us |  H exp. time |
| IMX412  |  05 | xvs L 110 ns |         n.a. |
| IMX415  |  02 |     L  14 us |         n.a. |
| IMX462  |  01 |         n.a. |         n.a. |
| IMX565  |  03 |  H exp. time |  H exp. time | 
| IMX566  |  03 |  H exp. time |  H exp. time |
| IMX567  |  03 |  H exp. time |  H exp. time |
| IMX568  |  04 |  H exp. time |  H exp. time |
| OV7281  |  01 |           no |         n.a. |
| OV9281  |  03 |  H exp. time |         n.a. |