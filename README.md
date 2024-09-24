# Arduino Darkroom Timer

![](doc/DSC_5918.JPG)  

https://github.com/user-attachments/assets/7a2ae1d2-c87a-452d-9ca1-876dc4518d32



Uses the Gamepad Input shield and Arduino Uno R4. Wiring info is in the src.  
Up/down button/joystick changes task, and left/right button/joystick changes state/setting.  

With the first 8x8 LED block to illuminate the darkroom.  

Implements 5 tasks
- LEDBrightnessTask
- EnlargerOnOffTask
- EnlargerTimerTask
- DevelopStopFixTask
- GeneralTimerTask

![](doc/index0.JPG)  

## LEDBrightnessTask
Sets the brightness of the 8x8 darkroom illumination LEDs from min 0 to max 15.

![](doc/index1.JPG)  

## EnlargerOnOffTask
Manual enlarger switch switched on.

![](doc/index2.JPG)  

## EnlargerTimerTask
Timed enlarger switch.

![](doc/index3.JPG)  

## DevelopStopFixTask
3 timers for develop, stop bath and fix.

![](doc/index4.JPG)  

## GeneralTimerTask
General timer starting from 0s.

![](doc/index5.JPG)  

