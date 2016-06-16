#include "XYLine.h"
/*************************************************************************/
XYline::XYline(float x0_, float dx_, std::vector<float> y_, float gain_, juce::Colour color_) : x0(x0_), y(y_), dx(dx_), color(color_), gain(gain_)
{
	// adjust gain
	numpts = y.size();
	for (int k = 0; k<numpts; k++)
	{
		y[k] *= gain;
	}
	xn = x0 + dx * (numpts - 1);
	verticalLine = false;
	fixedDx = true;
}

XYline::XYline(float x0_, float ymin, float ymax, juce::Colour color_) : x0(x0_), color(color_)
{
	gain = 1.0;
	y.clear();
	y.push_back(ymin);
	y.push_back(ymax);
	verticalLine = true;
	numpts = y.size();
	xn = x0;
	fixedDx = false;
}

int XYline::getNumPoints()
{
	return numpts;
}

void XYline::smooth(std::vector<float> smoothKernel)
{
	std::vector<float> smoothy;
	smoothy.resize(y.size());

	int numKernelBins = smoothKernel.size();
	int zeroIndex = (numKernelBins - 1) / 2;
	int numXbins = y.size();
	for (int k = 0; k<numXbins; k++)
	{
		float response = 0;
		for (int j = -zeroIndex; j<zeroIndex; j++)
		{
			if (k + j >= 0 && k + j < numXbins)
				response += y[k + j] * smoothKernel[j + zeroIndex];
		}
		smoothy[k] = response;
	}
	y = smoothy;
}

void XYline::getYRange(float xmin, float xmax, double &lowestValue, double &highestValue)
{
	int startIndex = MIN(numpts, MAX(0, (xmin - x0) / dx));
	int endIndex = MIN(numpts, MAX(0, (xmax - x0) / dx));
	for (int k = startIndex; k<endIndex; k++)
	{
		lowestValue = MIN(lowestValue, y[k]);
		highestValue = MAX(highestValue, y[k]);
	}
}

void XYline::removeMean()
{
	// first, remove gain...
	// then, compute mean
	// then, return gain.
	mean = 0;
	for (int k = 0; k<y.size(); k++)
	{
		y[k] /= gain;
		mean += y[k];
	}
	mean /= y.size();
	for (int k = 0; k<y.size(); k++)
	{
		y[k] = (y[k] - mean)*gain;
	}
}


float XYline::interp(float x_sample, bool &inrange)
{
	return interp_bilinear(x_sample, inrange);
}

float XYline::interp_bilinear(float x_sample, bool &inrange)
{
	inrange = (x_sample >= x0 && x_sample <= xn);
	if (!inrange)
		return 0;

	float bin = (x_sample - x0) / dx;
	int lowerbin = floor(bin);
	float fraction = bin - lowerbin;
	int higherbin = lowerbin + 1;
	if (lowerbin == y.size() - 1)
		return y[lowerbin];
	else
		return y[lowerbin] * (1 - fraction) + y[higherbin] * (fraction);
}


float XYline::interp_cubic(float x_sample, bool &inrange)
{
	inrange = (x_sample >= x0 && x_sample <= xn);
	if (!inrange)
		return 0;

	float bin = (x_sample - x0) / dx;
	int lowerbin = floor(bin);
	float t = bin - lowerbin;
	int higherbin = lowerbin + 1;
	if (lowerbin == y.size() - 1)
		return y[lowerbin];
	else {
		if (lowerbin - 2 >= 0 & lowerbin + 2 < y.size())
		{
			int k = lowerbin;
			double tsqr = t*t;
			double tcub = t*tsqr;
			double m0 = (y[k + 1] - y[k - 1]) / 2;
			k = lowerbin + 1;
			double m1 = (y[k + 1] - y[k - 1]) / 2;
			return (2 * tcub - 3 * tsqr + 1)*y[lowerbin] + (-2 * tcub + 3 * tsqr)*y[higherbin] + (tcub - 2 * tsqr + t)*m0 + (tcub - tsqr)*m1;
		}
		else {
			inrange = false;
			return 0;
		}
	}
}

