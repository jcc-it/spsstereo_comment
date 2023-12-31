/*
    Copyright (C) 2014  Koichiro Yamaguchi

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


/* 
****** c++ 17中的库的方法

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    std::string folderPath = "/path/to/folder";  // 替换为实际的文件夹路径

    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string fileName = entry.path().filename().string();
            // 在此处可以对文件名进行处理
            std::cout << fileName << std::endl;
        }
    }

    return 0;
} */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <png++/png.hpp>
#include "SPSStereo.h"
#include "defParameter.h"
#include <dirent.h>


void makeSegmentBoundaryImage(const png::image<png::rgb_pixel>& inputImage,
							  const png::image<png::gray_pixel_16>& segmentImage,
							  std::vector< std::vector<int> >& boundaryLabels,
							  png::image<png::rgb_pixel>& segmentBoundaryImage);
void writeDisparityPlaneFile(const std::vector< std::vector<double> >& disparityPlaneParameters, const std::string outputDisparityPlaneFilename);
void writeBoundaryLabelFile(const std::vector< std::vector<int> >& boundaryLabels, const std::string outputBoundaryLabelFilename);
void spsOnePreePng(std::string left,std::string right);


int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "usage: sgmstereo left right" << std::endl;
		exit(1);
	}
	
    std::string leftFilename = argv[1];
    std::string rightFilename = argv[2];
	// const char* directory = argv[1];
	// DIR* dir = opendir(directory );
	// if (dir == nullptr)
    // {
    //     std::cerr << "Failed to open directory: " << directory << std::endl;
    //     return 0;
    // }
	// struct dirent* entry;
    // while ((entry = readdir(dir)) != nullptr)
    // {
	// 	std::string filename = entry->d_name;
	// 	std::cout<<"proceing pic"<<filename<<std::endl;
	// 	if(filename=="."|| filename==".."){
	// 		std::cout<<"erro:"<<filename<<std::endl;
	// 		continue;
	// 	}
	// 	else{
	// 		spsOnePreePng(leftFilename+filename,rightFilename+filename);
	// 		}
		
    // }
    // closedir(dir);

	spsOnePreePng(leftFilename, rightFilename);
	return 0;


}

void spsOnePreePng(std::string left,std::string right){
	png::image<png::rgb_pixel> leftImage(left);                          //***将left图像文件加载到leftimage对象中
	png::image<png::rgb_pixel> rightImage(right);

	SPSStereo sps;
    sps.setIterationTotal(outerIterationTotal, innerIterationTotal);		//***迭代次数大于等于1
    sps.setWeightParameter(lambda_pos, lambda_depth, lambda_bou, lambda_smo); 		//***权重参数大于等于0
    sps.setInlierThreshold(lambda_d);		//***内点阈值大于0
    sps.setPenaltyParameter(lambda_hinge, lambda_occ, lambda_pen);
    
	png::image<png::gray_pixel_16> segmentImage;
	png::image<png::gray_pixel_16> disparityImage;
	std::vector< std::vector<double> > disparityPlaneParameters;
	std::vector< std::vector<int> > boundaryLabels;
	sps.compute(superpixelTotal, leftImage, rightImage, segmentImage, disparityImage, disparityPlaneParameters, boundaryLabels);

	//wendy 7.21
	// png::image<png::rgb_pixel> segmentBoundaryImage;
	// makeSegmentBoundaryImage(leftImage, segmentImage, boundaryLabels, segmentBoundaryImage);
	
	std::string outputBaseFilename = left;
	size_t slashPosition = outputBaseFilename.rfind('/');
	if (slashPosition != std::string::npos) outputBaseFilename.erase(0, slashPosition+1);
	size_t dotPosition = outputBaseFilename.rfind('.');
	if (dotPosition != std::string::npos) outputBaseFilename.erase(dotPosition);
	std::string outputDisparityImageFilename = outputBaseFilename + ".png";
	//std::string outputSegmentImageFilename = outputBaseFilename + "_segment.png";
	//std::string outputBoundaryImageFilename = outputBaseFilename + "_boundary.png";
	//std::string outputDisparityPlaneFilename = outputBaseFilename + "_plane.txt";
	//std::string outputBoundaryLabelFilename = outputBaseFilename + "_label.txt";

	disparityImage.write(outputDisparityImageFilename);
	//segmentImage.write(outputSegmentImageFilename);
	//segmentBoundaryImage.write(outputBoundaryImageFilename);
	//writeDisparityPlaneFile(disparityPlaneParameters, outputDisparityPlaneFilename);
	//writeBoundaryLabelFile(boundaryLabels, outputBoundaryLabelFilename);

}

