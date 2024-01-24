#include <SFML/Graphics.hpp>
#include "System.h"
int main()
{
	System system;
	system.GenerateGridObjects({ 50, 50 }, { 800, 800 }, 100, 1, 1, 1);
	//system.SetPistolDImpulse(0.5);
	system.SetPistolState(false);

	system.Run();
}