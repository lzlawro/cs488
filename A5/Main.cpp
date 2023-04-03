// Term-Winter 2023

#include "A5.hpp"
#include <X11/Xlib.h>

int main(int argc, char **argv)
{
    // CS488Window::launch(argc, argv, new A5(), 1024, 768, "W23 Assignment 5");

    XInitThreads();

    if (argc > 1) {
        std::string luaSceneFile(argv[1]);
		std::string title("W21 Assignment 5 - [");
		title += luaSceneFile;
		title += "]";

		CS488Window::launch(argc, argv, new A5(luaSceneFile), 1024, 768, title);
    } else {
        std::cout << "Must supply Lua file as First argument to program.\n";
        std::cout << "For example:\n";
        std::cout << "./A5 Assets/simpleScene.lua\n";
    }

    return 0;
}