void makeSegmentBoundaryImage(const png::image<png::rgb_pixel>& inputImage,
							  const png::image<png::gray_pixel_16>& segmentImage,
							  std::vector< std::vector<int> >& boundaryLabels,
							  png::image<png::rgb_pixel>& segmentBoundaryImage)
{
	int width = static_cast<int>(inputImage.get_width());
	int height = static_cast<int>(inputImage.get_height());
	int boundaryTotal = static_cast<int>(boundaryLabels.size());

	segmentBoundaryImage.resize(width, height);
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			segmentBoundaryImage.set_pixel(x, y, inputImage.get_pixel(x, y));
		}
	}

	int boundaryWidth = 2;
	for (int y = 0; y < height - 1; ++y) {
		for (int x = 0; x < width - 1; ++x) {
			int pixelLabelIndex = segmentImage.get_pixel(x, y);

			if (segmentImage.get_pixel(x + 1, y) != pixelLabelIndex) {
				for (int w = 0; w < boundaryWidth - 1; ++w) {
					if (x - w >= 0) segmentBoundaryImage.set_pixel(x - w, y, png::rgb_pixel(128, 128, 128));
				}
				for (int w = 1; w < boundaryWidth; ++w) {
					if (x + w < width) segmentBoundaryImage.set_pixel(x + w, y, png::rgb_pixel(128, 128, 128));
				}
			}
			if (segmentImage.get_pixel(x, y + 1) != pixelLabelIndex) {
				for (int w = 0; w < boundaryWidth - 1; ++w) {
					if (y - w >= 0) segmentBoundaryImage.set_pixel(x, y - w, png::rgb_pixel(128, 128, 128));
				}
				for (int w = 1; w < boundaryWidth; ++w) {
					if (y + w < height) segmentBoundaryImage.set_pixel(x, y + w, png::rgb_pixel(128, 128, 128));
				}
			}
		}
	}

	boundaryWidth = 7;
	for (int y = 0; y < height - 1; ++y) {
		for (int x = 0; x < width - 1; ++x) {
			int pixelLabelIndex = segmentImage.get_pixel(x, y);

			if (segmentImage.get_pixel(x + 1, y) != pixelLabelIndex) {
				png::rgb_pixel negativeSideColor, positiveSideColor;
				int pixelBoundaryIndex = -1;
				for (int boundaryIndex = 0; boundaryIndex < boundaryTotal; ++boundaryIndex) {
					if ((boundaryLabels[boundaryIndex][0] == pixelLabelIndex && boundaryLabels[boundaryIndex][1] == segmentImage.get_pixel(x + 1, y))
						|| (boundaryLabels[boundaryIndex][0] == segmentImage.get_pixel(x + 1, y) && boundaryLabels[boundaryIndex][1] == pixelLabelIndex))
					{
						pixelBoundaryIndex = boundaryIndex;
						break;
					}
				}
				if (boundaryLabels[pixelBoundaryIndex][2] == 3) continue;
				else if (boundaryLabels[pixelBoundaryIndex][2] == 2) {
					negativeSideColor.red = 0;  negativeSideColor.green = 225;  negativeSideColor.blue = 0;
					positiveSideColor.red = 0;  positiveSideColor.green = 225;  positiveSideColor.blue = 0;
				} else if (pixelLabelIndex == boundaryLabels[pixelBoundaryIndex][boundaryLabels[pixelBoundaryIndex][2]]) {
					negativeSideColor.red = 225;  negativeSideColor.green = 0;  negativeSideColor.blue = 0;
					positiveSideColor.red = 0;  positiveSideColor.green = 0;  positiveSideColor.blue = 225;
				} else {
					negativeSideColor.red = 0;  negativeSideColor.green = 0;  negativeSideColor.blue = 225;
					positiveSideColor.red = 225;  positiveSideColor.green = 0;  positiveSideColor.blue = 0;
				}

				for (int w = 0; w < boundaryWidth - 1; ++w) {
					if (x - w >= 0) segmentBoundaryImage.set_pixel(x - w, y, negativeSideColor);
				}
				for (int w = 1; w < boundaryWidth; ++w) {
					if (x + w < width) segmentBoundaryImage.set_pixel(x + w, y, positiveSideColor);
				}
			}
			if (segmentImage.get_pixel(x, y + 1) != pixelLabelIndex) {
				png::rgb_pixel negativeSideColor, positiveSideColor;
				int pixelBoundaryIndex = -1;
				for (int boundaryIndex = 0; boundaryIndex < boundaryTotal; ++boundaryIndex) {
					if ((boundaryLabels[boundaryIndex][0] == pixelLabelIndex && boundaryLabels[boundaryIndex][1] == segmentImage.get_pixel(x, y + 1))
						|| (boundaryLabels[boundaryIndex][0] == segmentImage.get_pixel(x, y + 1) && boundaryLabels[boundaryIndex][1] == pixelLabelIndex))
					{
						pixelBoundaryIndex = boundaryIndex;
						break;
					}
				}
				if (boundaryLabels[pixelBoundaryIndex][2] == 3) continue;
				else if (boundaryLabels[pixelBoundaryIndex][2] == 2) {
					negativeSideColor.red = 0;  negativeSideColor.green = 225;  negativeSideColor.blue = 0;
					positiveSideColor.red = 0;  positiveSideColor.green = 225;  positiveSideColor.blue = 0;
				} else if (pixelLabelIndex == boundaryLabels[pixelBoundaryIndex][boundaryLabels[pixelBoundaryIndex][2]]) {
					negativeSideColor.red = 225;  negativeSideColor.green = 0;  negativeSideColor.blue = 0;
					positiveSideColor.red = 0;  positiveSideColor.green = 0;  positiveSideColor.blue = 225;
				} else {
					negativeSideColor.red = 0;  negativeSideColor.green = 0;  negativeSideColor.blue = 225;
					positiveSideColor.red = 225;  positiveSideColor.green = 0;  positiveSideColor.blue = 0;
				}

				for (int w = 0; w < boundaryWidth - 1; ++w) {
					if (y - w >= 0) segmentBoundaryImage.set_pixel(x, y - w, negativeSideColor);
				}
				for (int w = 1; w < boundaryWidth; ++w) {
					if (y+ w < height) segmentBoundaryImage.set_pixel(x, y + w, positiveSideColor);
				}
			}
		}
	}
}