/*
FFT/IFFT routine. (see pages 507-508 of Numerical Recipes in C)

Inputs:
data : vector of complex* data points of size 2*NFFT+1.
* the n'th complex number x(n), for 0 <= n <= length(x)-1, is stored as:
data[2*n] = real(x(n))
data[2*n+1] = imag(x(n))
if length(Nx) < NFFT, the remainder of the array must be padded with zeros

nn : FFT order NFFT. This MUST be a power of 2 and >= length(x).
isign:  if set to 1,
computes the forward FFT
if set to -1,
computes Inverse FFT - in this case the output values have
to be manually normalized by multiplying with 1/NFFT.
Outputs:
data[] : The FFT or IFFT results are stored in data, overwriting the input.



void XYline::four1(std::vector<float> &data, int nn, int isign)
{
int n, mmax, m, j, istep, i;
double wtemp, wr, wpr, wpi, wi, theta;
double tempr, tempi;
const double twopi = 6.28318530718;
n = nn << 1;
j = 1;
for (i = 1; i < n; i += 2) {
if (j > i) {
tempr = data[j-1];     data[j-1] = data[i-1];     data[i-1] = tempr;
tempr = data[j+1-1]; data[j+1-1] = data[i+1-1]; data[i+1-1] = tempr;
}
m = n >> 1;
while (m >= 2 && j > m) {
j -= m;
m >>= 1;
}
j += m;
}
mmax = 2;
while (n > mmax) {
istep = 2*mmax;
theta = twopi/(isign*mmax);
wtemp = sin(0.5*theta);
wpr = -2.0*wtemp*wtemp;
wpi = sin(theta);
wr = 1.0;
wi = 0.0;
for (m = 1; m < mmax; m += 2) {
for (i = m; i <= n; i += istep) {
j =i + mmax;
tempr = wr*data[j-1]   - wi*data[j+1-1];
tempi = wr*data[j+1-1] + wi*data[j-1];
data[j-1]   = data[i-1]   - tempr;
data[j+1-1] = data[i+1-1] - tempi;
data[i-1] += tempr;
data[i+1-1] += tempi;
}
wr = (wtemp = wr)*wpr - wi*wpi + wr;
wi = wi*wpr + wtemp*wpi + wi;
}
mmax = istep;
}
}


XYline XYline::getFFT()
{
int Nx = numpts;
// calculate NFFT as the next higher power of 2 >= Nx
int NFFT = (int)pow(2.0, ceil(log((double)Nx)/log(2.0)));
std::vector<float> X;
X.resize(2*NFFT); // to hold complex

// Storing x(n) in a complex array to make it work with four1.
//This is needed even though x(n) is purely real in this case.
for(int i=0; i<Nx; i++)
{
X[2*i] = y[i];
X[2*i+1] = 0.0;
}
// pad the remainder of the array with zeros (0 + 0 j)
for(int i=Nx; i<NFFT; i++)
{
X[2*i] = 0.0;
X[2*i+1] = 0.0;
}

// calculate FFT
four1(X, NFFT, 1);


// now convert to power spectrum, and remove the second half of the signal (mirror symmetry of real function).
float Fs = 1.0/dx;
float df = (NFFT/2+1) / (Fs/2);
std::vector<float> powerspectrum;
powerspectrum.resize(NFFT/2+1);
for (int k=0;k<NFFT/2+1;k++)
{
powerspectrum[k] = log10(2 *  sqrt(X[2*k]*X[2*k]+ X[2*k+1]*X[2*k+1]));
}

return XYline(0,df, powerspectrum,1.0, color);
}

*/





/************************************************
* FFT code from the book Numerical Recipes in C *
* Visit www.nr.com for the licence.             *
************************************************/

// The following line must be defined before including math.h to correctly define M_PI

#define PI	3.14159265359	/* pi to machine precision, defined in math.h */
#define TWOPI	(2.0*PI)

