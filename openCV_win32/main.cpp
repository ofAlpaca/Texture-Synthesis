//
//	Algorithms Analysis - Texture Synthesis v1.1
//	- main.cpp
//
//	Created by ShauShian, Chiang on 31/05/2017.
//	Copyright @ 2017 ShauShian, Chiang. All rights reserved.


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <iomanip>
#include <math.h>

using namespace cv;
using namespace std;

char* get_cmd_option(char** begin, char** end, const string & option);
bool cmd_exist(char** begin, char** end, const string & option);
int number_of_pattern(int img_size, int overlap, int total_size);
double **get_eu_distance(Mat& img1, Mat& img2, int img_height, int img_width, bool is_hor);
int* get_shortest_path(double **ary, int station_size, int line_size);
Mat synthesis_two_image_horizontal(Mat image_left, Mat image_right, int overlap);
Mat synthesis_two_image_vertical(Mat image_top, Mat image_down, int overlap);
Mat horizontal_synthesis(Mat image, int overlap, int totalsize);
Mat vertical_synthesis(Mat image, int overlap, int totalsize);

char* get_cmd_option(char** begin, char** end, const string & option) {
	char ** it = find(begin, end, option);
	if (it != end && ++it != end)
		return *it;
	return 0;
}

bool cmd_exist(char** begin, char** end, const string & option) {
	return find(begin, end, option) != end;
}

int main(int argc, char* argv[]) {
	
	char* infilename = get_cmd_option(argv, argv + argc, "-i");
	char* outfilename = get_cmd_option(argv, argv + argc, "-o");
	char* overlap_size = get_cmd_option(argv, argv + argc, "-r");
	char* total_size = get_cmd_option(argv, argv + argc, "-s");

	if (infilename && outfilename && overlap_size && total_size) {
		Mat image = imread(infilename, 1); // Read the file
		if (image.data) // Check image data
		{
			image.convertTo(image, CV_8UC3, 1, 0);

			int overlap = atoi(overlap_size);
			int totalsize = atoi(total_size);

			cout << "Input Image: " << infilename << endl;
			cout << "Overlap: " << overlap << endl;
			cout << "Total size: " << totalsize << endl;
			cout << "Output Image: " << outfilename << endl;

			Mat horz_img = horizontal_synthesis(image, overlap, totalsize);
			Mat final_img = vertical_synthesis(horz_img, overlap, totalsize);
			Mat cut_img = final_img(Rect(0, 0, totalsize, totalsize));

			imshow("Display window", cut_img);
			imwrite(outfilename, cut_img); // Write down the image
			//namedWindow(outfilename, WINDOW_AUTOSIZE);
			//imshow(outfilename, cut_img); // Show our image inside it.
			waitKey(0);
		}
		else // Image is missed
			cout << "Could not open or find the image...\n";
	}
	else if (cmd_exist(argv, argv + argc, "-h"))
	{
		cout << "-i : 輸入影像名稱\n" << "-o : 輸出影像名稱\n" << "-r : 重疊區域像素個數\n" << "-s : 紋理合成大小\n";
	}
	else
		cout << "Wrong command option... try -h option\n";

	return 0;
}

Mat vertical_synthesis(Mat image, int overlap, int totalsize) {
	Size img_size = image.size();

	// How many pattern do we need to match the demanding size
	int pattern_need = number_of_pattern(img_size.height, overlap, totalsize);

	Mat pre_img = image;
	// There are pattern_need - 1 overlaping area
	for (int n = 0; n < pattern_need - 1; n++)
		pre_img = synthesis_two_image_vertical(pre_img, image, overlap);

	return pre_img;
}

Mat horizontal_synthesis( Mat image, int overlap, int totalsize ) {
	Size img_size = image.size();

	// How many pattern do we need to match the demanding size
	int pattern_need = number_of_pattern(img_size.width, overlap, totalsize);

	Mat pre_img = image;
	// There are pattern_need - 1 overlaping area
	for (int n = 0; n < pattern_need - 1; n++)
		pre_img = synthesis_two_image_horizontal(pre_img, image, overlap);

	return pre_img;
}

