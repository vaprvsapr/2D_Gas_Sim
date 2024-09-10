#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include <future>
#include <sstream>
#include <mutex>
#include <set>

#include "Object.h"
#include "Pistol.h"
#include "profile.h"

using namespace std;

class System
{
private:
	vector<Object> objects;

	vector<vector<vector<Object*>>> subregions;
	int subregions_degree = 10;

	const Vec2D system_size = { 1600, 900 };

	Pistol pistol = {1000, system_size.y, 0, 0, false};
	mutex pistol_position_mutex;

	float dt = 0.1f;
	size_t iteration = 0;
	int collisions_cnt = 0;


	chrono::steady_clock::time_point system_start;
	int collide_with_walls_time = 0;
	int collide_objects_time = 0;
	int visualize_time = 0;
	int update_subregions_time = 0;
	int move_time = 0;

	int number_of_visualized_objects = 1000;
	size_t stop_iteration = -1;

	float upper_wall_dimpulse = 0;
	float left_wall_dimpulse = 0;
	int pistol_iteration = 1;
	mutex wall_dimpuls_mutex;

	set<pair<float, float>> types_of_objects;




	void MoveObjects()
	{
		for (int i = 0; i < objects.size(); i++)
			objects[i].position += objects[i].velocity * dt;
	}

