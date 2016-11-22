#include "Haar.h"
#include "stdio.h"
#include "Util.h"
#include "stdlib.h"
#include "OpenCV.h"
int main(int argc, char** argv)
{
	if (argc != 3) {
		printf("Usage: ./autoDetect videopath savepath!\n");
	}
	Image img;
	Image img_1;
	Image img_2;

	img.imgPtr = (ImgPtr)&ImgRGBPool8[0];

	img_1.imgPtr = (ImgPtr)&ImgRGBPool8[1];
	img_2.imgPtr = (ImgPtr)&ImgRGBPool8[2];

	ReadFaceCascade();
	//char* classifypath;
	//classifypath = "\\cjj0917.xml";
	//cascade = (HaarClassifierCascade*)cvLoad(classifypath, 0, 0, 0);


	//char *imgpath = "D:\\lane\\8.8\\1.avi";         
	char *imgpath = argv[1];         
	CvCapture *capture = cvCaptureFromAVI(imgpath);
	int totalframes = (int)cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT); 
	int beishu=1;
	char* src_start;
	//for (int i = 5 ; i <= 100; i++)
	uchar data[360960];
	while (totalframes--)  
	{
		cvGrabFrame(capture);
		IplImage* imgframe = cvRetrieveFrame(capture, 0);
		IplImage* src = cvCreateImage(cvSize(imgframe->width,imgframe->height), 8, 1);

		cvCvtColor(imgframe,src,CV_BGR2GRAY);

		//char out_path[256];
		//sprintf(out_path, "..\\raw\\%d.raw",totalframes);
		//memcpy(data, src->imageData, /*3**/(src->width*src->height)*sizeof(unsigned char));
		//WriteRawData(data,out_path);

		Size minSize;
		minSize.height = 28;
		minSize.width =  28;

		
		//************************第一次
		if(1)
		{
			beishu=4;

			int height_4=400;
			int width_4=700;
			int tl_4x=50;
			int tl_4y=40;

			Get_roi(&img,src->imageData,tl_4x,tl_4y,width_4,height_4);

					//CvRect roi_4=cvRect(tl_4x,tl_4y,img.cols,img.rows);
			//cvSetImageROI(src,roi_4);
			//IplImage *src_roi4=cvCreateImage(cvSize(img.cols,img.rows),8,1);
			//cvCopy(src,src_roi4,0);
			////src(roi);       
			//memcpy(src_roi4->imageData, img.imgPtr, img.rows*img.cols*sizeof(unsigned char));
		
			DownSample(&img, beishu);
			HaarDetectObjects(&img, cascade,   NULL, 38, 2, 0, minSize,tl_4x,tl_4y,beishu);//8.22

			//if(result_seq.total != 0)
			//{
			//	char path[100];
			//	for(int k=0;k<result_seq.total;k++)
			//	{
			//		Rect rect = result_seq.rectQueue[k];
			//		//cvRectangle(imgframe, cvPoint(beishu*rect.x+tl_4x,beishu* rect.y+tl_4y), cvPoint(beishu*rect.x + beishu*rect.width+tl_4x, beishu*rect.y + beishu*rect.height+tl_4y), cvScalar(0, 0, 255,0), 2,2,0);
			//	}
			//}
		}
		//************************第一次

		//************************第二次
		if(1)
		{
			beishu=2;

			int height_2 =140;
			int width_2 =420;
			int tl_2x=140;
			int tl_2y=170;

		
			//CvRect roi_2=cvRect(tl_2x,tl_2y,img_1.cols,img_1.rows);
			//cvSetImageROI(src,roi_2);
			//IplImage *src_roi2=cvCreateImage(cvSize(img_1.cols,img_1.rows),8,1);
			//cvCopy(src,src_roi2,0);
			//memcpy(img_1.imgPtr, src_roi2->imageData, img_1.rows*img_1.cols*sizeof(unsigned char));

			Get_roi(&img_1,src->imageData,tl_2x,tl_2y,width_2,height_2);
			DownSample(&img_1, beishu);
			HaarDetectObjects(&img_1, cascade,   NULL, 38, 2, 0, minSize,tl_2x,tl_2y,beishu);//8.22
		
			//tl_2x=0;
			//tl_2y=0;
			//    cvRectangle(imgframe, cvPoint(175,180), cvPoint(175+img_1.cols*beishu, 180+img_1.   rows*beishu), cvScalar(255, 255, 0,0), 2,2,0);

			//if(result_seq.total != 0)
			//{
			//	char path[100];
			//	for(int k=0;k<result_seq.total;k++)
			//	{
			//		Rect rect = result_seq.rectQueue[k];
			//		//cvRectangle(imgframe, cvPoint(beishu*rect.x+tl_2x,beishu* rect.y+tl_2y), cvPoint(beishu*rect.x + beishu*rect.width+tl_2x, beishu*rect.y + beishu*rect.height+tl_2y), cvScalar(0, 255, 0,0), 2,2,0);
			//	}
			//}
		}
		//cvRectangle(imgframe, cvPoint(tl_2x,tl_2y), cvPoint(beishu*img_1.cols+tl_2x, beishu*img_1.rows+tl_2y), cvScalar(0, 255, 255,0), 2,2,0);
		//************************第二次

		///************************第三次b
		if(1)
		{beishu=1;
		int height_1 =80;            
		int width_1 =240;
		int tl_1x=260;
		int tl_1y=150;  
 

		//CvRect roi_1=cvRect(tl_1x,tl_1y,img_2.cols,img_2.rows);  

		//cvSetImageROI(src,roi_1);

		//IplImage *src_roi=cvCreateImage(cvSize(img_2.cols,img_2.rows),8,1);
		//cvCopy(src,src_roi,0);
		//memcpy(img_2.imgPtr, src_roi->imageData, img_2.rows*img_2.cols*sizeof(unsigned char));

		Get_roi(&img_2,src->imageData,tl_1x,tl_1y,width_1,height_1);
		HaarDetectObjects(&img_2, cascade, NULL, 38, 2, 0, minSize,tl_1x,tl_1y,beishu);

		}
		///************************第三次

		//char out_path1[256];
		//sprintf(out_path1, "..\\raw_kuang\\%d.bmp",totalframes);
		//cvSaveImage(out_path1,imgframe,0);
	

		if(result_seq.total != 0)
		{
			//char path[100];
			//for(int k=0;k<result_seq.total;k++)
			//{
 			//	Rect rect = result_seq.rectQueue[k];
			//	cvRectangle(imgframe, cvPoint(rect.x,rect.y), cvPoint(rect.x+rect.width,rect.y+rect.height), cvScalar(255, 0, 0,0), 2,2,0);
			//}
			for (int i=0; i<result_seq.total; i++)
			{
				CvRect rect;
				rect.x = result_seq.rectQueue[i].x;
				rect.y = result_seq.rectQueue[i].y;
				rect.width = result_seq.rectQueue[i].width;
				rect.height = result_seq.rectQueue[i].height;
				int length = 50;
				//int xx = rand()%(src->width-length);
				//int yy = rand()%(src->height-length);
				IplImage* dst = cvCreateImage(cvSize(length, length), 8, 1);
				char tmpstr[1024];
				//char tmpstr2[1024];
				sprintf(tmpstr, "%s_%d_%d.bmp", argv[2], totalframes, i);
				//sprintf(tmpstr2, "D:\\save\\neg\\%d_%d.bmp", totalframes, i);

				cvSetImageROI(src, rect);
				cvResize(src, dst, 1);
				cvSaveImage(tmpstr, dst, 0);
				cvResetImageROI(src);

				//cvReleaseImage(&dst);
				//cvSetImageROI(src, cvRect(xx, yy, length, length));
				//cvSaveImage(tmpstr2, src, 0);
				//cvResetImageROI(src);
			}
		}

		CLR_RESULT_QUEUE();
	}
	return 0;
}