Mat synthesis_two_image_vertical(Mat image_top, Mat image_down, int overlap) {

	Size top_size = image_top.size();
	Size down_size = image_down.size();

	Mat new_img = Mat::zeros(down_size.height + top_size.height - overlap, down_size.width, CV_8UC3);
	Size new_img_size = new_img.size();

	// The top part that would gonna overlap
	Mat top_part = image_top(Rect(0, top_size.height - overlap, top_size.width, overlap));
	// The down part that would gonna overlap
	Mat down_part = image_down(Rect(0, 0, down_size.width, overlap ));
	// The final result of overlaping
	Mat overlap_part = Mat::zeros(overlap, new_img_size.width, CV_8UC3);

	double **eu_dst = get_eu_distance(top_part, down_part, new_img_size.width, overlap, false);
	int* s_path = get_shortest_path(eu_dst, new_img_size.width, overlap);

	for (int y = 0; y < new_img_size.width; y++) {
		for (int x = 0; x < overlap ; x++) {
			if (x < s_path[y]) {
				overlap_part.at<Vec3b>(x, y) = top_part.at<Vec3b>(x, y);
			}
			else if (x >= s_path[y]) {
				overlap_part.at<Vec3b>(x, y) = down_part.at<Vec3b>(x, y);
			}
			else {
				overlap_part.at<Vec3b>(x,y)[0] = 255;
				overlap_part.at<Vec3b>(x,y)[1] = 255;
				overlap_part.at<Vec3b>(x,y)[2] = 255;
			}
		}
	}

	Mat remain_top_part = image_top(Rect(0, 0, top_size.width, top_size.height - overlap));
	Mat remain_down_part = image_down(Rect(0, overlap, down_size.width, down_size.height - overlap));

	remain_top_part.copyTo(new_img(Rect(0, 0, new_img_size.width , top_size.height - overlap)));
	overlap_part.copyTo(new_img(Rect(0, top_size.height - overlap, new_img_size.width, overlap)));
	remain_down_part.copyTo(new_img(Rect(0, top_size.height, new_img_size.width, down_size.height - overlap )));
	
	return new_img;
}

Mat synthesis_two_image_horizontal(Mat image_left, Mat image_right, int overlap) {

	Size left_size = image_left.size();
	Size right_size = image_right.size();

	Mat new_img = Mat::zeros(right_size.height, right_size.width + left_size.width - overlap, CV_8UC3 );
	Size new_img_size = new_img.size();

	// The left part that would gonna overlap
	Mat left_part = image_left(Rect(left_size.width - overlap, 0, overlap, left_size.height)); 
	// The right part that would gonna overlap
	Mat right_part = image_right(Rect(0, 0, overlap, right_size.height));
	// The final result of overlaping
	Mat overlap_part = Mat::zeros(new_img_size.height, overlap, CV_8UC3);

	double **eu_dst = get_eu_distance(left_part, right_part, new_img_size.height, overlap, true);
	int* s_path = get_shortest_path(eu_dst, new_img_size.height, overlap);

	for (int y = 0 ; y < new_img_size.height ; y++) {
		for (int x = 0 ; x < overlap ; x++) {
			if (x < s_path[y]) {
				overlap_part.at<Vec3b>(y, x) = left_part.at<Vec3b>(y, x);
			}
			else if (x >= s_path[y]) {
				overlap_part.at<Vec3b>(y, x) = right_part.at<Vec3b>(y, x);
			}
			else {
				overlap_part.at<Vec3b>(y, x)[0] = 255;
				overlap_part.at<Vec3b>(y, x)[1] = 255;
				overlap_part.at<Vec3b>(y, x)[2] = 255;
			}
		}
	}

	Mat remain_left_part = image_left(Rect(0, 0, left_size.width - overlap, left_size.height));
	Mat remain_right_part = image_right(Rect(overlap, 0, right_size.width - overlap, right_size.height));

	remain_left_part.copyTo(new_img(Rect(0, 0, left_size.width - overlap, new_img_size.height)));
	overlap_part.copyTo(new_img(Rect(left_size.width - overlap, 0, overlap, new_img_size.height)));
	remain_right_part.copyTo(new_img(Rect(left_size.width, 0, right_size.width - overlap, new_img_size.height)));

	return new_img;
}

int number_of_pattern(int img_size, int overlap, int total_size) {
	// Find out how many number of pattern is needed
	int n = 1;
	// We can find out the answer by the inequality, number_needed * (size_of_image - overlap) + overlap >= demanding_size
	for (n = 1; n * (img_size - overlap) + overlap < total_size; n++)
		; // If the total_size is smaller than one img_size, then return 1
	return n;
}

