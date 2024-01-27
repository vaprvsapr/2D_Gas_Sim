#include <SFML/Graphics.hpp>
#include "System.h"
int main()
{
	System system(300);
	system.GenerateGridObjects({ 50, 50 }, { 800, 350 }, 100, 1, 0.01, 1);
	system.GenerateGridObjects({ 50, 500 }, { 800, 350 }, 100, 2, 0.01, 1);
	system.SetNumberOfVisualizedObjects(1000);
	system.SetPistolState(false);
	system.Run();
}