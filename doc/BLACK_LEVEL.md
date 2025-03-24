# Black level
Adds an offset to the image output by setting
```
v4l2-ctl -d /dev/video0 -c black_level=<value>
```
where the value goes from 0 to 100000. This value is interpreted as milli% or m%.

A blacklevel value of 0 means 0%, there will be no offset added to the pixel output.
And a value of 100000 would add the maximum possible offset to the pixel data.
Every value between 0 and 100000 [m%] will be interpreted as a relative value in respect
to the maximal possible value of the sensors black level register. 
E.g. black_level=5859 (5.859%) will be correlating with an offset of 15 out of 256 possible
values for the 8-Bit mode. The same relation would apply to 60/1024 or 240/4096 for the 
10-Bit or the 12-Bit mode, respectively. The advantage of this relative property is that 
there is no need to set up a new absolute value when switching between different Bit-modes.

With initialization of the driver, an optimal default value according to the manufacturers
proposal will be set. Unfortunately the v4l2-ctl won't reflect the initial value until it 
is modified manually by the user. That means if the driver is starting up, the sensors 
default value is set, depending on the given Bit-mode, but the
```
v4l2-ctl -l
```
call would show a black_level value of 0. For the time being there is no appropriate solution
for this behaviour, because the initial value of this control is being held in the tegracam_ctrls.c