/*
FFT/IFFT routine. (see pages 507-508 of Numerical Recipes in C)

Inputs:
data[] : array of complex* data points of size 2*NFFT+1.
data[0] is unused,
* the n'th complex number x(n), for 0 <= n <= length(x)-1, is stored as:
data[2*n+1] = real(x(n))
data[2*n+2] = imag(x(n))
if length(Nx) < NFFT, the remainder of the array must be padded with zeros

nn : FFT order NFFT. This MUST be a power of 2 and >= length(x).
isign:  if set to 1,
computes the forward FFT
if set to -1,
computes Inverse FFT - in this case the output values have
to be manually normalized by multiplying with 1/NFFT.
Outputs:
data[] : The FFT or IFFT results are stored in data, overwriting the input.
*/

void XYline::four1(double data[], int nn, int isign)
{
	int n, mmax, m, j, istep, i;
	double wtemp, wr, wpr, wpi, wi, theta;
	double tempr, tempi;

	n = nn << 1;
	j = 1;
	for (i = 1; i < n; i += 2) {
		if (j > i) {
			tempr = data[j];     data[j] = data[i];     data[i] = tempr;
			tempr = data[j + 1]; data[j + 1] = data[i + 1]; data[i + 1] = tempr;
		}
		m = n >> 1;
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}
	mmax = 2;
	while (n > mmax) {
		istep = 2 * mmax;
		theta = TWOPI / (isign*mmax);
		wtemp = sin(0.5*theta);
		wpr = -2.0*wtemp*wtemp;
		wpi = sin(theta);
		wr = 1.0;
		wi = 0.0;
		for (m = 1; m < mmax; m += 2) {
			for (i = m; i <= n; i += istep) {
				j = i + mmax;
				tempr = wr*data[j] - wi*data[j + 1];
				tempi = wr*data[j + 1] + wi*data[j];
				data[j] = data[i] - tempr;
				data[j + 1] = data[i + 1] - tempi;
				data[i] += tempr;
				data[i + 1] += tempi;
			}
			wr = (wtemp = wr)*wpr - wi*wpi + wr;
			wi = wi*wpr + wtemp*wpi + wi;
		}
		mmax = istep;
	}
}

/******************************************/

XYline XYline::getFFT()
{
	int Nx = numpts;
	// calculate NFFT as the next higher power of 2 >= Nx 
	int NFFT = (int)pow(2.0, ceil(log((double)Nx) / log(2.0)));

	int i;

	double *data_aug;

	/* calculate NFFT as the next higher power of 2 >= Nx */
	NFFT = (int)pow(2.0, ceil(log((double)Nx) / log(2.0)));

	/* allocate memory for NFFT complex numbers (note the +1) */
	data_aug = (double *)malloc((2 * NFFT + 1) * sizeof(double));

	/* Storing x(n) in a complex array to make it work with four1.
	This is needed even though x(n) is purely real in this case. */
	for (i = 0; i<Nx; i++)
	{
		data_aug[2 * i + 1] = y[i];
		data_aug[2 * i + 2] = 0.0;
	}
	/* pad the remainder of the array with zeros (0 + 0 j) */
	for (i = Nx; i<NFFT; i++)
	{
		data_aug[2 * i + 1] = 0.0;
		data_aug[2 * i + 2] = 0.0;
	}

	four1(data_aug, NFFT, 1);


	// now convert to power spectrum, and remove the second half of the signal (mirror symmetry of real function).
	float Fs = 1.0 / dx;
	float df = ((Fs / 2) / (NFFT / 2));
	std::vector<float> powerspectrum;
	powerspectrum.resize(NFFT / 2 + 1);
	for (int k = 0; k<NFFT / 2 + 1; k++)
	{
		powerspectrum[k] = sqrt(data_aug[2 * k + 1] / NFFT*data_aug[2 * k + 1] / NFFT + data_aug[2 * k + 2] / NFFT*data_aug[2 * k + 2] / NFFT);
	}
	delete data_aug;

	return XYline(0, df, powerspectrum, 1.0, color);
}


