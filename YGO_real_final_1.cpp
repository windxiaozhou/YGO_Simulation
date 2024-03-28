#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <thread>
using namespace cv;
using namespace std;
std::mutex mtx;
Mat imgOriginal1;
int cards_count = 5;//总卡组类型数
int offset = 12;//位置偏移，目前用于下场地，偏移12位。
Mat backWarp;
VideoCapture cap(0);

int x_change, y_change = 0;

Mat imgOriginal, imgGray, imgBlur, imgCanny, imgThre, imgDil,imgOri;
Mat imgCrop_1, imgCrop_2, imgCrop_3, imgCrop_4, imgCrop_5, imgCrop_6, imgCrop_7, imgCrop_8, imgCrop_9, imgCrop_10;
Mat imgWarp_1, imgWarp_2, imgWarp_3, imgWarp_4, imgWarp_5, imgWarp_6, imgWarp_7, imgWarp_8, imgWarp_9, imgWarp_10;
Mat mask;
vector<Point> initialPoints_1, initialPoints_2, initialPoints_3, initialPoints_4, initialPoints_5,initialPoints_6,initialPoints_7,initialPoints_8,initialPoints_9,initialPoints_10;
vector<int> pos_state(10);
vector<int> count_times(10);
vector<string> pos_card(10);
vector<Point> docPoints_1, docPoints_2, docPoints_3, docPoints_4, docPoints_5,docPoints_6, docPoints_7, docPoints_8, docPoints_9, docPoints_10;
vector<string> card_name;
string mess_a = "curl -X POST https://rest-hz.goeasy.io/v2/pubsub/publish \-H \"Content-Type:application/json\" \-d\"{'appkey':'BC-af75f53b36a8449498ab18fc268d4d41','channel':'test_channel','content':'";
string mess_c = "'}\"";

Rect card_crop(50, 108, 300, 300);//按坐标与大小裁剪区域1
float w = 400, h = 580;//游戏王卡牌标准大小


void mess_send(string mess) {

	system((mess_a + mess + mess_c).c_str());

}

vector<string> read_card(String address)
{
	ifstream readFile;
	readFile.open(address, ios::in);
	vector<string> cardName(cards_count);
	int i = 0;
	//int line = 0;
	if (readFile.is_open())
	{
		cout << "Sucess open!" << endl;
		string str;

		while (getline(readFile, str))
		{
			cout << str << endl;
			cardName[i] = str;
			i += 1;
		}
	}
	else
	{
		cout << "Open Failure!" << endl;
	}

	readFile.close();
	return cardName;
}

int aHash(Mat matSrc1, Mat matSrc2)//返回的值越小越相似
{
	Mat matDst1, matDst2;
	cv::resize(matSrc1, matDst1, cv::Size(8, 8), 0, 0, cv::INTER_CUBIC);
	cv::resize(matSrc2, matDst2, cv::Size(8, 8), 0, 0, cv::INTER_CUBIC);

	cv::cvtColor(matDst1, matDst1, COLOR_BGR2GRAY);


	cv::cvtColor(matDst2, matDst2, COLOR_BGR2GRAY);

	int iAvg1 = 0, iAvg2 = 0;
	int arr1[64], arr2[64];

	for (int i = 0; i < 8; i++)
	{
		uchar* data1 = matDst1.ptr<uchar>(i);
		uchar* data2 = matDst2.ptr<uchar>(i);

		int tmp = i * 8;

		for (int j = 0; j < 8; j++)
		{
			int tmp1 = tmp + j;

			arr1[tmp1] = data1[j] / 4 * 4;
			arr2[tmp1] = data2[j] / 4 * 4;

			iAvg1 += arr1[tmp1];
			iAvg2 += arr2[tmp1];
		}
	}

	iAvg1 /= 64;
	iAvg2 /= 64;

	for (int i = 0; i < 64; i++)
	{
		arr1[i] = (arr1[i] >= iAvg1) ? 1 : 0;
		arr2[i] = (arr2[i] >= iAvg2) ? 1 : 0;
	}
	int iDiffNum = 0;

	for (int i = 0; i < 64; i++)
	{
		if (arr1[i] != arr2[i])
		{
			++iDiffNum;
		}

	}
	return iDiffNum;
}


