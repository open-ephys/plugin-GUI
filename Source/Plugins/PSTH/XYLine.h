#ifndef XYLINE_H
#define XYLINE_H

#include "../../../JuceLibraryCode/JuceHeader.h"

#include <list>
#include <vector>
#include <queue>

#ifndef MAX
#define MAX(x,y)((x)>(y))?(x):(y)
#endif 

#ifndef MIN
#define MIN(x,y)((x)<(y))?(x):(y)
#endif 
class XYline
{
public:
	//XYline(std::vector<float> x_, std::vector<float> y_, float gain, juce::Colour color_);
	XYline(float x0, float dx, std::vector<float> y_, float gain, juce::Colour color_);

	// for pure vertical lines
	XYline(float x0_, float ymin, float ymax, juce::Colour color_);
	XYline getFFT();
	void draw(Graphics &g, float xmin, float xmax, float ymin, float ymax, int width, int height, bool showBounds);
	void getYRange(float xmin, float xmax, double &lowestValue, double &highestValue);
	void removeMean();
	void smooth(std::vector<float> kernel);
	int getNumPoints();
private:
	void four1(std::vector<float> &data, int nn, int isign);
	void four1(double data[], int nn, int isign);

	float interp(float x_sample, bool &inrange);
	float interp_bilinear(float x_sample, bool &inrange);

	float interp_cubic(float x_sample, bool &inrange);
	bool sortedX, fixedDx, verticalLine;
	float gain, dx, x0, xn, mean;
	int numpts;
	std::vector<float> x;
	std::vector<float> y;
	juce::Colour color;
};


#endif