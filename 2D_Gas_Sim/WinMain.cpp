#include <SFML/Graphics.hpp>
#include "System.h"
int main()
{
	System system(1);
	system.GenerateGridObjects({ 50, 50 }, { 800, 800 }, 2, 1, 100, 0.1);
	system.SetNumberOfVisualizedObjects(1000);
	system.SetPistolState(false);
	system.Run();
}
