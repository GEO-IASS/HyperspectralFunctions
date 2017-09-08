/* ————————————————————————————————————————————————————————————————————————————————————————————————
* Imports a single wavelength of a hyperspectral .raw data file into matlab as a 2D slice.
———————————————————————————————————————————————————————————————————————————————————————————————————
This is a MEX function which imports a single band/wavelength of a multi-band/hyperspectral image
stored in band interleaved by line (BIL) format. The function calculates what the the linear position 
of every element in a band should be based on given input dimensions and returns them as a 2D image.
The data which is to be imported must be stored as uint16.

The function takes the following 5 inputs:

	       (1) Name of .raw file to import       (string)
		   (2) Rows of hypercube to import       (double)
		   (3) Columns of hypercube to import    (double)
		   (4) Bands of hypercube to import      (double)
		   (5) index of wavelength to import     (double)

Outputs:
		   (1) 2D image [rows-by-cols]           (double)

Example on how to compile and run from Matlab:
% Compile .C to .mexw64
>> mex ImportSingleWavelengthMEX.c

% Run from Matlab when compiled:

FileName = 'myimage.raw';

% The following metadata can be found in the .hdr headerfile
rows = 1312;
cols = 720;		 
bands = 200;

BandToImport = 100;

OneBandImage = ImportSingleWavelengthMEX(FileName, rows, cols, bands, BandToImport);


 Written 2017-08-23 by
 petter.stefansson@nmbu.no
 ———————————————————————————————————————————————————————————————————————————————————————————————— */

#include "mex.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	
	/* Variable declaration. */
	char *input_filename;
	uint64_T Rows, Cols, Bands, BandToImport, row, c;
	double *HyperCube;

	/* Gather Matlab inputs. */
	input_filename = mxArrayToString(prhs[0]);
	Rows = (uint64_T)mxGetScalar(prhs[1]);
	Cols = (uint64_T)mxGetScalar(prhs[2]);
	Bands = (uint64_T)mxGetScalar(prhs[3]);
	BandToImport = (uint64_T)mxGetScalar(prhs[4]); // expressed as matlabindex, i.e. 1 is the first band

	/* Create Matlab output cube according to given dimensions. */
	plhs[0] = mxCreateDoubleMatrix(Rows, Cols, mxREAL);

	/* Open file to import. */
	FILE *rawfile;
	rawfile = fopen(input_filename, "rb");

	/* Skip elements so that the pointer points to the first element of interest */
	_fseeki64(rawfile, sizeof(uint16_T) * Cols * (BandToImport-1), SEEK_CUR);

	/* Create Matlab output variable. */
	HyperCube = mxGetPr(plhs[0]);

	/* uint16 buffer variable which will hold one (row,:,wl) slice of the hypercube at a time as it is being imported. */
	uint16_T *HCslice;
	HCslice = (uint16_T*)malloc(sizeof(uint16_T)  * Cols);

	/* Loop all rows of the hypercube. */
	for (row = 0; row < Rows; row++) {

		/* Read in all elements in the current row of the current band */
		fread(HCslice, sizeof(uint16_T), Cols, rawfile);

		/* Loop columns/elements of current imported slice. */
		for (c = 0; c < Cols; c++) {

			/* Place the elements in the 2D slice */
			HyperCube[row + (Rows * c)] = HCslice[c];
		}
		/* Skip elements to end up at the next element of interest */
		_fseeki64(rawfile, sizeof(uint16_T) * (Bands - 1)*Cols, SEEK_CUR);

	}
	/* Close file before exit. */
	fclose(rawfile);

	/* Free memory allocated for the buffer */
	free(HCslice);
}