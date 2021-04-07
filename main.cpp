#include <time.h>
#include <iostream>
#include <random>
#include <stdlib.h>
#include <string>
#include "bitmap/bitmap.h"
#include "cmdline/cmdline.h"

double mathRand(double start, double end) {
	static bool s_seedFlag = true;
	static std::default_random_engine s_e;
	if (s_seedFlag)
	{
		std::random_device r;
		s_e = std::default_random_engine(r());
	}
	std::uniform_real_distribution<double> uniform_dist(start, end);
	return uniform_dist(s_e);
}

unsigned long long generateUID(void) {
	static int sY = 0, sM = 0, sD = 0, sH = 0, sMM = 0, sS = 0, sIdx = 1;
	time_t t = time(nullptr);
	struct tm* dt = localtime(&t);
	int year = 1900 + dt->tm_year, mon = 1 + dt->tm_mon, mday = dt->tm_mday, hour = dt->tm_hour, min = dt->tm_min, sec = dt->tm_sec;
	if (year == sY && mon == sM && mday == sD && hour == sH && min == sMM && sec == sS) {
		sIdx++;
	}
	else {
		sY = year; sM = mon; sD = mday; sH = hour; sMM = min; sS = sec; sIdx = 1;
	}
	return (unsigned long long)sY * 10000000000000 + (unsigned long long)sM * 100000000000 + (unsigned long long)sD * 1000000000 +
		(unsigned long long)sH * 10000000 + (unsigned long long)sMM * 100000 + (unsigned long long)sS * 1000 + (unsigned long long)sIdx;
}

std::string generateFilename(const std::string& extname) {
	char filename[64] = { 0 };
	sprintf(filename, "%llu%s", generateUID(), extname.empty() ? "" : ('.' == extname.at(0) ? extname.c_str() : ('.' + extname).c_str()));
	return filename;
}

#define DEFAULT_SIZE 6.0

int main(int argc, char* argv[]) {
	cmdline::parser a;
	a.add<std::string>("name", 'n', "image name", false, "");
	a.add<double>("size", 's', "image size(M), range[0.1, 64.0]", false, 0, cmdline::range(0.1, 64.0));
	a.add("help", 0, "print this message");
	a.set_program_name("make_image");

	bool ok = a.parse(argc, argv);

	if (a.exist("help")) {
		std::cerr << a.usage();
		return 0;
	}

	if (!ok) {
		std::cerr << a.error() << std::endl << a.usage();
		return 0;
	}

	double filesizeM = 0;
	if (a.exist("size")) {
		filesizeM = a.get<double>("size");
	}
	if (0 == filesizeM) {
		filesizeM = mathRand(0.1, 10.0);
	}
	double filesize = filesizeM * 1024 * 1024;
	size_t count = (size_t)(filesize / 3); /* 像素点个数(1个像素点占3个字节) */
	size_t width = (size_t)sqrt(count) + 1;
	size_t height = width;

	PixelMatrix mat;
	for (size_t row = 0; row < width; ++row) {
		std::vector<Pixel> rows;
		unsigned int r = (unsigned int)mathRand(0, 255);
		unsigned int g = (unsigned int)mathRand(0, 255);
		unsigned int b = (unsigned int)mathRand(0, 255);
		unsigned int mod = (unsigned int)mathRand(2, 9);
		unsigned int rr = (unsigned int)mathRand(0, 255);
		unsigned int gg = (unsigned int)mathRand(0, 255);
		unsigned int bb = (unsigned int)mathRand(0, 255);
		for (size_t col = 0; col < height; ++col) {
			if (0 == (row * col) % mod) {
				rows.push_back(Pixel(rr, gg, bb));
			}
			else {
				rows.push_back(Pixel(r, g, b));
			}
		}
		mat.push_back(rows);
	}

	std::string filename;
	if (a.exist("name")) {
		filename = a.get<std::string>("name");
	}
	if (filename.empty())
	{
		filename += "img_";
		filename += generateFilename(".jpg");
	}

	Bitmap bmp;
	bmp.fromPixelMatrix(mat);
	bmp.save(filename);

	std::cout << std::endl << "image file: " << filename << ", size: " << filesizeM << "(M), " << width << "x" << height << std::endl;
	return 0;
}