	void UpdateSubregions()
	{
		if (iteration == 1)
		{
			subregions.resize(subregions_degree);
			for (int i = 0; i < subregions_degree; i++)
				subregions[i].resize(subregions_degree);
		}


		// clear objects_references
		for (int i = 0; i < subregions_degree; i++)
			for (int j = 0; j < subregions_degree; j++)
				subregions[i][j].clear();


		// add references to subregions
		for (int i = 0; i < objects.size(); i++)
		{
			subregions[int(subregions_degree * objects[i].position.x / pistol.position)][
				int(subregions_degree * objects[i].position.y / system_size.y)].push_back(
						&objects[i]
					);
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
		lock_guard<mutex> lock(wall_dimpuls_mutex);

		for (int i = 0; i < objects.size(); i++)
		{
			if (objects[i].position.y < objects[i].radius)
			{
				objects[i].velocity.y *= -1;
				objects[i].position.y = objects[i].radius + 1;

				upper_wall_dimpulse += 2 * objects[i].mass * objects[i].velocity.y;
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

				left_wall_dimpulse += 2 * objects[i].mass * objects[i].velocity.x;
			}
			if (objects[i].position.x > pistol.position - objects[i].radius)
			{
				if (pistol.active)
				{

					float pistol_velocity = pistol.velocity;
					pistol.velocity = ((pistol.mass - objects[i].mass) * pistol.velocity + 2 * objects[i].mass * objects[i].velocity.x) / (objects[i].mass + pistol.mass);
					objects[i].velocity.x = (objects[i].velocity.x * (objects[i].mass - pistol.mass) + 2 * pistol.mass * pistol_velocity) / (objects[i].mass + pistol.mass);
				}
				else
					objects[i].velocity *= -1;

				objects[i].position.x = pistol.position - objects[i].radius - 1;
			}
		}
	}

	void CollideTwoIdenticalObjects(Object& lhs, Object& rhs)
	{	
		Vec2D centers = rhs.position - lhs.position;
		
		if (centers.Abs() == 0)
			centers = { 1, 1 };
		centers /= centers.Abs();
		Vec2D lhs_vel_parallel = lhs.velocity.Projection(centers);
		Vec2D rhs_vel_parallel = rhs.velocity.Projection(centers);
		Vec2D lhs_vel_perp = lhs.velocity - lhs_vel_parallel;
		Vec2D rhs_vel_perp = rhs.velocity - rhs_vel_parallel;

		lhs.velocity = lhs_vel_perp + (lhs_vel_parallel*(lhs.mass - rhs.mass) + rhs_vel_parallel*2*rhs.mass)/(lhs.mass + rhs.mass);
		rhs.velocity = rhs_vel_perp + (rhs_vel_parallel*(rhs.mass - lhs.mass) + lhs_vel_parallel*2*lhs.mass)/(lhs.mass + rhs.mass);

		float dradius = lhs.radius + rhs.radius;
		centers *= dradius * 0.01f;
		while ((rhs.position - lhs.position).Abs() < dradius)
		{
			rhs.position += centers;
			lhs.position -= centers;
		}
	}

	void CollideObjects()
	{
		for (int i = 0; i < objects.size() - 1; i++)
			for (int j = i + 1; j < objects.size(); j++)
				if((objects[i].position - objects[j].position).Abs() < objects[i].radius + objects[j].radius)
				{
					CollideTwoIdenticalObjects(objects[i], objects[j]);
					collisions_cnt++;
				}
	}

	void CollideObjectsInSubregions()
	{
		for (int i = 0; i < subregions_degree; i++)
			for (int j = 0; j < subregions_degree; j++)
			{
				if (subregions[i][j].size() > 1)
				{
					for (int k = 0; k < subregions[i][j].size() - 1; k++)
						for (int l = k + 1; l < subregions[i][j].size(); l++)
							if ((subregions[i][j][k]->position - subregions[i][j][l]->position).Abs() < subregions[i][j][k]->radius + subregions[i][j][l]->radius)
							{
								CollideTwoIdenticalObjects(*subregions[i][j][k], *subregions[i][j][l]);
								collisions_cnt++;
							}
				}
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

	void SetPistolMass(float m)
	{
		pistol.mass = m;
	}

	System(int _subregions_degree = 10)
	{
		subregions_degree = _subregions_degree;
	}

	void SetNumberOfVisualizedObjects(int n)
	{
		number_of_visualized_objects = n;
	}

	void SetNumberOfIterations(size_t n)
	{
		stop_iteration = n;
	}

	void GenerateGridObjects(Vec2D initial_point, Vec2D grid_size, int n, float mass, float radius, float initial_velocity)
	{
		types_of_objects.insert({ mass, radius });

		srand(0);
		for(int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
			{
				Vec2D position = {
					initial_point.x + grid_size.x / (n - 1) * i,
					initial_point.y + grid_size.y / (n - 1) * j 
				};

				float phi = float(rand() % 10000) / 10000 * 2 * 3.141592f;
				Vec2D velocity = { 
					initial_velocity * sin(phi),
					initial_velocity * cos(phi)
				};

				AddObject({ mass, radius, position, velocity });
			}
	}

	void ExecuteCommand()
	{
		while (true)
		{
			string command;
			cin >> command;


			if (command == "get_energy")
			{
				cout << Energy() << endl;
				continue;
			}
			if (command == "set_pistol_pos")
			{
				cout << "new pistol.position: ";
				float pos;
				cin >> pos;
				lock_guard<mutex> lock(pistol_position_mutex);
				pistol.position = pos;
				cout << "pistol.position set to " << pistol.position << " successfully" << endl;
				pistol_iteration = 0;
				left_wall_dimpulse = 0;
				upper_wall_dimpulse = 0;
			}
			if (command == "get_pressure")
			{
				cout << "left wall pressure: " << left_wall_dimpulse / dt / system_size.y / pistol_iteration << endl;
				cout << "uppper wall pressure: " << upper_wall_dimpulse / dt / pistol.position / pistol_iteration << endl;
			}
			
		}
	}

	void Run()
	{
		system_start = chrono::steady_clock::now();

		sf::RenderWindow window(sf::VideoMode(system_size.x, system_size.y), "Sim!");
		sf::CircleShape shape(100.f);
		sf::RectangleShape pistol_shape({ 100, system_size.y });
		shape.setFillColor(sf::Color::White);
		pistol_shape.setFillColor(sf::Color::White);


		future<void> execute_command_thread = async([this] {ExecuteCommand(); });


		while (window.isOpen())
		{
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					window.close();
			}

			iteration++;
			pistol_iteration++;
			/*cout << "iteration: " << iteration << '\n';*/

			if (iteration == stop_iteration)
			{
				cout << "run time: " << chrono::duration_cast<milliseconds>(chrono::steady_clock::now() - system_start).count() << " ms" << '\n';
				cout << "collide objects time: " << collide_objects_time << " ms" << '\n';
				cout << "visualize time: " << visualize_time << " ms" << '\n';
				cout << "collide with walls time: " << collide_with_walls_time << " ms" << '\n';
				cout << "move time: " << move_time << " ms" << '\n';
				cout << "update subregions_time time: " << update_subregions_time << " ms" << '\n';
				cout << "collisons_cnt: " << collisions_cnt << '\n';
				cout << "===========================" << '\n';
				window.close();
				break;
			}




			// visualization
			if(iteration % 10 == 0)
			{
				chrono::steady_clock::time_point start = chrono::steady_clock::now();
				window.clear();

				// drawing objects
				for (int i = 0; i < min(int(objects.size()),  number_of_visualized_objects); i++)
				{
					shape.setRadius(max(1.0f, objects[i].radius));
					shape.setPosition({ objects[i].position.x - objects[i].radius, objects[i].position.y - objects[i].radius });
					window.draw(shape);
				}

				// drawing pistol
				{
					pistol_shape.setPosition({ pistol.position, 0 });
					window.draw(pistol_shape);
				}

				visualize_time += chrono::duration_cast<milliseconds>(chrono::steady_clock::now() - start).count();

				shape.setFillColor(sf::Color::White);
				// drawing dist hist
				{
					sf::Vector2f hist_size = { 400, 200 };
					sf::Vector2f hist_offset = { 1100, 100 };

					int n = 100;

					sf::RectangleShape rectangle({ hist_size.x / n, hist_size.y });

					float max_vel = 0;
					for (int i = 0; i < objects.size(); i++)
						if (objects[i].velocity.Abs() > max_vel)
							max_vel = objects[i].velocity.Abs();

					max_vel += 0.001f;
					max_vel = 5;
					vector<int> columns;
					columns.resize(n);
					map<pair<float, float>, vector<int>> object_type_cnt;

					{
						vector<int> vec(n);
						for (const auto& type_of_object : types_of_objects)
						{
							object_type_cnt.insert({ type_of_object, vec });
						}
					}


					for (int i = 0; i < objects.size(); i++)
					{
						columns[int(objects[i].velocity.Abs() / max_vel * n) > (n - 1) ? n-1 : int(objects[i].velocity.Abs() / max_vel * n)]++;
						object_type_cnt.at({objects[i].mass, objects[i].radius})
							[int(objects[i].velocity.Abs() / max_vel * n) > (n - 1) ? n - 1 : int(objects[i].velocity.Abs() / max_vel * n)]++;
					}

					int columns_max = 0;
					for (int i = 0; i < n; i++)
						if (columns[i] > columns_max)
							columns_max = columns[i];

					float background_width = 20;
					rectangle.setSize({ hist_size.x + background_width, hist_size.y + background_width });
					rectangle.setPosition({ hist_offset.x - background_width / 2, hist_offset.y - background_width / 2 });
					window.draw(rectangle);
					rectangle.setFillColor(sf::Color::Black);
					rectangle.setSize({ hist_size.x + background_width / 2, hist_size.y + background_width / 2 });
					rectangle.setPosition({ hist_offset.x - background_width / 4, hist_offset.y - background_width / 4 });
					window.draw(rectangle);
					rectangle.setFillColor(sf::Color::White);

					shape.setRadius(1);
					shape.setFillColor(sf::Color::Red);

					for (int i = 0; i < n; i++)
					{
						rectangle.setSize({ hist_size.x / n, hist_size.y / columns_max * columns[i] });
						rectangle.setPosition({i * hist_size.x / n + hist_offset.x, hist_offset.y + hist_size.y - hist_size.y / columns_max * columns[i] });
						window.draw(rectangle);

						for (const auto& cnt : object_type_cnt)
						{
							shape.setPosition({ i * hist_size.x / n + hist_offset.x, hist_offset.y + hist_size.y - hist_size.y / columns_max * cnt.second[i]});
							window.draw(shape);
						}
					}



				}

				window.display();
			}
			

			// performing calculations
			{
				lock_guard<mutex> lock(pistol_position_mutex);
				{
					chrono::steady_clock::time_point start = chrono::steady_clock::now();
					MoveObjects();
					MovePistol();
					move_time += chrono::duration_cast<milliseconds>(chrono::steady_clock::now() - start).count();
				}
				{
					chrono::steady_clock::time_point start = chrono::steady_clock::now();
					CollideWithWalls();
					collide_with_walls_time += chrono::duration_cast<milliseconds>(chrono::steady_clock::now() - start).count();
				}
				{
					chrono::steady_clock::time_point start = chrono::steady_clock::now();
					UpdateSubregions();
					update_subregions_time += chrono::duration_cast<milliseconds>(chrono::steady_clock::now() - start).count();
				}

				{
					chrono::steady_clock::time_point start = chrono::steady_clock::now();
					//CollideObjects();
					CollideObjectsInSubregions();
					collide_objects_time += chrono::duration_cast<milliseconds>(chrono::steady_clock::now() - start).count();
				}
			}
		}
		execute_command_thread.get();
	}
};
