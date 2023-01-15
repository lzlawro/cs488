#version 330

uniform uint objid;

out uint fragColour;

void main() {
	fragColour = objid;
}
