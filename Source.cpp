#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int globalWidth, globalHeight;

struct colour {
	float r, g, b; // Contains a float for each of red, green and blue.

	// Default constructors
	colour();
	colour(float r, float g, float b);
	~colour(); // Destructor
};
// Default constructor definitions for colour
colour::colour() : r(0), g(0), b(0)
{
}
colour::colour(float r, float g, float b) : r(r), g(g), b(b)
{
}
colour::~colour()
{
}


class Bitmap
{
public:
	Bitmap(int width, int height);
	~Bitmap();

	std::vector<int> requiredDimensions(const char* filename);

	void Read(const char* filename);
	void Export(const char* filename) const;

	void GetDimensions() const;

	void InvertPixels();

private:
	int imageWidth;
	int imageHeight;
	std::vector<colour> imageColours;
};

// Couldn't get this constructor to work
// Bitmap::Bitmap() : imageWidth(0), imageHeight(0), imageColours(std::vector<colour>(0)) { }

Bitmap::Bitmap(int width, int height) : imageWidth(width), imageHeight(height), imageColours(std::vector<colour>(width * height))
{
}

Bitmap::~Bitmap() 
{
}

std::vector<int> Bitmap::requiredDimensions(const char* filename)
{
	std::ifstream f;
	f.open(filename, std::ios::in | std::ios::binary); // We will be reading a file in binary

	if (!f.is_open())
	{
		cout << "File could not be read.\n";
		exit(EXIT_FAILURE);
	}

	const int fileHeaderSize = 14;
	const int informationHeaderSize = 40;

	// Read file header into vector of unsigned chars
	unsigned char fileHeader[fileHeaderSize];
	f.read(reinterpret_cast<char*>(fileHeader), fileHeaderSize); // .read function requires argument of type char * so we reinterpret cast to this type for reading

	// Now that we have read the file header, we can check whether the file is a Bitmap by checking whether the first two characters are 'BM'
	if (fileHeader[0] != 'B' || fileHeader[1] != 'M')
	{
		cout << "File is not a Bitmap.\n";
		f.close();
		exit(EXIT_FAILURE);
	}

	// Read information header into vector of unsigned chars
	unsigned char informationHeader[informationHeaderSize];
	f.read(reinterpret_cast<char*>(informationHeader), informationHeaderSize);

	// Extract necessary dimensions from information header vector using knowledge of standard BMP format, ensuring to shift the bytes to correctly read the integer
	int imageWidth = informationHeader[4] + (informationHeader[5] << 8) + (informationHeader[6] << 16) + (informationHeader[7] << 24);
	int imageHeight = informationHeader[8] + (informationHeader[9] << 8) + (informationHeader[10] << 16) + (informationHeader[11] << 24);
	f.close();

	std::vector<int> v{ imageWidth, imageHeight };
	return v;
}


void Bitmap::Read(const char* filename)
{
	std::ifstream f;
	f.open(filename, std::ios::in | std::ios::binary); // We will be reading a file in binary
	
	if (!f.is_open())
	{
		cout << "File could not be read.\n";
		exit(EXIT_FAILURE);
	}

	const int fileHeaderSize = 14;
	const int informationHeaderSize = 40;

	unsigned char fileHeader[fileHeaderSize];
	f.read(reinterpret_cast<char*>(fileHeader), fileHeaderSize);

	// Now that we have read the file header, we can check whether the file is a Bitmap by checking whether the first two characters are 'BM'
	if (fileHeader[0] != 'B' || fileHeader[1] != 'M')
	{
		cout << "File is not a Bitmap.\n";
		f.close();
		exit(EXIT_FAILURE);
	}

	unsigned char informationHeader[informationHeaderSize];
	f.read(reinterpret_cast<char*>(informationHeader), informationHeaderSize);

	const int imageWidth = informationHeader[4] + (informationHeader[5] << 8) + (informationHeader[6] << 16) + (informationHeader[7] << 24);
	const int imageHeight = informationHeader[8] + (informationHeader[9] << 8) + (informationHeader[10] << 16) + (informationHeader[11] << 24);
	
	imageColours.resize(imageWidth * imageHeight);

	const int paddingAmount = ((4 - (imageWidth * 3) % 4) % 4); // Detect how much padding is at the end of each row
	

	for (int y = 0; y < imageHeight; y++)
	{
		for (int x = 0; x < imageWidth; x++)
		{
			unsigned char pixelColour[3];
			f.read(reinterpret_cast<char*>(pixelColour), 3);
			
			// These are inverted due to the way Bitmaps are read
			imageColours[y * imageWidth + x].r = static_cast<float>(pixelColour[2]) / 255.0f;
			imageColours[y * imageWidth + x].g = static_cast<float>(pixelColour[1]) / 255.0f;
			imageColours[y * imageWidth + x].b = static_cast<float>(pixelColour[0]) / 255.0f;
		}
		
		f.ignore(paddingAmount);
	}

	f.close();

	cout << "File read.\n";
}

