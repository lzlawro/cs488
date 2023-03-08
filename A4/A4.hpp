// Termm--Fall 2020

#pragma once

#include <glm/glm.hpp>

#include "Ray.hpp"
#include "SceneNode.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include "Light.hpp"
#include "Image.hpp"

void A4_Render(
		// What to render
		SceneNode * root,

		// Image to write to, set to a given width and height
		Image & image,

		// Viewing parameters
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters
		const glm::vec3 & ambient,
		const std::list<Light *> & lights
);

// Ray color
glm::vec3 rayColor(
	const Ray &ray, 
	SceneNode *root,
	const glm::vec3 & eye,
	const glm::vec3 & ambient,
	const std::list<Light *> & lights
	);

float hitSphere(const glm::vec3 &center, float radius, const Ray &ray);