int* get_shortest_path(double **ary, int station_size, int line_size) {
	// Use assembly line scheduling
	// ary[station][line]
	// f[station][line], l[station][line]

	vector<vector<double>> f(station_size, vector<double>(line_size,0.0));
	vector<vector<int>>  l(station_size, vector<int>(line_size, 0));

	// Initial f and l table
	for (int i = 0; i < line_size; i++) {
		f[0][i] = ary[0][i];
		l[0][i] = -1;
	}

	for (int i = 1; i < station_size; i++) {
		if ( islessequal(f[i - 1][0], f[i - 1][1]) ) {
			f[i][0] = f[i - 1][0] + ary[i][0];
			l[i][0] = 0;
		}
		else {
			f[i][0] = f[i - 1][1] + ary[i][1];
			l[i][0] = 1;
		}

		for (int j = 1; j < line_size - 1; j++) {
			if ( isless(f[i - 1][j - 1], f[i - 1][j]) &&
				 isless(f[i - 1][j - 1], f[i - 1][j + 1]) ) {
				// If the front line is shortest
				f[i][j] = f[i - 1][j - 1] + ary[i][j];
				l[i][j] = j - 1;
			}
			else if (isless(f[i - 1][j + 1], f[i - 1][j]) &&
					 isless(f[i - 1][j + 1], f[i - 1][j - 1]) ) {
				// If the later line is shortest
				f[i][j] = f[i - 1][j + 1] + ary[i][j];
				l[i][j] = j + 1;
			}
			else {
				// If the middle line is shortest or three of them have equal length
				f[i][j] = f[i - 1][j] + ary[i][j];
				l[i][j] = j;
			}
		}

		if (islessequal(f[i - 1][line_size - 1], f[i - 1][line_size - 2])) {
			f[i][line_size - 1] = f[i - 1][line_size - 1] + ary[i][line_size - 1];
			l[i][line_size - 1] = line_size - 1;
		}
		else {
			f[i][line_size - 1] = f[i - 1][line_size - 2] + ary[i][line_size - 1];
			l[i][line_size - 1] = line_size - 2;
		}
	}

	// Find the shortest path
	double minValue = f[station_size - 1][0];
	int ls = 0;
	for (int m = 1; m < line_size; m++)
		if (isgreater(minValue, f[station_size - 1][m])) {
			ls = m; // The current minimal Line
			minValue = f[station_size - 1][m]; // Update the minial value
		}

	int* shortestPath = new int[station_size - 1]; // Make a new array of shortest path
	for (int h = station_size - 1; h >= 0; h--) {
		shortestPath[h] = ls;
		ls = l[h][ls];
	}

	return shortestPath; // Return the pointer of the shortest path array
}

double **get_eu_distance(Mat & img1, Mat & img2, int station, int line, bool is_hor) {
	// Caculate the Eu-distance of all pixel which is inside the overlaping area
	double img1_B, img1_G, img1_R, img2_B, img2_G, img2_R, pow1, pow2, pow3, sum;

	// Create a empty 2D array with pointer
	double** ary = new double*[station];
	for (int m = 0; m < station; m++)
		ary[m] = new double[line];

	// The array must be in form of ary[station][line]
	if (is_hor) {
		// For horizontal, due to height is station and width is line, so we make arry[height][width]
		for (int i = 0; i < station; i++)
			for (int j = 0; j < line; j++) {
				img1_B = (double)img1.at<Vec3b>(i, j)[0];
				img1_G = (double)img1.at<Vec3b>(i, j)[1];
				img1_R = (double)img1.at<Vec3b>(i, j)[2];

				img2_B = (double)img2.at<Vec3b>(i, j)[0];
				img2_G = (double)img2.at<Vec3b>(i, j)[1];
				img2_R = (double)img2.at<Vec3b>(i, j)[2];

				pow1 = pow(img1_B - img2_B, 2.0);
				pow2 = pow(img1_G - img2_G, 2.0);
				pow3 = pow(img1_R - img2_R, 2.0);
				sum = pow1 + pow2 + pow3;
				ary[i][j] = sqrt(sum);
			}
	}
	else {
		// For vertical, due to height is line and width is station, so we make arry[width][height]
		for (int i = 0; i < station; i++)
			for (int j = 0; j < line; j++) {
				img1_B = (double)img1.at<Vec3b>(j, i)[0];
				img1_G = (double)img1.at<Vec3b>(j, i)[1];
				img1_R = (double)img1.at<Vec3b>(j, i)[2];

				img2_B = (double)img2.at<Vec3b>(j, i)[0];
				img2_G = (double)img2.at<Vec3b>(j, i)[1];
				img2_R = (double)img2.at<Vec3b>(j, i)[2];

				pow1 = pow(img1_B - img2_B, 2.0);
				pow2 = pow(img1_G - img2_G, 2.0);
				pow3 = pow(img1_R - img2_R, 2.0);
				sum = pow1 + pow2 + pow3;
				ary[i][j] = sqrt(sum);
			}
	}

	return ary;
}