/* ————————————————————————————————————————————————————————————————————————————————————————————————
Imports a .raw hyperspectral data file into Matlab as a [Rows-by-Cols-by-Wavelengths] hypercube.
———————————————————————————————————————————————————————————————————————————————————————————————————
This is a MEX function which reads an entire hyperspectral image, which must be stored in band 
interleaved by line (BIL) format, and returns it to Matlab. The function does the same thing as 
Matlab's 'multibandread', which for small files is probably just as fast as this function. But 
for large images this function can sometimes be ~4-500 % quicker because it directly maps the 
interleaved linear index to its corresponding 3D column-major linear index which makes the permute 
call which multibandread uses redundant.

The function takes the following 4 inputs:

	       (1) Name of .raw file to import       (string)
		   (2) Rows of hypercube to import       (double)
		   (3) Columns of hypercube to import    (double)
		   (4) Bands of hypercube to import      (double)

And outputs:
		   (1) 3D hypercube [rows-cols-wl]       (double)


Example on how to compile and run from Matlab:
% Compile .C to .mexw64
>> mex ImportHyperCubeMEX.c

% Run from Matlab when compiled:

FileName = 'myimage.raw';

% The following metadata can be found in the .hdr headerfile
rows = 1312;
cols = 720;		 
bands = 200;

HyperCube = ImportHyperCubeMEX(FileName, rows, cols, bands);

 Written 2017-07-07 by
 petter.stefansson@nmbu.no
 ———————————————————————————————————————————————————————————————————————————————————————————————— */

#include "mex.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
	/* Gather Matlab inputs. */
	char *input_filename;
	input_filename = mxArrayToString(prhs[0]);
	double *InputCols;
	InputCols = mxGetPr(prhs[2]);
	double *InputRows;
	InputRows = mxGetPr(prhs[1]);
	double *InputBands;
	InputBands = mxGetPr(prhs[3]);

	/* Make int variables of resolution scalars that are double. */
	int Cols  = *InputCols;
	int Rows  = *InputRows;
	int Bands = *InputBands;;
	
	/* Create Matlab output cube according to given dimensions. */
	mwSize ndim = 3;
	mwSize dims[3];
	dims[1] = Cols;
	dims[2] = Bands;
	dims[0] = Rows;
	plhs[0] = mxCreateNumericArray(ndim, dims, mxDOUBLE_CLASS, mxREAL);

	/* Variable declaration. */
	int row;
	int e;
	int c;
	int lambda;
	double *HyperCube;

	/* Open file to import. */
	FILE *rawfile;
	rawfile = fopen(input_filename, "rb");

	/* Create Matlab output variable. */
	HyperCube = mxGetPr(plhs[0]);

	/* uint16 buffer variable which will hold one (row,:,:) slice of the hypercube at a time as it is being imported. */
	uint16_T HCslice[Cols*Bands];

	/* Loop all rows of the hypercube. */
	for (row = 0; row < Rows; row++) {

		/* Read in one line/row of the hypercube. */
		fread(HCslice, 2, Cols*Bands, rawfile);
		e = 0;

		/* Loop wavelengths of current imported slice. */
		for (lambda = 0; lambda < Bands; lambda++) {

			/* Loop columns of current imported slice. */
			for (c = 0; c < Cols; c++) {

				/* Position current element in 3D array using linear index. */
				HyperCube[row + Rows * (c + Cols * lambda)] = HCslice[e];
				e = e + 1;
			}
		}
	}
	/* Close file before exit. */
	fclose(rawfile);
}