void writeDisparityPlaneFile(const std::vector< std::vector<double> >& disparityPlaneParameters, const std::string outputDisparityPlaneFilename) {
	std::ofstream outputFileStream(outputDisparityPlaneFilename.c_str(), std::ios_base::out);
	if (outputFileStream.fail()) {
		std::cerr << "error: can't open file (" << outputDisparityPlaneFilename << ")" << std::endl;
		exit(0);
	}

	int segmentTotal = static_cast<int>(disparityPlaneParameters.size());
	for (int segmentIndex = 0; segmentIndex < segmentTotal; ++segmentIndex) {
		outputFileStream << disparityPlaneParameters[segmentIndex][0] << " ";
		outputFileStream << disparityPlaneParameters[segmentIndex][1] << " ";
		outputFileStream << disparityPlaneParameters[segmentIndex][2] << std::endl;
	}

	outputFileStream.close();
}

void writeBoundaryLabelFile(const std::vector< std::vector<int> >& boundaryLabels, const std::string outputBoundaryLabelFilename) {
	std::ofstream outputFileStream(outputBoundaryLabelFilename.c_str(), std::ios_base::out);
	if (outputFileStream.fail()) {
		std::cerr << "error: can't open output file (" << outputBoundaryLabelFilename << ")" << std::endl;
		exit(1);
	}

	int boundaryTotal = static_cast<int>(boundaryLabels.size());
	for (int boundaryIndex = 0; boundaryIndex < boundaryTotal; ++boundaryIndex) {
		outputFileStream << boundaryLabels[boundaryIndex][0] << " ";
		outputFileStream << boundaryLabels[boundaryIndex][1] << " ";
		outputFileStream << boundaryLabels[boundaryIndex][2] << std::endl;
	}
	outputFileStream.close();
}
