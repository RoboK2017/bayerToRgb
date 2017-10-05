#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Command line parser
#include "cxxopts.hpp"

namespace {
   const int IMAGE_WIDTH = 752;
   const int IMAGE_HEIGHT = 480;
   const int BUFFERSIZE = 360961;
   const char * FILENAME = "AC_home";
   const int DELAY_IN_MS = 1;
}

void saveImage(const cv::Mat& img, const char *convertedFileName);
void showConvertedImage(const char * fileName, const char * data, bool debugMode, bool showImage);
cv::Mat loadRawImageFromBuffer(const char *buffer);
cv::Mat loadRawImage(const char *s);
char *loadRawData(const char *s, int size);
cv::Mat convertImage(const cv::Mat &bayer);

int main(int argc, char ** argv) {

   cxxopts::Options options("ImageReader", "The programe to convert Bayer format(MT9V034 specifically) to RGB for further processing.");
   options.add_options()
      ("d,debug", "Enable debugging, this will save the raw Bayer file for each frame as well as the converted Jpeg file.")
      ("s,show", "Show converted Jpeg file for each Bayer file converted.");
   options.parse(argc, argv);

   bool debugMode = options.count("debug") > 0;
   bool showImage = options.count("show") > 0;

   char buffer[BUFFERSIZE];

   std::ifstream fin(FILENAME, std::ios::in | std::ios::binary);
   fin.read(buffer, BUFFERSIZE);
   if (fin.good())
   {
      unsigned int totalImages = 0;
      while (fin.read(buffer, BUFFERSIZE)) {
         std::streamsize s = fin.gcount();
         if (s != BUFFERSIZE) {
            std::cerr << "Error reading number of bytes per image, should be " << BUFFERSIZE << ", but got " << s << ". System exiting with error code -1\n";
            fin.close();
            exit(-1);
            std::getchar();
         }
         uchar sequenceNum = buffer[0];
         std::string imgName = "img_" + std::to_string(sequenceNum);
         std::cout << "Read image: " << imgName << '\n';
         if (debugMode) {
            std::ofstream outfile(imgName, std::ios::out | std::ios::binary);
            std::cout << "[DEBUG] Writing image: " << imgName << '\n';
            outfile.write(buffer + 1, BUFFERSIZE - 1);
            outfile.close();
         }
         ++totalImages;
         showConvertedImage(imgName.c_str() , buffer + 1, debugMode, showImage);
      }
      std::cout << "Finished reading file " << FILENAME << ". " << totalImages << " files have been created\n";
      fin.close();
   }
   else {
      std::cerr << "Cannot open the input file!, system existing with error code -1\n";
      fin.close();
      std::getchar();
      exit(-1);
   }
   std::cout << "Please press ANY key to close the program." << std::endl;
   std::getchar();
   return 0;
}


cv::Mat loadRawImage(const char *s)
{
   cv::Mat img(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC1);
   memcpy(img.data, loadRawData(s, sizeof(char) * img.cols * img.rows),
          sizeof(char) * img.cols * img.rows);

   if (!img.data) {
      img.release();
      std::cerr << "loadRawData() failed\n";
      exit(-1);
   }

   return img;
}

cv::Mat loadRawImageFromBuffer(const char *buffer)
{
   cv::Mat img(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC1);
   memcpy(img.data, buffer, sizeof(unsigned char) * img.cols * img.rows);

   if (!img.data) {
      img.release();
      std::cerr << "Copy image buffer failed\n";
      exit(-1);
   }

   return img;
}


char *loadRawData(const char *s, int size)
{
   char *buffer = (char *)malloc(size);
   std::ifstream fin(s, std::ios::in | std::ios::binary);
   fin.read(buffer, size);

   if (fin.good())
   {
      if (fin.read(buffer, BUFFERSIZE)) {
         std::streamsize s = fin.gcount();
         if (s != BUFFERSIZE) {
            std::cerr << "Error reading number of bytes per image, should be " << BUFFERSIZE << ", but got " << s << ". System exiting with error code -1\n";
            fin.close();
            free(buffer);
            return NULL;
         }
      }
   }

   fin.close();
   return buffer;
}

cv::Mat convertImage(const cv::Mat &bayer)
{
   cv::Mat dst(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3);

   cv::cvtColor(bayer, dst, CV_BayerGR2BGR);
   return dst;
}

void saveImage(const cv::Mat& img, const char *convertedFileName)
{
   if (!convertedFileName)
   {
      std::cerr << "Converted file name cannot be null!\n";
      return;
   }
   cv::imwrite(convertedFileName, img);

}

void showConvertedImage(const char * fileName, const char * data, bool debugMode, bool showImage)
{
   cv::Mat bayer;
   cv::Mat rgb;
   if (debugMode) {
      std::cout << "[DEBUG] Read from raw Bayer file\n";
      bayer = loadRawImage(fileName);
   }
   else {
      bayer = loadRawImageFromBuffer(data);
   }

   if (!bayer.data) {
      std::cout << "Failed to load raw image. DebugMode " << (debugMode ? "Yes" : "No") << "\n";
      exit(1);
   }

   rgb = convertImage(bayer);

   if (rgb.data) {
      if (showImage) {
         cv::namedWindow("convertedJpeg"); // Create a window for display.
         cv::imshow("convertedJpeg", rgb);
         cv::waitKey(DELAY_IN_MS);
      }
      saveImage(rgb, std::string(std::string(fileName) + ".jpeg").c_str());
   }

   bayer.release();

   if (rgb.data)
      rgb.release();
}
