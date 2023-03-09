# CS488 Winter 2023 Assignment 3

## Compilation
No changes were made to the default premake4/make combination.

No lab machine was used to test the code.

The code was tested on Thinkpad T14 Gen 2 with Windows 10 running the VM.

## Additional information
The extra feature is random sampling, with 16 random rays issued for each pixel. In `A4.hpp` there is a switch `ENABLE_RANDOM_SAMPLING` that is turned on by default.

All the images are in `Assets/Images/` directory.

The unique rendered scene is 512*512 pixels called `sample.png`, which comes with another image `sample-noSampling.png` to distinguish whether random sampling is turned on.

Bounding volume demonstration is done with `nonhier-bb.png`.
