-- Create the top level root node named 'root'.
root = gr.node('root')

root:translate(0.0, 0.25, -2.5)

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
darkRed = gr.material({0.1, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
yellow = gr.material({1.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
black = gr.material({0.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

-- cubeMesh = gr.mesh('cube', 'name-of-cube')
-- cubeMesh:scale(1.0, 1.0, 1.0)
-- cubeMesh:rotate('y', 45.0)
-- cubeMesh:translate(0.0, -0.5, -5.0)
-- cubeMesh:set_material(gr.material({0.8, 0.2, 0.2}, {0.8, 0.8, 0.8}, 10.0))

-- root:add_child(cubeMesh)

-- sphereMesh = gr.mesh('sphere', 'name-of-sphere')
-- sphereMesh:scale(0.5, 0.5, 0.5)
-- sphereMesh:translate(0.0, 0.5, -5.0)
-- sphereMesh:set_material(gr.material({0.4, 0.8, 0.4}, {0.8, 0.8, 0.8}, 50.0))

-- root:add_child(sphereMesh)

-- ----------------------------------------------------------------------------------------
-- Torso
torso = gr.mesh('sphere', 'torso')
torso:scale(0.25, 0.4, 0.15)
torso:set_material(white)

root:add_child(torso)

-- ----------------------------------------------------------------------------------------
-- Shoulders
shouldersJoint = gr.joint('shouldersJoint', {-20, 0, 20}, {-20, 0, 20})
shouldersJoint:translate(0.0, 0.4, 0.0)
root:add_child(shouldersJoint)

shoulders = gr.mesh('sphere', 'shoulders')
shoulders:scale(0.35, 0.1, 0.15)
shoulders:set_material(yellow)

shouldersJoint:add_child(shoulders)

-- ----------------------------------------------------------------------------------------
-- Hip
hipJoint = gr.joint('hipJoint', {-20, 0, 20}, {-20, 0, 20})
hipJoint:translate(0.0, -0.4, 0.0)
root:add_child(hipJoint)

hip = gr.mesh('sphere', 'hip')
hip:scale(0.25, 0.1, 0.15)
hip:set_material(yellow)

hipJoint:add_child(hip)

-- ----------------------------------------------------------------------------------------
-- Right upper arm
rightUpperArmJoint = gr.joint('rightUpperArmJoint', {-20, 0, 20}, {-20, 0, 20})
rightUpperArmJoint:translate(0.35, 0.0, 0.0)
shouldersJoint:add_child(rightUpperArmJoint)

rightUpperArm = gr.mesh('sphere', 'rightUpperArm')
rightUpperArmJoint:translate(0.0, -0.25, 0.0)
rightUpperArm:scale(0.05, 0.25, 0.05)
rightUpperArm:set_material(red)
rightUpperArmJoint:add_child(rightUpperArm)

-- ------------------------------------------------------------------------------------
-- Right lower arm
rightLowerArmJoint = gr.joint('rightLowerArmJoint', {-20, 0, 20}, {-20, 0, 20})
rightLowerArmJoint:translate(0.0, -0.2, 0.0)
rightUpperArmJoint:add_child(rightLowerArmJoint)

rightLowerArm = gr.mesh('sphere', 'rightLowerArm')
rightLowerArm:scale(0.04, 0.20, 0.04)
rightLowerArm:translate(0.0, -0.2, 0.0)
rightLowerArm:set_material(green)

rightLowerArmJoint:add_child(rightLowerArm)

-- ----------------------------------------------------------------------------------------
-- Right hand
rightHandJoint = gr.joint('rightHandJoint', {-20, 0, 20}, {-20, 0, 20})
rightLowerArmJoint:add_child(rightHandJoint)

rightHand = gr.mesh('sphere', 'rightHand')
rightHand:scale(0.05, 0.05, 0.05)
rightHand:translate(0.0, -0.4, 0.0)
rightHand:set_material(black)

rightLowerArmJoint:add_child(rightHand)

-- ----------------------------------------------------------------------------------------
-- Right upper Leg
rightUpperLegJoint = gr.joint('rightUpperLegJoint', {-20, 0, 20}, {-20, 0, 20})
rightUpperLegJoint:translate(0.2, 0.0, 0.0)
hipJoint:add_child(rightUpperLegJoint)

rightUpperLeg = gr.mesh('sphere', 'rightUpperLeg')
rightUpperLegJoint:translate(0.0, -0.25, 0.0)
rightUpperLeg:scale(0.06, 0.3, 0.06)
rightUpperLeg:set_material(blue)
rightUpperLegJoint:add_child(rightUpperLeg)

-- ----------------------------------------------------------------------------------------
-- Right Lower Leg
rightLowerLegJoint = gr.joint('rightLowerLegJoint', {-20, 0, 20}, {-20, 0, 20})
rightLowerLegJoint:translate(0.0, -0.2, 0.0)
rightUpperLegJoint:add_child(rightLowerLegJoint)

rightLowerLeg = gr.mesh('sphere', 'rightLowerLeg')
rightLowerLegJoint:translate(0.0, -0.25, 0.0)
rightLowerLeg:scale(0.05, 0.3, 0.05)
rightLowerLeg:set_material(red)
rightLowerLegJoint:add_child(rightLowerLeg)

-- ----------------------------------------------------------------------------------------
-- Right foot
rightFootJoint = gr.joint('rightFootJoint', {-20, 0, 20}, {-20, 0, 20})
rightFootJoint:translate(0.0, -0.25, 0.0)
rightLowerLegJoint:add_child(rightFootJoint)

rightFoot = gr.mesh('sphere', 'rightFoot')
-- rightFoot:scale(0.1, 0.05, 0.05)
-- rightFoot:translate(0.05, 0, 0)
rightFoot:scale(0.05, 0.05, 0.1)
rightFoot:translate(0, 0, 0.05)
rightFoot:set_material(black)
rightFootJoint:add_child(rightFoot)

-- ----------------------------------------------------------------------------------------
-- left upper arm
leftUpperArmJoint = gr.joint('leftUpperArmJoint', {-20, 0, 20}, {-20, 0, 20})
leftUpperArmJoint:translate(-0.35, 0.0, 0.0)
shouldersJoint:add_child(leftUpperArmJoint)

leftUpperArm = gr.mesh('sphere', 'leftUpperArm')
leftUpperArmJoint:translate(0.0, -0.25, 0.0)
leftUpperArm:scale(0.05, 0.25, 0.05)
leftUpperArm:set_material(red)
leftUpperArmJoint:add_child(leftUpperArm)

-- ------------------------------------------------------------------------------------
-- left lower arm
leftLowerArmJoint = gr.joint('leftLowerArmJoint', {-20, 0, 20}, {-20, 0, 20})
leftLowerArmJoint:translate(0.0, -0.2, 0.0)
leftUpperArmJoint:add_child(leftLowerArmJoint)

leftLowerArm = gr.mesh('sphere', 'leftLowerArm')
leftLowerArm:scale(0.04, 0.20, 0.04)
leftLowerArm:translate(0.0, -0.2, 0.0)
leftLowerArm:set_material(green)

leftLowerArmJoint:add_child(leftLowerArm)

-- ----------------------------------------------------------------------------------------
-- left hand
leftHandJoint = gr.joint('leftHandJoint', {-20, 0, 20}, {-20, 0, 20})
leftLowerArmJoint:add_child(leftHandJoint)

leftHand = gr.mesh('sphere', 'leftHand')
leftHand:scale(0.05, 0.05, 0.05)
leftHand:translate(0.0, -0.4, 0.0)
leftHand:set_material(black)

leftLowerArmJoint:add_child(leftHand)

-- ----------------------------------------------------------------------------------------
-- left upper Leg
leftUpperLegJoint = gr.joint('leftUpperLegJoint', {-20, 0, 20}, {-20, 0, 20})
leftUpperLegJoint:translate(-0.2, 0.0, 0.0)
hipJoint:add_child(leftUpperLegJoint)

leftUpperLeg = gr.mesh('sphere', 'leftUpperLeg')
leftUpperLegJoint:translate(0.0, -0.25, 0.0)
leftUpperLeg:scale(0.06, 0.3, 0.06)
leftUpperLeg:set_material(blue)
leftUpperLegJoint:add_child(leftUpperLeg)

-- ----------------------------------------------------------------------------------------
-- left Lower Leg
leftLowerLegJoint = gr.joint('leftLowerLegJoint', {-20, 0, 20}, {-20, 0, 20})
leftLowerLegJoint:translate(0.0, -0.2, 0.0)
leftUpperLegJoint:add_child(leftLowerLegJoint)

leftLowerLeg = gr.mesh('sphere', 'leftLowerLeg')
leftLowerLegJoint:translate(0.0, -0.25, 0.0)
leftLowerLeg:scale(0.05, 0.3, 0.05)
leftLowerLeg:set_material(red)
leftLowerLegJoint:add_child(leftLowerLeg)

-- ----------------------------------------------------------------------------------------
-- left foot
leftFootJoint = gr.joint('leftFootJoint', {-20, 0, 20}, {-20, 0, 20})
leftFootJoint:translate(0.0, -0.25, 0.0)
leftLowerLegJoint:add_child(leftFootJoint)

leftFoot = gr.mesh('sphere', 'leftFoot')
-- leftFoot:scale(0.1, 0.05, 0.05)
-- leftFoot:translate(0.05, 0, 0)
leftFoot:scale(0.05, 0.05, 0.1)
leftFoot:translate(0, 0, 0.05)
leftFoot:set_material(black)
leftFootJoint:add_child(leftFoot)

-- ------------------------------------------------------------------------------------
-- neck
neckJoint = gr.joint('neckJoint', {-20, 0, 20}, {-20, 0, 20})
neckJoint:translate(0.0, 0.4, 0.0)
root:add_child(neckJoint)

neck = gr.mesh('sphere', 'neck')
neckJoint:translate(0.0, 0.2, 0.0)
neck:scale(0.07, 0.15, 0.07)
neck:set_material(blue)
neckJoint:add_child(neck)

-- ------------------------------------------------------------------------------------
-- head
headJoint = gr.joint('headJoint', {-20, 0, 20}, {-20, 0, 20})
headJoint:translate(0.0, 0.1, 0.0)
neckJoint:add_child(headJoint)

head = gr.mesh('sphere', 'head')
head:scale(0.2, 0.2, 0.2)
head:set_material(white)
headJoint:add_child(head)

-- ------------------------------------------------------------------------------------
-- Glasses
rightLensJoint = gr.joint('rightLensJoint', {0, 0, 0}, {0, 0, 0})
rightLensJoint:translate(0.08, 0.0, 0.2)
headJoint:add_child(rightLensJoint)

rightLens = gr.mesh('sphere', 'rightLens')
rightLens:scale(0.06, 0.06, 0.02)
rightLens:set_material(darkRed)
rightLensJoint:add_child(rightLens)

leftLensJoint = gr.joint('leftLensJoint', {0, 0, 0}, {0, 0, 0})
leftLensJoint:translate(-0.08, 0.0, 0.2)
headJoint:add_child(leftLensJoint)

leftLens = gr.mesh('sphere', 'leftLens')
leftLens:scale(0.06, 0.06, 0.02)
leftLens:set_material(darkRed)
leftLensJoint:add_child(leftLens)

-- ------------------------------------------------------------------------------------

-- Return the root with all of it's childern.  The SceneNode A3::m_root will be set
-- equal to the return value from this Lua script.
return root
