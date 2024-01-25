#include <SFML/Graphics.hpp>
#include "System.h"
int main()
{
	System system(300);
	system.GenerateGridObjects({ 50, 50 }, { 800, 800 }, 100, 1, 0.1, 1);
	system.SetNumberOfVisualizedObjects(1000);
	system.SetPistolState(false);
	system.Run();
}