scene_root = gr.node('root')

gr.render(scene_root, 'simple.png', 256, 256,
	  {0, 0, 800}, {0, 0, -800}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {white_light, magenta_light})

white_light = gr.light({-100.0, 150.0, 400.0}, {0.9, 0.9, 0.9}, {1, 0, 0})
magenta_light = gr.light({400.0, 100.0, 150.0}, {0.7, 0.0, 0.7}, {1, 0, 0})

gr.render(scene_root, 'simple.png', 256, 256,
    {0, 0, 800}, {0, 0, -800}, {0, 1, 0}, 50,
    {0.3, 0.3, 0.3}, {white_light, magenta_light})
