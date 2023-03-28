rootNode = gr.node('root')

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
darkRed = gr.material({0.1, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
yellow = gr.material({1.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
black = gr.material({0.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

planeMesh = gr.mesh('plane', 'name-of-plane')
planeMesh:scale(2.0, 2.0, 2.0)
planeMesh:rotate('y', 45.0)
planeMesh:translate(0.0, -0.5, -2.5)
planeMesh:set_material(blue)

rootNode:add_child(planeMesh)

return rootNode
