//	loveland, Sean		CS230 Section 12159 7/6/2021
//	Fifth Laboratory Assignment	- Count Colors

#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include "windows.h"
using namespace std;
ULONGLONG getTime64(LPFILETIME a);

struct pixel {
	int r = 0;
	int g = 0;
	int b = 0;
};

struct color {
	pixel pixelColor;
	string colorName;
	int count = 0;

	color() {}

	color(int red, int green, int blue, string name) {
		pixelColor.r = red;
		pixelColor.g = green;
		pixelColor.b = blue;
		colorName = name;
		count = 0;
	}
};

bool readColorFile(multimap<int, color>& colors);
bool checkDuplicate(multimap<int, color>& colors, color newColor);
bool readBitmap(multimap<int, color>& colors);
void searchPixelColor(multimap<int, color>& colors, pixel currentPixel);
void printUsedColors(multimap<int, color>& colors);

int main() {
	multimap<int, color> colors;		//multimap to store colors from color file

	//all this is for CPU time
	FILETIME creationTime, exitTime, kernelTime, userTime;
	LPFILETIME creation = &creationTime, exit = &exitTime, kernel = &kernelTime, user = &userTime;
	HANDLE myProcess = GetCurrentProcess();
	SYSTEMTIME loct;
	BOOL gotTime1, gotTime2;
	DWORD failReason;
	ULONGLONG  u1, u2;

	//read color file, exit if something goes wrong
	if (!readColorFile(colors)) {
		return 0;
	}

	//initialize CPU time
	gotTime1 = GetProcessTimes(myProcess, creation, exit, kernel, user);
	if (!gotTime1) {
		failReason = GetLastError();
		cout << "GetProcessTimes failed, Failure reason:" << failReason << endl;
		return 0;
	}
	u1 = getTime64(user);

	GetLocalTime(&loct);
	cout << "The starting local time is:" << loct.wHour << ':' << loct.wMinute << ':' << loct.wSecond << '.' << loct.wMilliseconds << endl;
	double fStartTime = loct.wHour * 3600 + loct.wMinute * 60 + loct.wSecond + (loct.wMilliseconds / 1000.0);

	//read the bitmap file, exit if something goes wrong
	if (!readBitmap(colors)) {
		return 0;
	}

	//get CPU time elapsed
	gotTime2 = GetProcessTimes(myProcess, creation, exit, kernel, user);
	if (!gotTime2) {
		failReason = GetLastError();
		cout << "GetProcessTimes failed, Failure reason:" << failReason << endl;
		return 0;
	}

	GetLocalTime(&loct);
	cout << "The   ending local time is:" << loct.wHour << ':' << loct.wMinute << ':' << loct.wSecond << '.' << loct.wMilliseconds << endl;
	double fEndTime = loct.wHour * 3600 + loct.wMinute * 60 + loct.wSecond + (loct.wMilliseconds / 1000.0);
	u2 = getTime64(user);
	double workUserTime = (u2 - u1) * 100.0 / 1000000000.0;

	double workTime = fEndTime - fStartTime;
	cout << "Elapsed clock time:" << workTime << endl;
	cout << "CPU busy percentage:" << (workUserTime / workTime) * 100 << endl;

	//print the colors used in this file
	printUsedColors(colors);
	return 0;
}

// convert a FILETIME structure to a 64-bit integer
ULONGLONG getTime64(LPFILETIME a) {
	ULARGE_INTEGER work;
	work.HighPart = a->dwHighDateTime;
	work.LowPart = a->dwLowDateTime;
	return work.QuadPart;
}

