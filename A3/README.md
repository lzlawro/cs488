# CS488 Winter 2023 Assignment 3

## Compilation
No changes were made to the default premake4/make combination.

No lab machine was used to test the code.

The code was tested on Thinkpad T14 Gen 2 with Windows 10 running the VM.

## Additional information
Changes to existing data structure:
- Added fields that represent the current rotation angles of x and y-axes for `JointNode`.
- Added an override for the `rotate` method in `JointNode` that compares between current rotation angles and rotation limits.

Hierarchical model tree:
```
                                rightHandJoint -- rightHand
                                |
                        rightLowerArmJoint -- rightLowerArm
                        |
                rightUpperArmJoint -- rightUpperArmJoint
                |
                |                               leftHandJoint -- leftHand
                |                               |
                |               leftLowerArmJoint -- leftLowerArm
                |               |
                leftUpperArmJoint -- leftUpperArm
                |
    shouldersJoint -- shoulders
    |
rootNode -- torso
    |
    hipJoint -- hip
        |
        leftUpperLegJoint -- leftUpperLeg
        |               |
        |               leftLowerLegJoint -- leftUpperLeg
        |                               |
        |                               leftFootJoint -- leftUpperLeg
        |
        rightUpperLegJoint -- rightUpperLeg
               |
               rightLowerLegJoint -- rightLowerLeg
                       |
                       rightFootJoint -- rightFoot
```
