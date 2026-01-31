# Trigger Modes
Activate a trigger mode by
```
v4l2-ctl -c trigger_mode=<trigger mode number>
```
The trigger mode remains set until it is deactivated with 
```
v4l2-ctl -c trigger_mode=0
```
Following you will find timing diagrams to illustrate the specific behavior of each mode.
## External, pulse width and overlap trigger mode (1, 2 and 8)
![External trigger mode](../doc/plantuml/tm_external.svg)

Regarding the exposure the overlap trigger mode(8) is similar to external trigger mode(1), but allows higher exposure times without reducing the frame rate.<br>

### Note
#### Mode 1:
* Tmin: The time between two successive exposure phases must not be shorter than the image transmission time. This limits the exposure time to a maximum value that is less than the frame time.
#### Mode 1 and 2:
* There is a constant delay of 5µs between the trigger in and the flash out signal due to the fpga, not shown in the image above.
#### Mode 8:
* Tmin: Image transmission occurs during the exposure time (overlapping). For this reason, the exposure time can be equal to the frame time.
* Instead of 5µs delay between the trigger in and the flash out signal, there is a jitter depending on the format/lane combination.

## Self trigger mode (3)
This feature uses the retrigger period register (reg13-16) to emulate a self-triggered mode that looks like the native streaming mode but with nanosecond accurate shutter speed setting and flash trigger output. The external trigger hardware signal is disabled in this mode. 

Be aware that the maximum framerate depends on the readout time of the sensor PLUS the shutter time in this mode (in contrast to the streaming mode where it is possible to expose the sensor PARALLEL to readout)

![Self trigger mode](../doc/plantuml/tm_self.svg)

## Single trigger mode (4)
Trigger the image acquisition by executing 
```
v4l2-ctl -c single_trigger=1
```
![Single trigger mode](../doc/plantuml/tm_single.svg)

## Self and sync trigger mode (3 and 5)
This mode is used to synchronise two or more sensor modules using a master/slave synchronisation.

Be aware of that you have to enable the flash output of the master sensor. Connect flash output of the master to trigger input of the slave sensor modules.

![Self and sync trigger mode](../doc/plantuml/tm_masterslave.svg)

## Stream edge trigger mode (6)
![Stream edge trigger mode](../doc/plantuml/tm_stream_edge.svg)

## Stream level trigger mode (7)
![Stream level trigger mode](../doc/plantuml/tm_stream_level.svg)

## Support for trigger modes
In the table below you can find, which camera supports which trigger mode.

| cameras | 1: external | 2: pulsewidth | 3: self | 4: single | 5: sync | 6: stream_edge | 7: stream_level | 8: overlap |
| ------ | --- | --- | --- | --- | --- | --- | --- | --- |
| IMX178 | yes |   - | yes | yes | yes |   - |   - |   - |
| IMX183 | yes |   - | yes | yes | yes |   - |   - |   - |
| IMX226 | yes |   - | yes | yes | yes | yes | yes |   - |
| IMX250 | yes | yes | yes | yes |   - |   - |   - |   - |
| IMX290 |   - |   - |   - |   - |   - |   - |   - |   - |
| IMX252 | yes | yes | yes | yes |   - |   - |   - | yes |
| IMX264 | yes | yes | yes | yes |   - |   - |   - |   - |
| IMX265 | yes | yes | yes | yes |   - |   - |   - |   - |
| IMX273 | yes | yes | yes | yes |   - |   - |   - |   - |
| IMX296 | yes | yes | yes |   - |   - |   - |   - |   - |
| IMX297 | yes | yes | yes |   - |   - |   - |   - |   - |
| IMX327 |   - |   - |   - |   - |   - |   - |   - |   - |
| IMX335 |   - |   - |   - |   - |   - |   - |   - |   - |
| IMX392 | yes | yes | yes | yes |   - |   - |   - |   - |
| IMX412 |   - |   - |   - |   - | yes |   - |   - |   - |
| IMX415 |   - |   - |   - |   - |   - |   - |   - |   - |
| IMX462 |   - |   - |   - |   - |   - |   - |   - |   - |
| IMX565 | yes | yes | yes | yes |   - |   - |   - |   - |
| IMX566 | yes | yes | yes | yes |   - |   - |   - |   - |
| IMX567 | yes | yes | yes | yes |   - |   - |   - |   - |
| IMX568 | yes | yes | yes | yes |   - |   - |   - |   - |
| IMX585 |   - |   - |   - |   - |   - |   - |   - |   - |
| OV7251 |   - |   - |   - |   - |   - |   - |   - |   - |
| OV9281 |   - |   - |   - |   - |   - |   - |   - |   - |

#### Note
The overlap trigger jitter for IMX252 is between 11.7µs and 30.5µs.