bool readColorFile(multimap<int, color>& colors) {

	string fileName;
	cout << "Enter the path to your color file: ";
	cin >> fileName;

	ifstream infile(fileName);

	if (!infile) {
		cout << "Unable to open file, terminating program . . .\n";
		return false;
	}

	int red, green, blue;
	string colorName;

	//priming read
	infile >> red;

	while (!infile.eof()) {
		infile >> green;
		infile >> blue;

		infile.ignore();		//ignore the \t character before the string
		getline(infile, colorName);
		infile.clear();			//flush the buffer since we are using getline

		if (red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255) {
			cout << "(" << red << ", " << green << ", " << blue << ") - " << colorName << " is not a valid color, ignoring this data\n";
		}
		else if (!checkDuplicate(colors, color(red, green, blue, colorName))) {
			//MAYBE CHANGE THISSSSS
			colors.insert(make_pair(red + green + blue, color(red, green, blue, colorName)));
		}

		infile >> red;
	}

	infile.close();
	return true;
}

bool checkDuplicate(multimap<int, color>& colors, color newColor) {
	auto range = colors.equal_range(newColor.pixelColor.r + newColor.pixelColor.g + newColor.pixelColor.b);

	for (auto i = range.first; i != range.second; ++i) {
		if (i->second.colorName.compare(newColor.colorName) == 0) {
			return true;
		}
	}

	return false;
}

bool readBitmap(multimap<int, color>& colors) {
	/*
	* REMEMBER TO CHANGE THIS
	* IT IS SET TP UNTITLED, NEEDS TO BE SET TO SOMETHING ELSE
	*/
	ifstream bmpfile("C:/Temp/Untitled.bmp", ios::in | ios::binary);

	if (!bmpfile) {
		cout << "Bitmap file couldn't be opened, data cannot be read . . .\n";
		return false;
	}

	const int fileHeaderSize = 14;	//file header is always 14 bytes long
	const int informationHeaderSize = 40;	//information header is always 40 bytes long

	//read in the file header
	unsigned char fileHeader[fileHeaderSize];
	bmpfile.read(reinterpret_cast<char*>(fileHeader), fileHeaderSize);

	//read in the information header
	unsigned char informationHeader[informationHeaderSize];
	bmpfile.read(reinterpret_cast<char*>(informationHeader), informationHeaderSize);

	//the file size is 4 bytes long and is directly after the first two reserved bytes
	int fileSize = fileHeader[2] + (fileHeader[3] << 8) + (fileHeader[4] << 16) + (fileHeader[5] << 24);

	int imageWidth = informationHeader[4] + (informationHeader[5] << 8) + (informationHeader[6] << 16) + (informationHeader[7] << 24);
	int imageHeight = informationHeader[8] + (informationHeader[9] << 8) + (informationHeader[10] << 16) + (informationHeader[11] << 24);


	const int paddingAmount = ((4 - (imageWidth * 3) % 4) % 4);

	unsigned char bmpcolor[3];
	pixel currentPixel;

	for (int y = 0; y < imageHeight; ++y) {
		for (int x = 0; x < imageWidth; ++x) {
			bmpfile.read(reinterpret_cast<char*>(bmpcolor), 3);

			currentPixel.r = static_cast<int> (bmpcolor[2]);
			currentPixel.g = static_cast<int> (bmpcolor[1]);
			currentPixel.b = static_cast<int> (bmpcolor[0]);

			searchPixelColor(colors, currentPixel);
		}

		bmpfile.ignore(paddingAmount);
	}
}

void searchPixelColor(multimap<int, color>& colors, pixel currentPixel) {
	auto range = colors.equal_range(currentPixel.r + currentPixel.g + currentPixel.b);

	for (auto i = range.first; i != range.second; ++i) {
		if (i->second.pixelColor.r == currentPixel.r && i->second.pixelColor.g == currentPixel.g && i->second.pixelColor.b == currentPixel.b) {
			i->second.count++;
			return;
		}
	}
}

void printUsedColors(multimap<int, color>& colors) {
	multimap<int, color>::iterator it;

	for (it = colors.begin(); it != colors.end(); ++it) {
		if (it->second.count > 0) {
			cout << it->second.count << " pixels were " << it->second.colorName << " (" << it->second.pixelColor.r << ", " << it->second.pixelColor.g << ", " << it->second.pixelColor.b << ")" << endl;
		}
	}
}