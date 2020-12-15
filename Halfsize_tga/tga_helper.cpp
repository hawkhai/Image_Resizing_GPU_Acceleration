//------------------------------------------------------------------------------
// File : tga_helper.cpp
//------------------------------------------------------------------------------

// Copyright 2020 
// Author - Mohammed Muddasser
//------------------------------------------------------------------------------
// Permission to use, copy, modify, distribute and sell this software and its 
// documentation for any purpose is hereby granted without fee, provided that 
// the above copyright notice appear in all copies and that both that copyright 
// notice and this permission notice appear in supporting documentation. 
// Binaries may be compiled with this software without any royalties or 
// restrictions. 
//

//============================================================================
// Includes
//============================================================================
#include "tga_helper.h"

using namespace std;

char imgTypeStr[16][16] = { "NULL", "MAP", "RGB", "MONO", "4", "5", "6", "7", "8",
"RLE-MAP", "RLE-RGB", "RLE-MONO", "12", "13", "14", "15" };

//------------------------------------------------------------------------------------------------
// Read and parse an image file. Currently only supports mono, rgb 16, 24 and rgba32 tga formats.
//
// void TGA::read(const char *imgName)
// Input Parameters :	const char *imgName - image to be read
//								
// Return Parameters:	None
// 
//-------------------------------------------------------------------------------------------------
void TGA::read(const char *imgName)
{
#if TGA_HELPER_DEBUG
	fprintf(stdout, "\n-------------------------------------------------------------\n");
	fprintf(stdout, "In TGA::read function, reading image %s.\n", imgName);
#endif

	// Read the image file.
	std::ifstream inpImg(imgName, ios::in | ios::binary);

	// If file open fails
	if (inpImg.fail())
	{
		fprintf(stderr, "TGA: Failed to open file `%s'. Please check the file path\n", imgName);
		throw ("TGA: Failed to open file `%s'.\n", imgName);
	}

	// If file already open throw exception
	if(!inpImg.is_open())
		throw ("TGA: Failed to open file `%s'.\n", imgName);
  
	// Get file size
	inpImg.seekg(0, ios::end);
	uint32_t iSize = (uint32_t)inpImg.tellg();
	inpImg.seekg(0, ios::beg);

#if TGA_HELPER_DEBUG
	fprintf(stderr, "Image size is %d bytes\n", iSize);
#endif

	// Parse the header into an array
	uint8_t tmpHeader[TGA_HEADER_LENGTH];
	inpImg.read((char*)&tmpHeader, TGA_HEADER_LENGTH);

	// Set header fields accordingly
	mHeader.imgIDLength			= tmpHeader[TGA_IDLENGTH];
	mHeader.imgColorMapType		= tmpHeader[TGA_COLORMAPTYPE];
	mHeader.imgType				= tmpHeader[TGA_TYPE];
	mHeader.imgColorMapIndex	= tmpHeader[(TGA_COLORMAPINDEX + 1)] << 8 | tmpHeader[TGA_COLORMAPINDEX];
	mHeader.imgColorMapLength	= tmpHeader[(TGA_COLORMAPLENGTH + 1)] << 8 | tmpHeader[TGA_COLORMAPLENGTH];
	mHeader.imgColorMapSize		= tmpHeader[TGA_COLORMAPSIZE];
	mHeader.imgXOrigin			= tmpHeader[(TGA_XORIGIN + 1)] << 8	| tmpHeader[TGA_XORIGIN];
	mHeader.imgYOrigin			= tmpHeader[(TGA_YORIGIN + 1)] << 8	| tmpHeader[TGA_YORIGIN];
	mHeader.imgWidth			= tmpHeader[(TGA_WIDTH + 1)] << 8	| tmpHeader[TGA_WIDTH];
	mHeader.imgHeight			= tmpHeader[(TGA_HEIGHT + 1)] << 8	| tmpHeader[TGA_HEIGHT];
	mHeader.imgPixelSize		= tmpHeader[TGA_PIXELSIZE];
	mHeader.imgDesc				= tmpHeader[TGA_DESC];

	// Error handling
	if (inpImg.gcount() != TGA_HEADER_LENGTH)
	{
		inpImg.close();
		throw ("Error when reading header: only %d number of bytes could be read", inpImg.gcount());
	}

#if TGA_HELPER_DEBUG
	fprintf(stdout, "imgIDLength is - %d\n", mHeader.imgIDLength);
	fprintf(stdout, "imgColorMapType is - %d\n", mHeader.imgColorMapType);
	fprintf(stdout, "imgType is - %d\n", mHeader.imgType);
	fprintf(stdout, "imgColorMapIndex is - %d\n", mHeader.imgColorMapIndex);
	fprintf(stdout, "imgColorMapLength is - %d\n", mHeader.imgColorMapLength);
	fprintf(stdout, "imgColorMapSize is - %d\n", mHeader.imgColorMapSize);
	fprintf(stdout, "imgXOrigin is - %d\n", mHeader.imgXOrigin);
	fprintf(stdout, "imgYOrigin is - %d\n", mHeader.imgYOrigin);
	fprintf(stdout, "imgWidth is - %d\n", mHeader.imgWidth);
	fprintf(stdout, "imgHeight is - %d\n", mHeader.imgHeight);
	fprintf(stdout, "imgPixelSize is - %d\n", mHeader.imgPixelSize);
	fprintf(stdout, "imgDesc is - %d\n", mHeader.imgDesc);

	fprintf(stdout, "TGA Image type %s bpp = %d\n", imgTypeStr[mHeader.imgType], int(mHeader.imgPixelSize));

	/*fprintf(stdout, "Header is: \n");
	for (int i = 0; i < sizeof(tmpHeader); i++)
		fprintf(stdout, "%02X \t", tmpHeader[i]);
	fprintf(stdout, "\n");
	*/

#endif

	// Error for unsupported formats (Only Mono, RGB 16, 24 and RGBA 32 are supported)
	if ((mHeader.imgType != TGA_MONO) && (mHeader.imgType != TGA_RGB) || (mHeader.imgType == TGA_RGB && mHeader.imgPixelSize == 16))
	{
		inpImg.close();
		fprintf (stderr, "Error unsupported format %s and pixelsize %d. Currrenty only Uncompressed Monochrome B&W, RGB 24 and RGBA 32 are supported ", imgTypeStr[mHeader.imgType], mHeader.imgPixelSize);
		throw ("Error unsupported format %s. Currrenty only Uncompressed Monochrome B&W, RGB 16, 24 and RGBA 32 are supported ", imgTypeStr[mHeader.imgType]);
	}

	// Parse Image ID
	mID.resize(mHeader.imgIDLength);
	inpImg.read((char *)mID.data(), mHeader.imgIDLength);

#if TGA_HELPER_DEBUG
	
	fprintf(stdout, "ID is: \n");
	for (int i = 0; i < mID.size(); i++)
		fprintf(stdout, "%02X \t", mID[i]);
	fprintf(stdout, "\n");
	
#endif

	// Resizing error handling
	if (inpImg.gcount() != mHeader.imgIDLength)
	{
		inpImg.close();
		fprintf (stderr, "Error when reading ID: only %d number of bytes could be read", (uint32_t)inpImg.gcount());
		throw ("Error when reading ID: only %d number of bytes could be read", inpImg.gcount());
		// The dynamic memory allocated for member variables which are vectors will be deallocated when exception thrown in contructor
	}

	// Parse ColorMapSpecField 
	uint32_t mColorMapSpecSize = mHeader.imgColorMapType ? ((mHeader.imgColorMapSize / 8) * (mHeader.imgColorMapLength)) : 0;

	mColorMapSpec.resize(mColorMapSpecSize);
	inpImg.read((char *)mColorMapSpec.data(), mColorMapSpecSize);

#if TGA_HELPER_DEBUG
	/*if (mColorMapSpec.size() > 0)
	{
		fprintf(stdout, "ColorMapSpecField is: \n");
		for (int i = 0; i < mColorMapSpec.size(); i++)
			fprintf(stdout, "%02X \t", mColorMapSpec[i]);
		fprintf(stdout, "\n");
	}*/
#endif

	// Resizing error handling
	if (inpImg.gcount() != mColorMapSpecSize)
	{
		inpImg.close();
		fprintf (stderr, "Error when reading Color Map Spec: only %d number of bytes could be read", (uint32_t)inpImg.gcount());
		throw ("Error when reading Color Map Spec: only %d number of bytes could be read", inpImg.gcount());
		// The dynamic memory allocated for member variables which are vectors will be deallocated when exception thrown in contructor
	}

	// Calculate the size of data
	// mPixelDataLen = iSize - sizeof(tgaHeader) - mHeader.imgIDLength;					// Works only for TGA initial version
	mPixelDataLen = mHeader.imgWidth * mHeader.imgHeight * (mHeader.imgPixelSize / 8);

	// Parse data from image file
	mPixelData.resize(mPixelDataLen);
	// Resizing error handling
	if (mPixelDataLen != mPixelData.size())
	{
		inpImg.close();
		fprintf (stderr, "Error when resizing data vector and dynamic memory allocation");
		throw ("Error when resizing data vector and dynamic memory allocation");
		// The dynamic memory allocated for member variables which are vectors will be deallocated when exception thrown in contructor
	}
	inpImg.read((char *)mPixelData.data(), mPixelDataLen);

#if TGA_HELPER_DEBUG
	fprintf(stdout, "Data size is %d bytes\n", mPixelDataLen);
	fprintf(stdout, "First 130 bytes of data is: \n");
	for (int i = 0; i < 132; i++)
		fprintf(stdout, "%02X \t", mPixelData[i]);
	fprintf(stdout, "\n");
	
	fprintf(stdout, "-------------------------------------------------------------\n");
#endif

	// Data read error handling
	if (inpImg.gcount() != mPixelDataLen)
	{
		inpImg.close();
		fprintf (stderr, "Error when reading pixel data: only %d number of bytes could be read", (uint32_t)inpImg.gcount());
		throw ("Error when reading pixel data: only %d number of bytes could be read", inpImg.gcount());
		// The dynamic memory allocated for member variables which are vectors will be deallocated when exception thrown in contructor
	}

	// Close the file
	inpImg.close();

	return;
}