// draw a function.
// stretch Y such that ymin->0 and ymax->plotHeight
// and only display points between [xmin and xmax]
void XYline::draw(Graphics &g, float xmin, float xmax, float ymin, float ymax, int plotWidth, int plotHeight, bool showBounds)
{
	if (ymax - ymin < 1e-6 || xmax - xmin < 1e-6)
		return;

	g.setColour(color);
	if (verticalLine)
	{
		float drawX = (x0 - xmin) / (xmax - xmin) * plotWidth;
		float y0 = (y[0] - ymin) / (ymax - ymin) * plotHeight;
		float y1 = (y[1] - ymin) / (ymax - ymin) * plotHeight;
		g.drawLine(drawX, plotHeight - y0, drawX, plotHeight - y1);
		return;
	}
	// function is given in [x,y], where dx is fixed and known.
	// use bilinear interpolation.
	float xrange = xmax - xmin;
	int screenQuantization;
	if (xrange  < 100 * 1e-3)  // if we are looking at a region that is smaller than 50 ms, try to get better visualization...
	{
		// usually, at this resolution, you see high frequency components. 
		// if you subsample, you will no display things correctly...
		screenQuantization = 1;
	}
	else
	{
		screenQuantization = 5; // draw every 5 pixels
	}
	int numPointsToDraw = plotWidth / screenQuantization;

	float scalex = plotWidth / (numPointsToDraw - 1);

	// bilinear interpolation
	bool prevInrange;
	float prevY = interp(xmin, prevInrange);
	float prevYscaled = (prevY - ymin) / (ymax - ymin) * plotHeight;

	std::vector<int> screenx, bins; // used to draw upper & lower limits of the signal..
	std::vector<float> min_in_bin, max_in_bin; // used to draw upper & lower limits of the signal..
	if (prevInrange) {
		screenx.push_back(0);
		bins.push_back((xmin - x0) / dx);
	}
	bool inrange;


	for (int i = 1; i<numPointsToDraw; i++)
	{
		float x_sample = xmin + i * xrange / (numPointsToDraw - 1);
		float currY = interp(x_sample, inrange);
		float yScaled = (currY - ymin) / (ymax - ymin) * plotHeight;
		if (inrange && prevInrange)
		{
			g.drawLine((i - 1)*scalex, plotHeight - prevYscaled, (i)*scalex, plotHeight - yScaled);
			screenx.push_back(i*scalex);
			bins.push_back((x_sample - x0) / dx);
		}
		prevYscaled = yScaled;
		prevInrange = inrange;
	}

	if (showBounds && screenQuantization > 1)
	{
		// now, we have screen coordinates and corresponding bins.
		// compute minimum and maximum in between each bins...
		for (int i = 0; i<screenx.size() - 1; i++)
		{
			double minV = 1e10;
			double maxV = -1e10;
			for (int k = bins[i]; k <= bins[i + 1]; k++)
			{
				minV = MIN(y[k], minV);
				maxV = MAX(y[k], maxV);
			}
			min_in_bin.push_back(minV);
			max_in_bin.push_back(maxV);
		}
		// now plot upper & lower bounds.
		//	g.setColour(juce::Colours::white);
		float dash_length[2] = { 3, 3 };
		for (int i = 0; i<max_in_bin.size() - 1; i++)
		{
			double upperBoundLeft = (max_in_bin[i] - ymin) / (ymax - ymin)*plotHeight;
			double upperBoundRight = (max_in_bin[i + 1] - ymin) / (ymax - ymin)*plotHeight;

			double lowerBoundLeft = (min_in_bin[i] - ymin) / (ymax - ymin)*plotHeight;
			double lowerBoundRight = (min_in_bin[i + 1] - ymin) / (ymax - ymin)*plotHeight;
			juce::Line<float> upperboundLine(screenx[i], plotHeight - upperBoundLeft, screenx[i + 1], plotHeight - upperBoundRight);
			juce::Line<float> lowerboundLine(screenx[i], plotHeight - lowerBoundLeft, screenx[i + 1], plotHeight - lowerBoundRight);
			g.drawDashedLine(upperboundLine, dash_length, 2, 1);
			g.drawDashedLine(lowerboundLine, dash_length, 2, 1);

		}
	}

}
