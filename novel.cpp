// CarDetectAndTrack.cpp : �������̨Ӧ�ó������ڵ㡣
//


#include <iostream>
#include <opencv2/opencv.hpp>
#include <cv.h>
#include<highgui.h>

using namespace std;
using namespace cv;

#define VideoFile "video1.avi"
#define TrainNo 50
#define ZERO 0.000001
#define PI 3.141593
#define SLOTnum 10
#define Thre1 1
#define Thre2 1
#define Thre3 1
#define Thre4 1

struct feature{
double density;
double motion_strength;
};

vector<struct feature> feature_store;

//�������������������
bool biggerSort(vector<cv::Point> v1, vector<cv::Point> v2)
{
	return cv::contourArea(v1)>cv::contourArea(v2);
}

Mat shade_detect(Mat hsv,Mat back_hsv,Mat foreground)
{
int height=foreground.rows;
int width=foreground.cols;
int no=0;
for(int i=0;i<height;i++)
	for(int j=0;j<width;j++)
	{
	if(	(hsv.at<uchar>(i,(3*j+2))-back_hsv.at<uchar>(i,(3*j+2))<0) &&
		(1<(back_hsv.at<uchar>(i,(3*j+2))/(hsv.at<uchar>(i,(3*j+2))+1)))&&
		(back_hsv.at<uchar>(i,(3*j+2))/(hsv.at<uchar>(i,(3*j+2))+1)<=3)&&
		(hsv.at<uchar>(i,(3*j+1))-back_hsv.at<uchar>(i,(3*j+1))<0.15)&&
		(abs(hsv.at<uchar>(i,(3*j+0))-back_hsv.at<uchar>(i,(3*j+0)))<=0.7)	)
		{
			foreground.at<uchar>(i,j)=0;
			no++;
		}
	}

//cout<<no;
return foreground;

}


double den_cal(Mat forground)
{
int number=countNonZero(forground);
return double(number)/double((forground.rows*forground.cols));

}


int judge(vector<struct feature> info,double density,double stren)
{

density=double(density/SLOTnum);
stren=double(stren/SLOTnum);
double accm_density=0.0;
double accm_stren=0.0;

for_each(begin(info),end(info),[&](struct feature data)
{accm_density+=(data.density-density)*(data.density-density);});


for_each(begin(info),end(info),[&](struct feature data)
{accm_stren+=(data.motion_strength-stren)*(data.motion_strength-stren);});


double result_density=sqrt(accm_density/(info.size()-1));

double result_stren=sqrt(accm_stren/(info.size()-1));

cout<<"ǿ�ȷ���"<<result_stren<<"�ܶȷ���"<<result_density<<"ƽ���ܶ�"<<density<<"ƽ��ǿ��"<<stren<<endl;

if(density>Thre1&&stren>Thre2&&result_density>Thre3&&result_stren>Thre4)
	return 1;
else 0;

}


