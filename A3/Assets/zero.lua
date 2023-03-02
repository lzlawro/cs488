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

-- ------------------------------------------------------------------------------------

-- Return the root with all of it's childern.  The SceneNode A3::m_root will be set
-- equal to the return value from this Lua script.
return root