Mat preProcessing(Mat img)
{
	cvtColor(img, imgGray, COLOR_BGR2GRAY);
	GaussianBlur(imgGray, imgBlur, Size(3, 3), 3, 0);
	Canny(imgBlur, imgCanny, 25, 75);
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(imgCanny, imgDil, kernel);
	//erode(imgDil, imgErode, kernel);
	return imgDil;
}

vector<Point> getContours(Mat image) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	findContours(image, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	vector<vector<Point>> conPoly(contours.size());
	vector<Rect> boundRect(contours.size());

	vector<Point> biggest;
	int maxArea = 0;

	for (int i = 0; i < contours.size(); i++)
	{
		int area = contourArea(contours[i]);
		string objectType;

		if (area > 1000)
		{
			float peri = arcLength(contours[i], true);
			approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);

			if (area > maxArea && conPoly[i].size() == 4) {

				biggest = { conPoly[i][0],conPoly[i][1] ,conPoly[i][2] ,conPoly[i][3] };
				maxArea = area;
			}
		}
	}
	return biggest;
}


vector<Point> reorder(vector<Point> points)
{
	vector<Point> newPoints;
	vector<int>  sumPoints, subPoints;

	for (int i = 0; i < 4; i++)
	{
		sumPoints.push_back(points[i].x + points[i].y);
		subPoints.push_back(points[i].x - points[i].y);
	}

	newPoints.push_back(points[min_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); // 0
	newPoints.push_back(points[max_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); //1
	newPoints.push_back(points[min_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); //2
	newPoints.push_back(points[max_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); //3

	return newPoints;
}

Mat getWarp(Mat img, vector<Point> points, float w, float h, Mat imgWarp)
{
	Point2f src[4] = { points[0],points[1],points[2],points[3] };
	Point2f dst[4] = { {0.0f,0.0f},{w,0.0f},{0.0f,h},{w,h} };

	Mat matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, imgWarp, matrix, Point(w, h));

	return imgWarp;
}

void card_show(Mat card_scan,int pos,int atk)//扫描的图片，x坐标，y坐标，当前在哪个位置扫描的，以及卡片状态
{
	string mess = "";
	int k1 = 64;
	String closet = "";
	for (int i = 0; i < card_name.size(); i++)
	{
		
		
		Mat temp = imread("pic/" + card_name[i].erase(8, 1) + ".jpg");
		resize(temp, temp, Size(400, 580));
		resize(card_scan, card_scan, Size(400, 580));
		temp(card_crop).copyTo(temp);
		Mat temp2 = card_scan(card_crop);
		int k = aHash(temp, temp2);
		//imshow("1",temp2);
		if (k <= 15) {
			if (k <= k1) {
				k1 = k;
				closet = card_name[i].erase(8, 1);
				
			}
		
		}
		
		
	}
	if (k1 != 64) {
		if (pos_state[pos] == 1) {

			if (pos_card[pos] != closet) {
				pos_card[pos] = closet;
				if (offset != 12) {
					mess = "0"+to_string(pos + offset) + "1" + to_string(atk) + closet;
					mess_send(mess);
					//thread t(mess_send, mess);
					
					cout << mess << endl;
					//t.join();
					
				}
				else {
					mess = to_string(pos + offset) + "1" + to_string(atk) + closet;
					//thread t(mess_send,mess);
					
					mess_send(mess);
					cout << mess << endl;
					//t.join();
				}
		

			}

		}
		else if (pos_state[pos] == 0) {
			pos_state[pos] = 1;
			pos_card[pos] = closet;
			if (offset != 12) {
				mess = "0" + to_string(pos + offset) + "1" + to_string(atk) + closet;
				mess_send(mess);
				//thread t(mess_send, mess);
				
				cout << mess << endl;
				//t.join();
			}
			else {
				mess = to_string(pos + offset) + "1" + to_string(atk) + closet;
				mess_send(mess);
				//thread t(mess_send, mess);
				
				
				cout << mess << endl;
				//t.join();
			}
	
	}
	

	}
}


static void fun()
{

	//VideoCapture cap(0);	
	cap.read(imgOriginal1);
	int crop_x = imgOriginal1.cols / 5;
	int crop_y = imgOriginal1.rows / 2;
	while (1)
	{
		mtx.lock();
		cap.read(imgOriginal1);
		//Mat image = imgOriginal1;
		line(imgOriginal1, Point(0, crop_y), Point(imgOriginal1.cols, crop_y), Scalar(0, 0, 255), 2);
		line(imgOriginal1, Point(crop_x, 0), Point(crop_x, imgOriginal1.rows), Scalar(0, 0, 255), 2);
		line(imgOriginal1, Point(crop_x * 2, 0), Point(crop_x * 2, imgOriginal1.rows), Scalar(0, 0, 255), 2);
		line(imgOriginal1, Point(crop_x * 3, 0), Point(crop_x * 3, imgOriginal1.rows), Scalar(0, 0, 255), 2);
		line(imgOriginal1, Point(crop_x * 4, 0), Point(crop_x * 4, imgOriginal1.rows), Scalar(0, 0, 255), 2);
		//startWindowThread();
		namedWindow("YGO");
		imshow("YGO", imgOriginal1);
		waitKey(1);		
		mtx.unlock();

	}

}




int main() {

	
	card_name = read_card("decks.txt");//读取txt文件中的卡组
	for (int i = 0; i < pos_state.size();i++) {//初始化每个位置的状态
		pos_state[i] = 0;
	
	}
	for (int i = 0; i < pos_card.size(); i++) {//初始化每个位置的卡片
		pos_card[i] = "";

	}
	for (int i = 0; i < count_times.size(); i++) {//初始化每个位置的卡片
		count_times[i] = 0;

	}


	//VideoCapture cap(0);
	cap.read(imgOriginal);
	

	int crop_x = imgOriginal.cols / 5;
	int crop_y = imgOriginal.rows / 2;
	Rect camera_crop_1(0, 0 , crop_x, crop_y);//按坐标与大小裁剪区域1
	Rect camera_crop_2(crop_x, 0, crop_x, crop_y );//按坐标与大小裁剪区域2
	Rect camera_crop_3(crop_x * 2, 0, crop_x, crop_y );//按坐标与大小裁剪区域3
	Rect camera_crop_4(crop_x * 3, 0, crop_x, crop_y );//按坐标与大小裁剪区域4
	Rect camera_crop_5(crop_x * 4, 0, crop_x, crop_y );//按坐标与大小裁剪区域5
	Rect camera_crop_6(0, crop_y, crop_x, crop_y);//按坐标与大小裁剪区域1
	Rect camera_crop_7(crop_x, crop_y, crop_x, crop_y);//按坐标与大小裁剪区域2
	Rect camera_crop_8(crop_x * 2, crop_y, crop_x,  crop_y);//按坐标与大小裁剪区域3
	Rect camera_crop_9(crop_x * 3, crop_y, crop_x, crop_y);//按坐标与大小裁剪区域4
	Rect camera_crop_10(crop_x * 4, crop_y, crop_x,  crop_y);//按坐标与大小裁剪区域5
	//resize(imgOriginal, imgOriginal,Size(1280,720))
	thread t1(fun);
	t1.detach();	
	//fun();

	while (true) {
		//VideoCapture cap(0);
		cap.read(imgOriginal);
		//imshow("1",imgOriginal);

		// Preprpcessing - Step 1 
		imgThre = preProcessing(imgOriginal);
		//imshow("2",imgOriginal);
		//imshow("1",imgThre);
		imgCrop_1 = imgThre(camera_crop_1);
		imgCrop_2 = imgThre(camera_crop_2);
		imgCrop_3 = imgThre(camera_crop_3);
		imgCrop_4 = imgThre(camera_crop_4);
		imgCrop_5 = imgThre(camera_crop_5);
		imgCrop_6 = imgThre(camera_crop_6);
		imgCrop_7 = imgThre(camera_crop_7);
		imgCrop_8 = imgThre(camera_crop_8);
		imgCrop_9 = imgThre(camera_crop_9);
		imgCrop_10 = imgThre(camera_crop_10);
		//imshow("imgCrop2", imgCrop_2);

		// Get Contours - Biggest  - Step 2
		initialPoints_1 = getContours(imgCrop_1);
		initialPoints_2 = getContours(imgCrop_2);
		initialPoints_3 = getContours(imgCrop_3);
		initialPoints_4 = getContours(imgCrop_4);
		initialPoints_5 = getContours(imgCrop_5);
		initialPoints_6 = getContours(imgCrop_6);
		initialPoints_7 = getContours(imgCrop_7);
		initialPoints_8 = getContours(imgCrop_8);
		initialPoints_9 = getContours(imgCrop_9);
		initialPoints_10 = getContours(imgCrop_10);

		if (initialPoints_1.size() == 4)//检测到了目标
		{

			int atk = 0;
			
			docPoints_1 = reorder(initialPoints_1);
			float length_h = docPoints_1[0].x - docPoints_1[3].x;
			float length_w = docPoints_1[0].y - docPoints_1[3].y;
			if (length_h < 0) { length_h = -length_h; }
			if (length_w < 0) { length_w = -length_w; }
			if (length_h < length_w) 
				{
				imgWarp_1 = getWarp(imgOriginal, docPoints_1, w, h, imgWarp_1);
				atk = 1;
				}
			// Warp - Step 3 
			else 
				{
				imgWarp_1 = getWarp(imgOriginal, docPoints_1, h, w, imgWarp_1);
				transpose(imgWarp_1, imgWarp_1);
				flip(imgWarp_1, imgWarp_1, 1);
				atk = 0;
				}

			
			
			card_show(imgWarp_1, 0, atk);
			
		}
		else if (initialPoints_1.size() != 4)
		{
			if (pos_state[0] != 0) {
				

				if (count_times[0] > 10) {
					pos_state[0] = 0;
					if (offset==12) {
					String mess = "120000000000";//检测到之前的图片消失，传送代码
					system((mess_a + mess + mess_c).c_str());
					cout << mess << endl;
					count_times[0] = 0;
					
					}
					else {
						String mess = "000000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[0] = 0;
					
					}
				
				}
				else { count_times[0] += 1; }			
			}
		}

		if (initialPoints_2.size() == 4)//检测第二块区域
		{

			int atk = 0;
			docPoints_2 = reorder(initialPoints_2);
			for (int i = 0; i < docPoints_2.size(); i++) //重新计算对应的坐标
			{
				docPoints_2[i].x += crop_x;
			}
			float length_h = docPoints_2[0].x - docPoints_2[3].x;
			float length_w = docPoints_2[0].y - docPoints_2[3].y;
			if (length_h < 0) { length_h = -length_h; }
			if (length_w < 0) { length_w = -length_w; }
			if (length_h < length_w)
			{
				imgWarp_2 = getWarp(imgOriginal, docPoints_2, w, h, imgWarp_2);
				atk = 1;
			}
			// Warp - Step 3 
			else
			{
				imgWarp_2 = getWarp(imgOriginal, docPoints_2, h, w, imgWarp_2);
				transpose(imgWarp_2, imgWarp_2);
				flip(imgWarp_2, imgWarp_2, 1);
				atk = 0;

			}
			card_show(imgWarp_2,1,atk);
		
		}
		else if (initialPoints_2.size() != 4)
		{
			if (pos_state[1] != 0) {


				if (count_times[1] > 10) {
					pos_state[1] = 0;
					if (offset == 12) {
						String mess = "130000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[1] = 0;

					}
					else {
						String mess = "010000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[1] = 0;
					
					}				

				}
				else { count_times[1] += 1; }

			}
		}

		if (initialPoints_3.size() == 4)//检测第三块区域
		{

			int atk = 0;
			docPoints_3 = reorder(initialPoints_3);
			for (int i = 0; i < docPoints_3.size(); i++) //重新计算对应的坐标
			{
				docPoints_3[i].x += crop_x*2;
			}
			float length_h = docPoints_3[0].x - docPoints_3[3].x;
			float length_w = docPoints_3[0].y - docPoints_3[3].y;
			if (length_h < 0) { length_h = -length_h; }
			if (length_w < 0) { length_w = -length_w; }
			if (length_h < length_w)
			{
				imgWarp_3 = getWarp(imgOriginal, docPoints_3, w, h, imgWarp_3);
				atk = 1;
			}
			// Warp - Step 3 
			else
			{
				imgWarp_3 = getWarp(imgOriginal, docPoints_3, h, w, imgWarp_3);
				transpose(imgWarp_3, imgWarp_3);
				flip(imgWarp_3, imgWarp_3, 1);
				atk = 0;

			}
			card_show(imgWarp_3,2,atk);
		
		}
		else if (initialPoints_3.size() != 4)
		{
			if (pos_state[2] != 0) {


				if (count_times[2] > 10) {
					pos_state[2] = 0;
					if (offset == 12) {
						String mess = "140000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[2] = 0;

					}
					else {
						String mess = "020000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[2] = 0;

					}

				}
				else { count_times[2] += 1; }

			}
		}


		if (initialPoints_4.size() == 4)//检测第四块区域
		{

			int atk = 0;
			docPoints_4 = reorder(initialPoints_4);
			for (int i = 0; i < docPoints_4.size(); i++) //重新计算对应的坐标
			{
				docPoints_4[i].x += crop_x * 3;
			}
			float length_h = docPoints_4[0].x - docPoints_4[3].x;
			float length_w = docPoints_4[0].y - docPoints_4[3].y;
			if (length_h < 0) { length_h = -length_h; }
			if (length_w < 0) { length_w = -length_w; }
			if (length_h < length_w)
			{
				atk = 1;
				imgWarp_4 = getWarp(imgOriginal, docPoints_4, w, h, imgWarp_4);
			}
			// Warp - Step 3 
			else
			{
				imgWarp_4 = getWarp(imgOriginal, docPoints_4, h, w, imgWarp_4);
				transpose(imgWarp_4, imgWarp_4);
				flip(imgWarp_4, imgWarp_4, 1);
				atk = 0;

			}
			card_show(imgWarp_4,3,atk);
		}
		else if (initialPoints_4.size() != 4)
		{
			if (pos_state[3] != 0) {


				if (count_times[3] > 10) {
					pos_state[3] = 0;
					if (offset == 12) {
						String mess = "150000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[3] = 0;

					}
					else {
						String mess = "030000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[3] = 0;

					}


				}
				else { count_times[3] += 1; }

			}
		}


		if (initialPoints_5.size() == 4)//检测第五块区域
		{

			int atk = 0;
			docPoints_5 = reorder(initialPoints_5);
			for (int i = 0; i < docPoints_5.size(); i++) //重新计算对应的坐标
			{
				docPoints_5[i].x += crop_x * 4;
			}
			float length_h = docPoints_5[0].x - docPoints_5[3].x;
			float length_w = docPoints_5[0].y - docPoints_5[3].y;
			if (length_h < 0) { length_h = -length_h; }
			if (length_w < 0) { length_w = -length_w; }
			if (length_h < length_w)
			{
				imgWarp_5 = getWarp(imgOriginal, docPoints_5, w, h, imgWarp_5);
				atk = 1;
			}
			// Warp - Step 3 
			else
			{
				imgWarp_5 = getWarp(imgOriginal, docPoints_5, h, w, imgWarp_5);
				transpose(imgWarp_5, imgWarp_5);
				flip(imgWarp_5, imgWarp_5, 1);
				atk = 0;

			}
			card_show(imgWarp_5,4,atk);
	
		}
		else if (initialPoints_5.size() != 4)
		{
			if (pos_state[4] != 0) {


				if (count_times[4] > 10) {
					pos_state[4] = 0;
					if (offset == 12) {
						String mess = "160000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());

						cout << mess << endl;
						count_times[4] = 0;
					
					}
					else {
						String mess = "040000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());

						cout << mess << endl;
						count_times[4] = 0;
					
					}
				


				}
				else { count_times[4] += 1; }

			}
		}


		if (initialPoints_6.size() == 4)//检测到了目标
		{

			int atk = 0;

			docPoints_6 = reorder(initialPoints_6);
			for (int i = 0; i < docPoints_6.size(); i++) //重新计算对应的坐标
			{
				docPoints_6[i].y += crop_y;
			}
			float length_h = docPoints_6[0].x - docPoints_6[3].x;
			float length_w = docPoints_6[0].y - docPoints_6[3].y;
			if (length_h < 0) { length_h = -length_h; }
			if (length_w < 0) { length_w = -length_w; }
			if (length_h < length_w)
			{
				imgWarp_6 = getWarp(imgOriginal, docPoints_6, w, h, imgWarp_6);
				atk = 1;
			}
			// Warp - Step 3 
			else
			{
				imgWarp_6 = getWarp(imgOriginal, docPoints_6, h, w, imgWarp_6);
				transpose(imgWarp_6, imgWarp_6);
				flip(imgWarp_6, imgWarp_6, 1);
				atk = 0;
			}

			card_show(imgWarp_6, 5, atk);
		}
		else if (initialPoints_6.size() != 4)
		{
			if (pos_state[5] != 0) {


				if (count_times[5] > 10) {
					pos_state[5] = 0;
					if (offset == 12) {
						String mess = "170000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[5] = 0;
					
					}
					else {
						String mess = "050000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[5] = 0;
					
					}
					


				}
				else { count_times[5] += 1; }

			}
		}

		if (initialPoints_7.size() == 4)//检测到了目标
		{

			int atk = 0;

			docPoints_7 = reorder(initialPoints_7);
			for (int i = 0; i < docPoints_7.size(); i++) //重新计算对应的坐标
			{
				docPoints_7[i].y += crop_y;
				docPoints_7[i].x += crop_x;
			}
			float length_h = docPoints_7[0].x - docPoints_7[3].x;
			float length_w = docPoints_7[0].y - docPoints_7[3].y;
			if (length_h < 0) { length_h = -length_h; }
			if (length_w < 0) { length_w = -length_w; }
			if (length_h < length_w)
			{
				imgWarp_7 = getWarp(imgOriginal, docPoints_7, w, h, imgWarp_7);
				atk = 1;
			}

			else
			{
				imgWarp_7 = getWarp(imgOriginal, docPoints_7, h, w, imgWarp_7);
				transpose(imgWarp_7, imgWarp_7);
				flip(imgWarp_7, imgWarp_7, 1);
				atk = 0;
			}


			card_show(imgWarp_7, 6, atk);
		}
		else if (initialPoints_7.size() != 4)
		{
			if (pos_state[6] != 0) {


				if (count_times[6] > 10) {
					pos_state[6] = 0;
					if (offset == 12) {
						String mess = "180000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[6] = 0;
					}
					else{
					
						String mess = "060000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[6] = 0;
					}
					


				}
				else { count_times[6] += 1; }

			}
		}

		if (initialPoints_8.size() == 4)//检测到了目标
		{

			int atk = 0;

			docPoints_8 = reorder(initialPoints_8);
			for (int i = 0; i < docPoints_8.size(); i++) //重新计算对应的坐标
			{
				docPoints_8[i].y += crop_y;
				docPoints_8[i].x += crop_x*2;
			}
			float length_h = docPoints_8[0].x - docPoints_8[3].x;
			float length_w = docPoints_8[0].y - docPoints_8[3].y;
			if (length_h < 0) { length_h = -length_h; }
			if (length_w < 0) { length_w = -length_w; }
			if (length_h < length_w)
			{
				imgWarp_8 = getWarp(imgOriginal, docPoints_8, w, h, imgWarp_8);
				atk = 1;
			}

			else
			{
				imgWarp_8 = getWarp(imgOriginal, docPoints_8, h, w, imgWarp_8);
				transpose(imgWarp_8, imgWarp_8);
				flip(imgWarp_8, imgWarp_8, 1);
				atk = 0;
			}


			card_show(imgWarp_8, 7, atk);
		}
		else if (initialPoints_8.size() != 4)
		{
			if (pos_state[7] != 0) {


				if (count_times[7] > 10) {
					pos_state[7] = 0;
					if (offset == 12) {
						String mess = "190000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[7] = 0;

					
					}
					else {
						String mess = "070000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[7] = 0;
					}
					


				}
				else { count_times[7] += 1; }

			}
		}

		if (initialPoints_9.size() == 4)//检测到了目标
		{

			int atk = 0;

			docPoints_9 = reorder(initialPoints_9);
			for (int i = 0; i < docPoints_9.size(); i++) //重新计算对应的坐标
			{
				docPoints_9[i].y += crop_y;
				docPoints_9[i].x += crop_x*3;
			}
			float length_h = docPoints_9[0].x - docPoints_9[3].x;
			float length_w = docPoints_9[0].y - docPoints_9[3].y;
			if (length_h < 0) { length_h = -length_h; }
			if (length_w < 0) { length_w = -length_w; }
			if (length_h < length_w)
			{
				imgWarp_9 = getWarp(imgOriginal, docPoints_9, w, h, imgWarp_9);
				atk = 1;
			}

			else
			{
				imgWarp_9 = getWarp(imgOriginal, docPoints_9, h, w, imgWarp_9);
				transpose(imgWarp_9, imgWarp_9);
				flip(imgWarp_9, imgWarp_9, 1);
				atk = 0;
			}


			card_show(imgWarp_9, 8, atk);
		}
		else if (initialPoints_9.size() != 4)
		{
			if (pos_state[8] != 0) {


				if (count_times[8] > 10) {
					pos_state[8] = 0;
					if (offset == 12) {
						String mess = "200000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[8] = 0;
					
					}
					else {
						String mess = "080000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[8] = 0;
					}
					


				}
				else { count_times[8] += 1; }

			}
		}


		if (initialPoints_10.size() == 4)//检测到了目标
		{

			int atk = 0;

			docPoints_10 = reorder(initialPoints_10);
			for (int i = 0; i < docPoints_10.size(); i++) //重新计算对应的坐标
			{
				docPoints_10[i].y += crop_y;
				docPoints_10[i].x += crop_x*4;
			}
			float length_h = docPoints_10[0].x - docPoints_10[3].x;
			float length_w = docPoints_10[0].y - docPoints_10[3].y;
			if (length_h < 0) { length_h = -length_h; }
			if (length_w < 0) { length_w = -length_w; }
			if (length_h < length_w)
			{
				imgWarp_10 = getWarp(imgOriginal, docPoints_10, w, h, imgWarp_10);
				atk = 1;
			}

			else
			{
				imgWarp_10 = getWarp(imgOriginal, docPoints_10, h, w, imgWarp_10);
				transpose(imgWarp_10, imgWarp_10);
				flip(imgWarp_10, imgWarp_10, 1);
				atk = 0;
			}


			card_show(imgWarp_10, 9, atk);
		}
		else if (initialPoints_10.size() != 4)
		{
			if (pos_state[9] != 0) {


				if (count_times[9] > 10) {
					pos_state[9] = 0;
					if (offset ==12) {
						String mess = "210000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[9] = 0;
					
					}
					else {
						String mess = "090000000000";//检测到之前的图片消失，传送代码
						system((mess_a + mess + mess_c).c_str());
						cout << mess << endl;
						count_times[9] = 0;
					
					}
			


				}
				else { count_times[9] += 1; }

			}
		}


		//line(imgOriginal, Point(0, crop_y), Point(imgOriginal.cols, crop_y), Scalar(0, 0, 255), 2);
		//line(imgOriginal, Point(crop_x, 0), Point(crop_x, imgOriginal.rows), Scalar(0, 0, 255), 2);
		//line(imgOriginal, Point(crop_x * 2, 0), Point(crop_x * 2, imgOriginal.rows), Scalar(0, 0, 255), 2);
		//line(imgOriginal, Point(crop_x * 3, 0), Point(crop_x * 3, imgOriginal.rows), Scalar(0, 0, 255), 2);
		//line(imgOriginal, Point(crop_x * 4, 0), Point(crop_x * 4, imgOriginal.rows), Scalar(0, 0, 255), 2);
		
		//imshow("Image", imgOriginal);
		//waitKey(2);
		

	}
	return 0;
}