int main(int argc, char* argv[])
{
	//��Ƶ�����ڣ��ͷ���

	cv::VideoCapture cap(VideoFile);
	if(cap.isOpened()==false)
		return 0;

	//�������
	int i;
	feature_store.clear();
	cv::Mat frame;			//��ǰ֡
	cv::Mat background;
	cv::Mat foreground;		//ǰ��
	cv::Mat bw;				//�м��ֵ����
	cv::Mat se;				//��̬ѧ�ṹԪ��

	//�û�ϸ�˹ģ��ѵ������ͼ��
	cv::BackgroundSubtractorMOG2 mog;	
	for(i=0;i<TrainNo;++i)
	{
		cout<<"����ѵ������:"<<i<<endl;
		cap>>frame;
		//cvtColor(frame,frame,CV_RGB2GRAY);
		if(frame.empty()==true)
		{
			cout<<"��Ƶ̫֡�٣��޷�ѵ������"<<endl;
			getchar();
			return 0;
		}
		mog(frame,foreground,0.01);	
	}

	//Ŀ����ӿ����ɽṹԪ�أ��������ӶϿ���СĿ�꣩
	cv::Rect rt;
	se=cv::getStructuringElement(cv::MORPH_RECT,cv::Size(5,5));

	//ͳ��Ŀ��ֱ��ͼʱʹ�õ��ı���
	vector<cv::Mat> vecImg;
	vector<int> vecChannel;
	vector<int> vecHistSize;
	vector<float> vecRange;
	cv::Mat mask(frame.rows,frame.cols,cv::DataType<uchar>::type);
	//������ʼ��
	vecChannel.push_back(0);
	vecHistSize.push_back(32);
	vecRange.push_back(0);
	vecRange.push_back(180);

	cv::Mat hsv;		//HSV��ɫ�ռ䣬��ɫ��H�ϸ���Ŀ�꣨camshift�ǻ�����ɫֱ��ͼ���㷨��
	cv::Mat back_hsv;//����hsv�ռ�
	cv::MatND hist;		//ֱ��ͼ����
	double maxVal;		//ֱ��ͼ���ֵ��Ϊ�˱���ͶӰͼ��ʾ����Ҫ��ֱ��ͼ��һ����[0 255]������
	cv::Mat backP;		//����ͶӰͼ
	cv::Mat result;		//���ٽ��
	cv::Mat speed_angle;
	int frame_no=0;
	IplImage img_prev_grey;
	Mat grey,prev_grey;
	struct feature info;
	
	double density1=0;
	double count1=0;
	//��Ƶ��������
	while(1)
	{	
		//����Ƶ
		cap>>frame;
		//cvtColor(frame,frame,CV_RGB2GRAY);
		if(frame.empty()==true)
			break;		

		//���ɽ��ͼ
		frame.copyTo(result);
		frame.copyTo(speed_angle);
		memset(&info,0,sizeof(info));
		frame_no++;
		//���Ŀ��ǰ��
		mog(frame,foreground,0.01);
		//cout<<foreground.channels();
		//threshold(foreground, foreground, 128, 255, THRESH_BINARY_INV);
		//cv::imshow("��ϸ�˹���ǰ��",foreground);
		//cvMoveWindow("��ϸ�˹���ǰ��",400,0);
		
		///��Ҫ������Ӱ���
		
		mog.getBackgroundImage(background);
		//cout<<background.channels();
		cv::cvtColor(frame,hsv,cv::COLOR_BGR2HSV);
		cv::cvtColor(background,back_hsv,cv::COLOR_BGR2HSV);
		foreground=shade_detect(hsv,back_hsv,foreground);

		//��ǰ��������ֵ�˲�����̬ѧ���Ͳ�������ȥ��αĿ��ͽ����Ͽ���СĿ�꣨һ��������ʱ��Ͽ��ɼ���СĿ�꣩	
		cv::medianBlur(foreground,foreground,5);
		//cv::imshow("��ֵ�˲�",foreground);
		//cvMoveWindow("��ֵ�˲�",800,0);
		cv::morphologyEx(foreground,foreground,cv::MORPH_DILATE,se);//��̬ѧ����


		double density=den_cal(foreground);
		density1=density1+density;
		info.density=density;
		//����ǰ���и�����ͨ����������
		foreground.copyTo(bw);
		vector<vector<cv::Point>> contours;
		cv::findContours(bw,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_NONE);
		if(contours.size()<1)
			continue;
		//����ͨ������������
		std::sort(contours.begin(),contours.end(),biggerSort);

		//���camshift���¸���λ�ã�����camshift�㷨�ڵ�һ�����£�����Ч���ǳ��ã�
		//�����ڼ����Ƶ�У����ڷֱ���̫�͡���Ƶ����̫�Ŀ��̫��Ŀ����ɫ��������
		//�ȸ������أ����¸���Ч���ǳ��  ��ˣ���Ҫ�߸��١��߼�⣬������ٲ����ã�
		//���ü��λ���޸�
		
		vecImg.clear();
		vecImg.push_back(hsv);
		for(int k=0;k<contours.size();++k)
		{
			//��k����ͨ��������Ӿ��ο�
			if(cv::contourArea(contours[k])<cv::contourArea(contours[0])/5)
				break;
			rt=cv::boundingRect(contours[k]);				
			mask=0;
			mask(rt)=255;

			//ͳ��ֱ��ͼ
			cv::calcHist(vecImg,vecChannel,mask,hist,vecHistSize,vecRange);				
			cv::minMaxLoc(hist,0,&maxVal);
			hist=hist*255/maxVal;
			//���㷴��ͶӰͼ
			cv::calcBackProject(vecImg,vecChannel,hist,backP,vecRange,1);
			//camshift����λ��
			cv::Rect search=rt;
			cv::RotatedRect rrt=cv::CamShift(backP,search,cv::TermCriteria(cv::TermCriteria::COUNT+cv::TermCriteria::EPS,10,1));
			cv::Rect rt2=rrt.boundingRect();
			rt&=rt2;

			//���ٿ򻭵���Ƶ��
			cv::rectangle(result,rt,cv::Scalar(0,255,0),2);		


		}

		//�����ʾ
		//cv::imshow("Origin",frame);
		//cvMoveWindow("Origin",0,0);

		//cv::imshow("��������",foreground);
		//cvMoveWindow("��������",0,300);

		//cv::imshow("����ͶӰ",backP);
		//cvMoveWindow("����ͶӰ",400,300);
		
		
		
		cv::cvtColor(speed_angle,grey,COLOR_BGR2GRAY);
		grey.convertTo(grey,CV_8UC1);
		IplImage  img_grey= IplImage(grey); 
		CvSize winSize = cvSize(5,5);
		IplImage *velx = cvCreateImage( cvSize(grey.cols ,grey.rows),IPL_DEPTH_32F, 1 );
		IplImage *vely = cvCreateImage( cvSize(grey.cols ,grey.rows),IPL_DEPTH_32F, 1 );
		IplImage *abs_img= cvCreateImage( cvSize(grey.cols ,grey.rows),IPL_DEPTH_8U, 1 );
		if(frame_no!=1)
		{
		cvCalcOpticalFlowLK( &img_grey, &img_prev_grey, winSize, velx, vely );
		cvAbsDiff(&img_grey,&img_prev_grey, abs_img );
		
		CvScalar total_speed = cvSum(abs_img);
		int winsize=5;
		//cout<<total_speed.val[0];
		float ss = (float)total_speed.val[0]/(4*winsize*winsize)/255;
		//cout<<ss<<endl;
		CvScalar total_x = cvSum(velx);
		float xx = (float)total_x.val[0];

		CvScalar total_y = cvSum(vely);
		float yy = (float)total_y.val[0];

		double alpha_angle;

		if(xx<ZERO && xx>-ZERO)
			alpha_angle = PI/2;
		else
			alpha_angle = abs(atan(yy/xx));

		if(xx<0 && yy>0) alpha_angle = PI- alpha_angle ;
		if(xx<0 && yy<0) alpha_angle = PI + alpha_angle ;
		if(xx>0 && yy<0) alpha_angle = 2*PI - alpha_angle ;
		double count=sqrt(alpha_angle*alpha_angle+ss*ss);
		count1=count1+count;
		info.motion_strength=count;
		
		//cout<<count<<endl;

		}
		
		feature_store.push_back(info);
		grey.copyTo(prev_grey);
		img_prev_grey= IplImage(prev_grey); 
		
		if(frame_no%10==0)
		{
		//cout<<density1<<endl;
			//cout<<count1<<endl;
			int result=judge(feature_store,density1,count1);
		if (result==1)
			{
			Beep(500,500);//����
			}
		feature_store.clear();
		density1=0;
		count1=0;

		}
		cvReleaseImage(&velx);
		cvReleaseImage(&vely);
		cvReleaseImage(&abs_img);
		cv::imshow("����Ч��",result);
		cvMoveWindow("����Ч��",500,150);
		cvWaitKey(15);

	}

	getchar();
	return 0;
}

