// Spring 2020

#include "InstancedExample.hpp"

int main( int argc, char **argv ) 
{
	CS488Window::launch( argc, argv, new InstancedExample(), 1024, 768,
			"Instanced Rendering Example" );
	return 0;
}
