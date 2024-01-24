#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <random>
#include <chrono>

#include "Object.h"
#include "Pistol.h"
#include "profile.h"

using namespace std;

class System
{
private:
	vector<Object> objects;
	const Vec2D system_size = { 1600, 900 };

	Pistol pistol = {1000, system_size.y, 0, 0, false};

	float dt = 0.1;
	int iteration = 0;
	steady_clock::time_point start;

	void MoveObjects()
	{
		for (int i = 0; i < objects.size(); i++)
		{
			objects[i].position += objects[i].velocity * dt;
		}
	}

	void MovePistol() {
		if (pistol.active)
		{
			pistol.velocity += pistol.dimpulse / pistol.mass;
			pistol.position += pistol.velocity * dt;
		}
	}

	void CollideWithWalls()
	{
		for (int i = 0; i < objects.size(); i++)
		{
			if (objects[i].position.y < objects[i].radius)
			{
				objects[i].velocity.y *= -1;
				objects[i].position.y = objects[i].radius + 1;
			}

			if (objects[i].position.y > system_size.y - objects[i].radius)
			{
				objects[i].velocity.y *= -1;
				objects[i].position.y = system_size.y - objects[i].radius - 1;
			}
			if (objects[i].position.x < objects[i].radius)
			{
				objects[i].velocity.x *= -1;
				objects[i].position.x = objects[i].radius + 1;
			}
			if (objects[i].position.x > pistol.position - objects[i].radius)
			{
				pistol.velocity += 2 * objects[i].velocity.x * objects[i].mass / pistol.mass;
				objects[i].velocity.x *= -1;
				objects[i].position.x = pistol.position - objects[i].radius - 1;
			}
		}
	}

	void CollideTwoIdenticalObjects(Object& lhs, Object& rhs)
	{
		Vec2D centers = rhs.position - lhs.position;
		if (centers.Abs() < lhs.radius + rhs.radius)
		{
			centers /= centers.Abs();
			Vec2D lhs_vel_parallel = lhs.velocity.Projection(centers);
			Vec2D rhs_vel_parallel = rhs.velocity.Projection(centers);
			Vec2D lhs_vel_perp = lhs.velocity - lhs_vel_parallel;
			Vec2D rhs_vel_perp = rhs.velocity - rhs_vel_parallel;


			lhs.velocity = lhs_vel_perp + rhs_vel_parallel;
			rhs.velocity = rhs_vel_perp + lhs_vel_parallel;
		}
	}

	void CollideObjects()
	{
		for (int i = 0; i < objects.size() - 1; i++)
			for (int j = i + 1; j < objects.size(); j++)
			{
				CollideTwoIdenticalObjects(objects[i], objects[j]);
			}
	}

	float Energy()
	{
		float E = 0;
		for (int i = 0; i < objects.size(); i++)
		{
			E += objects[i].mass * objects[i].velocity.Squared() / 2;
		}
		return E;
	}

public:
	void AddObject(const Object& object)
	{
		objects.push_back(object);
	}

	void SetPistolDImpulse(const float& dimpulse)
	{
		pistol.dimpulse = -dimpulse;
	}

	void SetPistolState(bool active)
	{
		pistol.active = active;
	}

	void GenerateGridObjects(Vec2D initial_point, Vec2D grid_size, int n, float mass, float radius, float initial_velocity)
	{
		srand(0);
		for(int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
			{
				Vec2D position = {
					initial_point.x + grid_size.x / (n - 1) * i,
					initial_point.y + grid_size.y / (n - 1) * j 
				};

				float phi = float(rand() % 10000) / 10000 * 2 * 3.141592;
				Vec2D velocity = { 
					initial_velocity * sin(phi),
					initial_velocity * cos(phi)
				};

				AddObject({ mass, radius, position, velocity });
			}
	}

	void Run()
	{
		start = chrono::steady_clock::now();

		sf::RenderWindow window(sf::VideoMode(system_size.x, system_size.y), "Sim!");
		sf::CircleShape shape(100.f);
		sf::RectangleShape pistol_shape({ 100, system_size.y });
		shape.setFillColor(sf::Color::White);
		pistol_shape.setFillColor(sf::Color::White);

		while (window.isOpen())
		{
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
			}

			iteration++;
			cout << "iteration: " << iteration << '\n';


			if (iteration == 1000)
			{
				cout << "run time: " << chrono::duration_cast<milliseconds>(chrono::steady_clock::now() - start).count()
					<< " ms" << '\n';
				window.close();
				break;
			}





			{
				LOG_DURATION("Visualization");
				window.clear();

				// drawing objects
				for (int i = 0; i < objects.size(); i++)
				{
					shape.setRadius(objects[i].radius);
					shape.setPosition({ objects[i].position.x - objects[i].radius, objects[i].position.y - objects[i].radius });
					window.draw(shape);
				}

				// drawing pistol
				{
					pistol_shape.setPosition({ pistol.position, 0 });
					window.draw(pistol_shape);
				}
			}
			

			// performing calculations
			{
				{
					LOG_DURATION("MoveObjects()");
					MoveObjects();
					MovePistol();
				}
				{
					LOG_DURATION("CollideWithWalls()");
					CollideWithWalls();
				}
				{
					LOG_DURATION("CollideObjects()");
					CollideObjects();
				}
				{
					
					LOG_DURATION("Energy()");
					cout << "E: " << Energy() << '\n';
				}

			}
			window.display();
		}
	}
};