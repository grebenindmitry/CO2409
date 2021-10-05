//
// ColourConversions.cpp
//
// Source file for colour conversion functions. The key functions are initially
// empty - you need to implement them for the lab exercise

// Find the minimum of three numbers (helper function for exercise below)

#include <cmath>

float Min( float f1, float f2, float f3 )
{
	float fMin = f1;
	if (f2 < fMin)
	{
		fMin = f2;
	}
	if (f3 < fMin)
	{
		fMin = f3;
	}
	return fMin;
}

// Find the maximum of three numbers (helper function for exercise below)
float Max( float f1, float f2, float f3 )
{
	float fMax = f1;
	if (f2 > fMax)
	{
		fMax = f2;
	}
	if (f3 > fMax)
	{
		fMax = f3;
	}
	return fMax;
}

// Convert an RGB colour to a HSL colour
void RGBToHSL( int R, int G, int B, int& H, int& S, int& L )
{
	float fR = R / 255.0;
	float fG = G / 255.0;
	float fB = B / 255.0;
	float MIN = Min(fR, fG, fB);
	float MAX = Max(fR, fG, fB);

	L = 50 * (MAX + MIN);

	if (MIN == MAX) {
		H = 0;
		S = 0;
		return;
	}

	if (L < 50) S = 100 * (MAX - MIN) / (MAX + MIN);
	else S = 100 * (MAX - MIN) / (2.0 - MAX - MIN);

	if (MAX == fR) H = 60 * (fG - fB) / (MAX - MIN);
	if (MAX == fG) H = 60 * (fB - fR) / (MAX - MIN) + 120;
	if (MAX == fB) H = 60 * (fR - fG) / (MAX - MIN) + 240;

	if (H < 0) H = H + 360;
}

// Convert a HSL colour to an RGB colour
void HSLToRGB( int H, int S, int L, int& R, int& G, int& B )
{
	float hsl[3] = {H / 60.0, S / 100.0, L / 100.0};
	float rgb[3] = { R, G, B };
	if (hsl[1] == 0)
		rgb[0] = rgb[1] = rgb[2] = hsl[2];
	else if (hsl[2] == 0)
		rgb[0] = rgb[1] = rgb[2] = 0.0f;
	else
	{
		const float q = hsl[2] < 0.5f ? hsl[2] * (1.0f + hsl[1]) : hsl[2] + hsl[1] - hsl[2] * hsl[1];
		const float p = 2.0f * hsl[2] - q;
		float t[] = { hsl[0] + 2.0f, hsl[0], hsl[0] - 2.0f };

		for (int i = 0; i < 3; ++i)
		{
			if (t[i] < 0.0f)
				t[i] += 6.0f;
			else if (t[i] > 6.0f)
				t[i] -= 6.0f;

			if (t[i] < 1.0f)
				rgb[i] = p + (q - p) * t[i];
			else if (t[i] < 3.0f)
				rgb[i] = q;
			else if (t[i] < 4.0f)
				rgb[i] = p + (q - p) * (4.0f - t[i]);
			else
				rgb[i] = p;
		}
	}

	R = round(rgb[0] * 255);
	G = round(rgb[1] * 255);
	B = round(rgb[2] * 255);
}
