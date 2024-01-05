# ARDUINO BURGLAR ALARM

## Try it here:
https://www.tinkercad.com/things/gHybwDxUlSY-arduino-burglar-alarm?sharecode=FclphhJ4QrlBgkOZMehvzLW4_NQwQA5o1VYyD1b2tw4

### States:
- Blue led is on: alarm mode is activated
and device is monitoring surroundings
- Red led is on: alarm is triggered and it needs to be acknowledged
- Green led is on: alarm is deactivated

### Controllers:
- Slider toggles the state of the device
- Button acknowledges the alarm
- Potentiometer is used to set the length of the alarm tone,
- or silence it completely


### Logic:
- When the alarm mode is set, device monitors its surroundings.
- Distance sensor allows a small amount of movement. If values go over
set threshold, alarm is raised
- Triggered alarm is acknowledged by pressing a button. That sets a
new threshold, that upcoming values are compared to.