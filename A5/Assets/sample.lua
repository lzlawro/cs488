rootNode = gr.node('root')

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 2)
darkRed = gr.material({0.1, 0.0, 0.0}, {0.1, 0.1, 0.1}, 2)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 2)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 2)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 2)
yellow = gr.material({1.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 2)
black = gr.material({0.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 2)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 2)
orange = gr.material({1.0, 0.7, 0.3}, {0.1, 0.1, 0.1}, 2)

rootNode:translate(0.0, 0.0, -7.5)

cubeMesh = gr.mesh('cube', 'pool')
cubeMesh:scale(4.0, 2.5, 4.0)
cubeMesh:translate(0.0, -1.5, 0.0)
-- cubeMesh:rotate('y', 45.0)
-- cubeMesh:rotate('x', 45.0)
-- cubeMesh:set_material(white)

rootNode:add_child(cubeMesh)

planeMesh = gr.mesh('plane', 'water')
-- planeMesh:scale(2.0, 2.0, 2.0)
planeMesh:scale(4.0, 4.0, 4.0)
planeMesh:translate(0.0, -0.25, 0.0)
-- planeMesh:rotate('y', 45.0)
-- planeMesh:rotate('x', 45.0)
-- planeMesh:set_material(blue)

rootNode:add_child(planeMesh)

sphereMesh = gr.mesh('sphere', 'sphere')
sphereMesh:scale(0.75, 0.75, 0.75)
sphereMesh:translate(0.2, 1.5, 0.2)
sphereMesh:set_material(orange)
rootNode:add_child(sphereMesh)

return rootNode
