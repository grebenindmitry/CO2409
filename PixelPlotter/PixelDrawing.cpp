/*******************************************
	PixelDrawing.cpp

	2D pixel based drawing functions
********************************************/

#include "stdafx.h"            // Precompiled headers for Visual Studio
#include "PixelPlotterForm.h"  // Class header file
#include <math.h>              // Need standard math functions for drawing circles
#include <algorithm>
#include <iostream>

// All application definitions are contained in a 'namespace'. Namespaces primarily protect against
// variable name clashes when using many libraries together, they also promote good encapsulation
// Here the namespace is needed for the main drawing application - you can safely ignore it
namespace PixelPlotter {

	////////////////////////////////////
	// Instructions
	////////////////////////////////////
	//
	// The functions below are initially empty, you need to implement them for the lab exercise.
	// Note that they are all member functions of the class PixelPlotterForm, so they begin with 
	// "PixelPlotterForm::". This does not affect how you write the functions
	//
	// The following function has already been declared:
	//     SetViewportPixel( int X, int Y, Color PixelColour );
	// To write the functions you only need to call this function once for each pixel in your shape
	//
	// Where necessary, use the definition of PI below
	// Also note that <math.h> has already been included

	const float PI = 3.14159f;


	// Draw a rectangle with top-left at (X, Y) with given width, height and colour
	void PixelPlotterForm::DrawRectangle(int X, int Y, int Width, int Height, Color PixelColour) {
		for (int y = Y; y <= Y + Height; y++) {
			SetViewportPixel(X, y, PixelColour);
			SetViewportPixel(X + Width, y, PixelColour);
		}
		for (int x = X; x <= X + Width; x++) {
			SetViewportPixel(x, Y, PixelColour);
			SetViewportPixel(x, Y + Height, PixelColour);
		}
	}


	// Draw a line from (X1, Y1) to (X2, Y2) in given colour
	void PixelPlotterForm::DrawLine(int X1, int Y1, int X2, int Y2, Color PixelColour) {
		int MaxDist = std::max(abs(X2 - X1), abs(Y2 - Y1));
		float XStep = (X2 - X1) / (float)MaxDist;
		float YStep = (Y2 - Y1) / (float)MaxDist;

		float X = X1 + 0.5;
		float Y = Y1 + 0.5;

		while (MaxDist >= 0) {
			SetViewportPixel(round(X), round(Y), PixelColour);
			X += XStep;
			Y += YStep;
			MaxDist--;
		}
	}


	// Draw a regular polygon with the given number of sides. The centre of the polygon will be at
	// (X, Y) and the points are on a circle of radius R
	void PixelPlotterForm::DrawPolygon(int Sides, int X, int Y, int R, Color PixelColour) {
		for (int side = 1; side <= Sides; side++) {
			float X1 = R * sin(2 * PI / Sides * side + PI) + 0.5;
			float Y1 = R * cos(2 * PI / Sides * side + PI) + 0.5;
			float X2 = R * sin(2 * PI / Sides * (side + 1) + PI) + 0.5;
			float Y2 = R * cos(2 * PI / Sides * (side + 1) + PI) + 0.5;

			DrawLine(X1 + X, Y1 + Y, X2 + X, Y2 + Y, PixelColour);
		}
	}


	// Draw a regular star with the given number of points. The centre of the star will be at (X, Y)
	// and it the points are on a circle of radius R. The number of points will always be odd.
	void PixelPlotterForm::DrawStar(int Sides, int X, int Y, int R, Color PixelColour) {
		for (int side = 1; side <= Sides; side++) {
			float X1 = R * sin(2 * PI / Sides * side + PI) + 0.5;
			float Y1 = R * cos(2 * PI / Sides * side + PI) + 0.5;
			float X2 = R * sin(2 * PI / Sides * (side + 2) + PI) + 0.5;
			float Y2 = R * cos(2 * PI / Sides * (side + 2) + PI) + 0.5;

			DrawLine(X1 + X, Y1 + Y, X2 + X, Y2 + Y, PixelColour);
		}
	}


	// Draw a circle with centre (X, Y) and radiuses RX (horizontal) and RY (vertical)
	void PixelPlotterForm::DrawCircle(int X, int Y, int R, Color PixelColour) {
		float x = 0, y = R;
		float P = 1 - R;

		SetViewportPixel(x + X, y + Y, PixelColour);

		while (x <= y) {
			if (P < 0) {
				x++;
				P = P + 2 * x + 1;
			}
			else {
				x++;
				y--;
				P = P - 2 * y + 2 * x + 1;
			}

			SetViewportPixel(x + X, y + Y, PixelColour);
			SetViewportPixel(-x + X, y + Y, PixelColour);
			SetViewportPixel(x + X, -y + Y, PixelColour);
			SetViewportPixel(-x + X, -y + Y, PixelColour);
			SetViewportPixel(y + X, x + Y, PixelColour);
			SetViewportPixel(-y + X, x + Y, PixelColour);
			SetViewportPixel(y + X, -x + Y, PixelColour);
			SetViewportPixel(-y + X, -x + Y, PixelColour);
		}

		SetViewportPixel(X + R, Y, PixelColour);
		SetViewportPixel(X - R, Y, PixelColour);
		SetViewportPixel(X, Y + R, PixelColour);
		SetViewportPixel(X, Y - R, PixelColour);
	}


} // end namespace PixelPlotter