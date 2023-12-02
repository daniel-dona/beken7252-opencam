# beken7252-opencam
RT-Thread alternative project for A9 cameras


# DISCLAIMER
This project is still in early stages and the time it will take to have an stable firmware is unknown. I'll do my best to give updates from time to time.

# Updates
## 20/11/2023
Last month, I was waiting for some additional A9 cameras to arrive, as I only had one, and I was concerned about ruining the PCB with this constant soldering and unsoldering of test wires.
Well, that didn't result as expected. I got like 12 different cameras as they were just like 3€ in Aliexpress, and I got:

~6 with a Beken chip

~3 with an unknown chip, that looks like some custom XRadio/Allwinner

~1 with other vendor that I don't remember

~1 with a chip with any markings

Yeah, the numbers don't sum up very well; the important thing is that I got more with the Beken chip... or is It? 

The chip is marked as BK7252, but the pinout is different. The chip that is documented and that we have at least a datasheet has 68 pints; this has 48 pins and the reference is BK7252UQN48. 

More here: https://github.com/daniel-dona/beken7252-opencam/wiki/BK7252-SoC-variations

I'm not going to work with this chip for the moment. I tried to upload a working code to this chip, and it is not working as expected. For sure, they removed some functionalities, or now they are muxed differently. Maybe memory regions for some registers are different, perhaps I'm stupid, and I uploaded the code wrong...

In any case, I think it is better to work first to have something working in the known chip, and then we can explore this new one. 



# build docker env

From the root folder of this repository

`make image`


# build project

`make build`



# TODO

- Document variants for PCB and sensors
- Document pinouts for camera variants
- Document flashing procedure (and unbricking)
- Cleanup Beken SDK
- Migrate SDK code to more modern GCC and Scons
- Other (?)
