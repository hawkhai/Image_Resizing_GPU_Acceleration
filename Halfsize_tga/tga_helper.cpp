//------------------------------------------------------------------------------
// File : tga_helper.cpp
//------------------------------------------------------------------------------
// GLVU :  Mohammed Muddasser work Copyright 2020
//------------------------------------------------------------------------------
// Permission to use, copy, modify, distribute and sell this software and its 
// documentation for any purpose is hereby granted without fee, provided that 
// the above copyright notice appear in all copies and that both that copyright 
// notice and this permission notice appear in supporting documentation. 
// Binaries may be compiled with this software without any royalties or 
// restrictions. 
//
//============================================================================
// tga_helper.hpp : Targa image format module
//============================================================================
// Mohammed Muddasser, Feb 2020 
//

//============================================================================
// Includes
//============================================================================

#include "tga_helper.h""

#define TGA_DEBUG 1

using namespace std;

//----------------------------------------------------------------------------
// READ AND PARSE AN IMAGE FILE. CURRENTLY ONLY SUPPORTS MONO, RGB 16, 24 AND RGBA32 TGA FORMATS.
//
// Input Parameters :	const char *imgName - image to be read
//						uint8_t* &data		- pixel data
//						int &width			- total width (no of pixels)
//						int &height			- total height (no of pixels)
//						int &channels		- number of channels
//								
// Return Parameters:	0 - no error
//						1 - error
// 
//----------------------------------------------------------------------------
void TGA::read(const char *imgName)
{
	// Read the image file.
	std::ifstream inpImg(imgName, ios::in | ios::binary);
	if(!inpImg.is_open())
		throw ("TGA: Failed to open file `%s'.\n", imgName);
  
	// Get file size
	inpImg.seekg(0, ios::end);
	uint32_t iSize = (uint32_t)inpImg.tellg();
	inpImg.seekg(0, ios::beg);

	// Decode the header into header variable
	inpImg.read((char*)&mHeader, sizeof(mHeader));

	// Error handling
	if (inpImg.gcount() != sizeof(mHeader))
	{
		inpImg.close();
		throw ("error when reading header: only %d number of bytes could be read", inpImg.gcount());
	}

	// Decode Image ID
	mID.resize(mHeader.imgIDLength);
	inpImg.read((char *)mID.data(), mHeader.imgIDLength);

	// Error handling
	if (inpImg.gcount() != mHeader.imgIDLength)
	{
		inpImg.close();
		throw ("error when reading ID: only %d number of bytes could be read", inpImg.gcount());
		// The dynamic memory allocated for member variables which are vectors will be deallocated when exception thrown in contructor
	}

	// Read the data
	mPixelDataLen = iSize - sizeof(tgaHeader) - mHeader.imgIDLength;
	mPixelData.resize(mPixelDataLen);

	// Error handling
	if (mPixelDataLen != mPixelData.size())
	{
		inpImg.close();
		throw ("error when resizing data vector and dynamic memeory allocation");
		// The dynamic memory allocated for member variables which are vectors will be deallocated when exception thrown in contructor
	}

	inpImg.read((char *)mPixelData.data(), mPixelDataLen);

	// Error handling
	if (inpImg.gcount() != mPixelDataLen)
	{
		inpImg.close();
		throw ("error when reading pixel data: only %d number of bytes could be read", mPixelDataLen);
		// The dynamic memory allocated for member variables which are vectors will be deallocated when exception thrown in contructor
	}

	// Close the file
	inpImg.close();

#if TGA_DEBUG
	{
		fprintf(stderr, "imgIDLength is - %d\n",		mHeader.imgIDLength);
		fprintf(stderr, "imgColorMapType is - %d\n",	mHeader.imgColorMapType);
		fprintf(stderr, "imgType is - %d\n",			mHeader.imgType);
		fprintf(stderr, "imgColorMapIndex is - %d\n",	mHeader.imgColorMapIndex);
		fprintf(stderr, "imgColorMapLength is - %d\n",	mHeader.imgColorMapLength);
		fprintf(stderr, "imgColorMapSize is - %d\n",	mHeader.imgColorMapSize);
		fprintf(stderr, "imgXOrigin is - %d\n",			mHeader.imgXOrigin);
		fprintf(stderr, "imgYOrigin is - %d\n",			mHeader.imgYOrigin);
		fprintf(stderr, "imgWidth is - %d\n",			mHeader.imgWidth);
		fprintf(stderr, "imgHeight is - %d\n",			mHeader.imgHeight);
		fprintf(stderr, "imgPixelSize is - %d\n",		mHeader.imgPixelSize);
		fprintf(stderr, "imgDesc is - %d\n",			mHeader.imgDesc);
	}

	char imgTypeStr[16][16] = { "NULL", "MAP", "RGB", "MONO", "4", "5", "6", "7", "8",
		"RLE-MAP", "RLE-RGB", "RLE-MONO", "12", "13", "14", "15" };

	fprintf(stderr, "TGA Image type %s bpp = %d\n", imgTypeStr[mHeader.imgType], int(mHeader.imgPixelSize));
#endif

	return;
}

void TGA::write(tgaHeader mheader, std::vector<uint8_t> mID, std::vector<uint8_t> mPixelData, const char *fname)
{

	return;
}