void Bitmap::Export(const char* filename) const
{
	std::ofstream f;
	f.open(filename, std::ios::out | std::ios::binary); // We will be writing a file in binary
	
	if (!f.is_open())
	{
		cout << "File could not be opened for writing.\n";
		exit(EXIT_FAILURE);
	}

	unsigned char bmpPad[3] = { 0, 0, 0 };
	const int paddingAmount = ((4 - (imageWidth * 3) % 4) % 4); // Works out how many bytes we need to fill at the end of each row to ensure we have a multiple of four bytes

	const int fileHeaderSize = 14;
	const int informationHeaderSize = 40;
	const unsigned fileSize = fileHeaderSize + informationHeaderSize + imageWidth * imageHeight * 3 + paddingAmount * imageHeight; // Padding amount is per row

	unsigned char fileHeader[fileHeaderSize];

	// File type
	fileHeader[0] = 'B';
	fileHeader[1] = 'M';
	// File size
	fileHeader[2] = fileSize;
	fileHeader[3] = fileSize >> 8;
	fileHeader[4] = fileSize >> 16;
	fileHeader[5] = fileSize >> 24;
	// Reserved 1
	fileHeader[6] = 0;
	fileHeader[7] = 0;
	// Reserved 2
	fileHeader[8] = 0;
	fileHeader[9] = 0;
	// Pixel data offset
	fileHeader[10] = fileHeaderSize + informationHeaderSize;
	fileHeader[11] = 0;
	fileHeader[12] = 0;
	fileHeader[13] = 0;

	unsigned char informationHeader[informationHeaderSize];

	// Header size
	informationHeader[0] = informationHeaderSize;
	informationHeader[1] = 0;
	informationHeader[2] = 0;
	informationHeader[3] = 0;
	// Image width
	informationHeader[4] = imageWidth;
	informationHeader[5] = imageWidth >> 8;
	informationHeader[6] = imageWidth >> 16;
	informationHeader[7] = imageWidth >> 24;
	// Image height
	informationHeader[8] = imageHeight;
	informationHeader[9] = imageHeight >> 8;
	informationHeader[10] = imageHeight >> 16;
	informationHeader[11] = imageHeight >> 24;
	// Planes
	informationHeader[12] = 1;
	informationHeader[13] = 0;
	// Bits per pixel (RGB)
	informationHeader[14] = 24;
	informationHeader[15] = 0;
	// Compression
	informationHeader[16] = 0;
	informationHeader[17] = 0;
	informationHeader[18] = 0;
	informationHeader[19] = 0;
	
	// Image size, printing options and colour palette options are all zero
	for (int i = 20; i <= 39; i++) informationHeader[i] = 0;

	f.write(reinterpret_cast<char*>(fileHeader), fileHeaderSize);
	f.write(reinterpret_cast<char*>(informationHeader), informationHeaderSize);

	// Loop through each pixel of the image, obtaining its colour and then casting it to unsigned char and finally writing the RGB triplets to the file
	for (int y = 0; y < imageHeight; y++)
	{
		for (int x = 0; x < imageWidth; x++)
		{
			unsigned char r = static_cast<unsigned char>(imageColours[y * imageWidth + x].r * 255.0f);
			unsigned char g = static_cast<unsigned char>(imageColours[y * imageWidth + x].g * 255.0f);
			unsigned char b = static_cast<unsigned char>(imageColours[y * imageWidth + x].b * 255.0f);

			unsigned char pixelColour[] = { b, g ,r }; // Reverse order due to the way Bitmaps are read

			f.write(reinterpret_cast<char*>(pixelColour), 3);
		}

		f.write(reinterpret_cast<char*>(bmpPad), paddingAmount); // Add padding to end of each row
	}
	
	f.close();

	cout << "File exported.\n";
}

void Bitmap::GetDimensions() const // Used for testing
{
	cout << "The image is " << imageHeight << " by " << imageWidth << " pixels.\n";
}

void Bitmap::InvertPixels() // Loops through each pixel of the image and inverts the intensity of each component of its colour vector
{
	for (int y = 0; y < imageHeight; y++)
	{
		for (int x = 0; x < imageWidth; x++)
		{
			imageColours[y * imageWidth + x].r = 1.0f - imageColours[y * imageWidth + x].r;
			imageColours[y * imageWidth + x].g = 1.0f - imageColours[y * imageWidth + x].g;
			imageColours[y * imageWidth + x].b = 1.0f - imageColours[y * imageWidth + x].b;
		}
	}

	cout << "File inverted.\n";
}


void Invert(const char* filename)
{
	std::string output = std::string(filename);

	cout << "File selected for inversion: " << output << "\n";

	Bitmap dummy(0, 0); // Initialise empty Bitmap object

	std::vector<int> v = dummy.requiredDimensions(filename); // My 'Read' function only seems to work if I call it on a Bitmap object that is already the correct size

	Bitmap invert(v[0], v[1]); // Initialise Bitmap object of correct dimensions

	invert.Read(filename); // Read the file onto the new Bitmap object

	invert.InvertPixels(); // Perform colour inversion

	output.resize(output.size() - 4); // Prepare file name for renaming by removing ".bmp" from the end
	output += "_invert.bmp"; // Add "_invert" to file name
	const char* c = output.c_str(); // Convert back to const char * type

	invert.Export(c); // Export inverted Bitmap with correct file name
}

int main()
{
	Invert("smiley.bmp");
	Invert("Arrows.bmp");
	Invert("Screen.bmp");

	return 0;
}