//------------------------------------------------------------------------------------------------
// Write the image oblject to a file. Currently only supports mono, rgb 16, 24 and rgba32 tga formats.
//
// bool TGA::write(const char *imgName)
// Input Parameters :	const char *imgName - image to be read
//								
// Return Parameters:	Return 1 for error
//							   0 no error
// 
//-------------------------------------------------------------------------------------------------

bool TGA::write(const char *imgName)
{

#if TGA_HELPER_DEBUG
	fprintf(stdout, "\n-------------------------------------------------------------\n");
	fprintf(stdout, "In TGA::write function, creating image %s.\n", imgName);
#endif

	// Error for unsupported formats (Only Mono, RGB 16, 24 and RGBA 32 are supported)
	if ((mHeader.imgType != TGA_MONO) && (mHeader.imgType != TGA_RGB))
	{
		fprintf(stderr, "Error unsupported format %s. Currrenty only Uncompressed Monochrome B&W, RGB 16, 24 and RGBA 32 are supported ", imgTypeStr[mHeader.imgType]);
		return 1;
	}

	// Performing basic checks on the validity of the header.
	if (mHeader.imgHeight == 0 || mHeader.imgWidth == 0 || mPixelDataLen == 0 || mID.size() == 0 || mPixelData.size() == 0 ||
		mHeader.imgIDLength != mID.size() || (mHeader.imgPixelSize != 8 && mHeader.imgPixelSize != 16 && mHeader.imgPixelSize != 24 && mHeader.imgPixelSize != 32))
	{
		fprintf(stderr, "	\n");

		fprintf(stdout, "imgIDLength is - %d\n", mHeader.imgIDLength);
		fprintf(stdout, "imgColorMapType is - %d\n", mHeader.imgColorMapType);
		fprintf(stdout, "imgType is - %d\n", mHeader.imgType);
		fprintf(stdout, "imgColorMapIndex is - %d\n", mHeader.imgColorMapIndex);
		fprintf(stdout, "imgColorMapLength is - %d\n", mHeader.imgColorMapLength);
		fprintf(stdout, "imgColorMapSize is - %d\n", mHeader.imgColorMapSize);
		fprintf(stdout, "imgXOrigin is - %d\n", mHeader.imgXOrigin);
		fprintf(stdout, "imgYOrigin is - %d\n", mHeader.imgYOrigin);
		fprintf(stdout, "imgWidth is - %d\n", mHeader.imgWidth);
		fprintf(stdout, "imgHeight is - %d\n", mHeader.imgHeight);
		fprintf(stdout, "imgPixelSize is - %d\n", mHeader.imgPixelSize);
		fprintf(stdout, "imgDesc is - %d\n", mHeader.imgDesc);
		fprintf(stdout, "mID.size() is - %d\n", (uint32_t)mID.size());
		fprintf(stdout, "mPixelDataLen is - %d\n", mPixelDataLen);
		fprintf(stdout, "mPixelData.size() is - %d\n", (uint32_t)mPixelData.size());

		return 1;
	}

	// Open file with clear contents (empty file)
	FILE *file;
	if (fopen_s(&file, imgName, "wb") != 0)
	{
		fprintf(stderr, "Error: Failed to open file %s for writing.\n", imgName);
		return 1;
	}

	// temporary count variable
	size_t tmpCount = 0;

	//Update the header with error handling
	if (fwrite(&mHeader.imgIDLength, sizeof(uint8_t), sizeof(mHeader.imgIDLength), file) < sizeof(mHeader.imgIDLength)) {
		fprintf(stderr, "Error: Failed to write the imgIDLength\n");
		fclose(file); return 1;
	}
	if (fwrite(&mHeader.imgColorMapType, sizeof(uint8_t), sizeof(mHeader.imgColorMapType), file) < sizeof(mHeader.imgColorMapType)) {
		fprintf(stderr, "Error: Failed to write the imgColorMapType\n");
		fclose(file); return 1;
	}
	if (fwrite(&mHeader.imgType, sizeof(uint8_t), sizeof(mHeader.imgType), file) < sizeof(mHeader.imgType)) {
		fprintf(stderr, "Error: Failed to write the imgType\n");
		fclose(file); return 1;
	}
	if (fwrite(&mHeader.imgColorMapIndex, sizeof(uint8_t), sizeof(mHeader.imgColorMapIndex), file) < sizeof(mHeader.imgColorMapIndex)) {
		fprintf(stderr, "Error: Failed to write the imgColorMapIndex\n");
		fclose(file); return 1;
	}
	if (fwrite(&mHeader.imgColorMapLength, sizeof(uint8_t), sizeof(mHeader.imgColorMapLength), file) < sizeof(mHeader.imgColorMapLength)) {
		fprintf(stderr, "Error: Failed to write the imgIDLength\n");
		fclose(file); return 1;
	}
	if (fwrite(&mHeader.imgColorMapSize, sizeof(uint8_t), sizeof(mHeader.imgColorMapSize), file) < sizeof(mHeader.imgColorMapSize)) {
		fprintf(stderr, "Error: Failed to write the imgColorMapSize\n");
		fclose(file); return 1;
	}
	if (fwrite(&mHeader.imgXOrigin, sizeof(uint8_t), sizeof(mHeader.imgXOrigin), file) < sizeof(mHeader.imgXOrigin)) {
		fprintf(stderr, "Error: Failed to write the imgXOrigin\n");
		fclose(file); return 1;
	}
	if (fwrite(&mHeader.imgYOrigin, sizeof(uint8_t), sizeof(mHeader.imgYOrigin), file) < sizeof(mHeader.imgYOrigin)) {
		fprintf(stderr, "Error: Failed to write the imgYOrigin\n");
		fclose(file); return 1;
	}
	if (fwrite(&mHeader.imgWidth, sizeof(uint8_t), sizeof(mHeader.imgWidth), file) < sizeof(mHeader.imgWidth)) {
		fprintf(stderr, "Error: Failed to write the imgWidth\n");
		fclose(file); return 1;
	}
	if (fwrite(&mHeader.imgHeight, sizeof(uint8_t), sizeof(mHeader.imgHeight), file) < sizeof(mHeader.imgHeight)) {
		fprintf(stderr, "Error: Failed to write the imgHeight\n");
		fclose(file); return 1;
	}
	if (fwrite(&mHeader.imgPixelSize, sizeof(uint8_t), sizeof(mHeader.imgPixelSize), file) < sizeof(mHeader.imgPixelSize)) {
		fprintf(stderr, "Error: Failed to write the imgPixelSize\n");
		fclose(file); return 1;
	}
	if (fwrite(&mHeader.imgDesc, sizeof(uint8_t), sizeof(mHeader.imgDesc), file) < sizeof(mHeader.imgDesc)) {
		fprintf(stderr, "Error: Failed to write the imgDesc\n");
		fclose(file); return 1;
	}

	// Update the ID
#if TGA_HELPER_DEBUG
	fprintf(stdout, "Size of ID is : %d bytes\n", (uint32_t)mID.size());
#endif

	tmpCount = fwrite(mID.data(), sizeof mID[0], mID.size(), file);
	if(tmpCount < mID.size())
	{
		fprintf(stderr, "Error: Failed to write the complete ID. %d number of bytes written.\n", (uint32_t)tmpCount);
		fclose(file);	return 1;
	}

	// Update the ColorMapSpec
#if TGA_HELPER_DEBUG
	fprintf(stdout, "Size of mColorMapSpec is : %d bytes\n", (uint32_t)mColorMapSpec.size());
#endif

	if (mColorMapSpec.size() > 0)
	{
		tmpCount = fwrite(mColorMapSpec.data(), sizeof mColorMapSpec[0], mColorMapSpec.size(), file);
		if (tmpCount < mColorMapSpec.size())
		{
			fprintf(stderr, "Error: Failed to write the complete ColorMapSpec. %d number of bytes written.\n", (uint32_t)tmpCount);
			fclose(file);	return 1;
		}
	}

	
#if TGA_HELPER_DEBUG
	//fprintf(stdout, "Actual data to be written is %d bytes\n", mPixelDataLen);
	fprintf(stdout, "Data size being written is %d bytes\n", (uint32_t)mPixelData.size());
	/*fprintf(stdout, "First 50 bytes of data is: \n");
	for (int i = 0; i < 50; i++)
		fprintf(stdout, "%02X \t", mPixelData[i]);
	fprintf(stdout, "\n");*/
	fprintf(stdout, "-------------------------------------------------------------\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "-------------------------------------------------------------\n");
#endif

	// Update the Data intothe file
	tmpCount = fwrite(mPixelData.data(), sizeof mPixelData[0], mPixelData.size(), file);
	if (tmpCount < mPixelData.size())
	{
		fprintf(stderr, "Error: Failed to write the complete data. %d number of bytes written.\n", (uint32_t)tmpCount);
		fclose(file);	return 1;
	}

	// Close the file
	fclose(file);

	return 0;
}