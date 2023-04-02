rootNode = gr.node('root')

-- red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
-- darkRed = gr.material({0.1, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
-- blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
-- green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
-- white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
-- yellow = gr.material({1.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
-- black = gr.material({0.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
-- white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

rootNode:translate(0.0, 0.0, -5.5)

planeMesh = gr.mesh('plane', 'water')
-- planeMesh:scale(2.0, 2.0, 2.0)
planeMesh:scale(3.0, 3.0, 3.0)
planeMesh:translate(0.0, -0.24, 0.0)
planeMesh:rotate('y', 45.0)
-- planeMesh:rotate('x', 45.0)
-- planeMesh:set_material(blue)

rootNode:add_child(planeMesh)

cubeMesh = gr.mesh('cube', 'pool')
cubeMesh:scale(3.0, 1.5, 3.0)
cubeMesh:translate(0.0, -1.0, 0.0)
cubeMesh:rotate('y', 45.0)
-- cubeMesh:rotate('x', 45.0)
-- cubeMesh:set_material(white)

rootNode:add_child(cubeMesh)

sphereMesh = gr.mesh('sphere', 'ball')
sphereMesh:scale(0.3, 0.3, 0.3)
sphereMesh:translate(0.1, 0.5, 0.1)
-- sphereMesh:set_material(white)
rootNode:add_child(sphereMesh)

return rootNode
