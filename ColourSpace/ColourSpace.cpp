// ColourSpace.cpp : main project file.

#include "stdafx.h"
#include "ColourSpaceForm.h"

using namespace ColourSpace;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// Create the main window and run it
	Application::Run(gcnew ColourSpaceForm());
	return 0;
}
