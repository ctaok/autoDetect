#include "Haar.h"
#include "loadCascade.h"
#include "Util.h"
#include "stdio.h"
#include "string.h"
#include <math.h>
#include "OpenCV.h"
/*******************Global************************************/
HaarClassifierCascade *cascade ;
//HidHaarClassifierCascade hid_cascade;
//32bits cell Mat
int		MatPool32[MaxMatNum][MAXROWS][MAXCOLS];
//8bits cell 
unsigned char  MatPool8[MaxMatNum][MAXROWS][MAXCOLS];

//8bits*3 cell 
unsigned char  ImgRGBPool8[MaxMatNum][RGBCHANNEL][MAXROWS][MAXCOLS];

//unsigned char  ImgRGBPool8_2[MaxMatNum][RGBCHANNEL][MAXROWS][MAXCOLS];

//unsigned char  ImgRGBPool8[MaxMatNum][RGBCHANNEL][MAXROWS][MAXCOLS];
//64bits  cell 
__int64	 MatPool64[MaxMatNum][MAXROWS][MAXCOLS];

//��ѡ��������ڵ㲢�鼯
PTreeNode PTreeNodes[MAXPTREENODES];

char HidCascade[MAXHIDCASCADE];

//�������������������
Sequence result_seq;





//==================================================================
//��������  IsEqual
//���ߣ�    qiurenbo
//���ڣ�    2014-10-1
//���ܣ�    �ж����������Ƿ��ڽ�
//���������_r1  _r2 ��ѡ�������      
//����ֵ��  ���������ԣ��Ƿ����ڽӵľ��Σ�
//�޸ļ�¼��
//==================================================================
int IsEqual( const void* _r1, const void* _r2)
{
    const Rect* r1 = (const Rect*)_r1;
    const Rect* r2 = (const Rect*)_r2;
	int distance5x = r1->width ;//int distance = cvRound(r1->width*0.2);
	
    return r2->x*5 <= r1->x*5 + distance5x &&
		r2->x*5 >= r1->x*5 - distance5x &&
		r2->y*5 <= r1->y*5 + distance5x &&
		r2->y*5 >= r1->y*5 - distance5x &&
		r2->width*5 <= r1->width * 6 &&
		r2->width * 6 >= r1->width*5;
}

//==================================================================
//��������  ReadFaceCascade
//���ߣ�    qiurenbo
//���ڣ�    2014-10-1
//���ܣ�    ���ݺ�ѡ����������ԣ�IsEqual�������������鼯
//���������seq  ��ѡĿ����������      
//����ֵ��  ���ط���������� 
//�޸ļ�¼��
//==================================================================
int SeqPartition( const Sequence* seq )
{
	Sequence* result = 0;
    //CvMemStorage* temp_storage = 0;
    int class_idx = 0;
    

	memset(PTreeNodes, 0, MAXPTREENODES*sizeof(PTreeNode));
    
    int i, j;
   


    //������seq��Ԫ��Ϊ���ڵ��ɭ��
    for( i = 0; i < seq->total; i++ )
		PTreeNodes[i].element = (char*)&seq->rectQueue[i];

	//�������и��ڵ�
	for( i = 0; i < seq->total; i++ )
    {
        PTreeNode* node = &PTreeNodes[i];
		PTreeNode* root = node;
		//ȷ��node��Ԫ��ָ�벻Ϊ��
        if( !node->element )
            continue;
		
        //�ҵ�Ԫ�������еĸ����
        while( root->parent )
            root = root->parent;
		
        for( j = 0; j < seq->total; j++ )
        {
            PTreeNode* node2 = &PTreeNodes[j];
            
			//ȷ��1.node��Ԫ��ָ�벻Ϊ��
			//    2.�Ҳ���ͬһ��node���
			//    3.������������
			// ��������������ϲ�Ԫ��
            if( node2->element && node2 != node &&
                IsEqual( node->element, node2->element))
            {
                PTreeNode* root2 = node2;
                
               //�ҵ�Ԫ�������еĸ����
                while( root2->parent )
                    root2 = root2->parent;
				
				//�ϲ���ǰ���ǲ���һ������
                if( root2 != root )
                {
					//��С���������ȴ������
                    if( root->rank > root2->rank )
                        root2->parent = root;
					//����ȵ�ʱ��Ÿı�������
                    else
                    {
                        root->parent = root2;
                        root2->rank += root->rank == root2->rank;
                        root = root2;
                    }
                    //assert( root->parent == 0 );
					
                    // ·��ѹ�����ӽڵ�node2ֱ��ָ����ڵ�
                    while( node2->parent )
                    {
                        PTreeNode* temp = node2;
                        node2 = node2->parent;
                        temp->parent = root;
                    }
					
                    // ·��ѹ�����ӽڵ�nodeֱ��ָ����ڵ�
                    node2 = node;
                    while( node2->parent )
                    {
                        PTreeNode* temp = node2;
                        node2 = node2->parent;
                        temp->parent = root;
                    }
                }
            }
		
        }
    }


	for( i = 0; i < seq->total; i++ )
    {
        PTreeNode* node = &PTreeNodes[i];
        int idx = -1;
        
        if( node->element )
        {
            while( node->parent )
                node = node->parent;
			
			//�����м��ò����������������ȡ�������ظ�����
            if( node->rank >= 0 )
                node->rank = ~class_idx++;
            idx = ~node->rank;
        }
		
       
    }

    return class_idx;
}


//==================================================================
//��������  ReadFaceCascade
//���ߣ�    qiurenbo
//���ڣ�    2014-09-30
//���ܣ�    ��ȡCascade�ļ�
//���������void      
//����ֵ��  void  
//�޸ļ�¼��
//==================================================================
void ReadFaceCascade()
{
	int i;
	//load cascade
	cascade = (HaarClassifierCascade*)HaarClassifierCascade_face;

	//load stages
	int stage_size = StageClassifier_face[0];
	HaarStageClassifier *stages ;
	stages = (HaarStageClassifier *)(StageClassifier_face+1);
	
	//load classifier
	int classifier_size = Classifier_face[0];
	HaarClassifier *cls ;
	cls = (HaarClassifier*) (Classifier_face+1);
	
	int class_info_size = class_info[0];
	int * cls_info ;
	cls_info = (int*)(class_info+1);

	
	
	//link cascade with stages
	cascade->stage_classifier = stages;
	//link stages��classifiers
	int offset=0;
	int offset_t=(sizeof(HaarFeature)/sizeof(int));
	int offset_l=offset_t+1;
	int offset_r=offset_t+2;
	int offset_a=offset_t+3;
	int offset_total=0;
	for(i=0;i<stage_size;++i)
	{
		(stages+i)->classifier = (cls+offset);
		offset +=(stages+i)->count;
	}
	
	offset_total = 5+ (sizeof(HaarFeature)/sizeof(int));
	//link classifiers and haar_featrue;
	for(i=0;i<classifier_size;++i)
	{
		HaarClassifier *cs= cls+i;
	
		cs->haar_feature = (HaarFeature*)(cls_info+i*offset_total);
		cs->threshold = (int*)(cls_info+i*offset_total+offset_t);
		cs->left =(int*)(cls_info+i*offset_total+offset_l);
		cs->right=(int*)(cls_info+i*offset_total+offset_r);
		cs->alpha=(int*)(cls_info+i*offset_total+offset_a);
	
	}
}
//==================================================================
//��������  IntegralImage
//���ߣ�    qiurenbo
//���ڣ�    2014-09-26
//���ܣ�    �Ӿ�����л�ȡrows * cols�ľ��� 
//���������mat		����ṹ���ַ
//			rows	�����������
//			cols	�����������
//			type    ������ľ������� 
//			matIndex �Ӿ�����з���ľ������У��ֶ�ָ��..��      
//����ֵ��  void  
//�޸ļ�¼��
//==================================================================

void GetMat(void* mat, int rows, int cols, int type, int matIndex)
{
	switch(type)
	{
		case BITS8: 
				((Mat8*)mat)->rows = rows;
				((Mat8*)mat)->cols = cols;
				((Mat8*)mat)->mat8Ptr =  (Mat8Ptr)&MatPool8[matIndex];
				break;
			
		case BITS32: 
				((Mat32*)mat)->rows = rows;
				((Mat32*)mat)->cols = cols;
				((Mat32*)mat)->mat32Ptr =  (Mat32Ptr)&MatPool32[matIndex];
				break;

		case BITS64:
				((Mat64*)mat)->rows = rows;
				((Mat64*)mat)->cols = cols;
				((Mat64*)mat)->mat64Ptr =  (Mat64Ptr)&MatPool64[matIndex];
				break;
	}
}


//==================================================================
//��������  IntegralImage
//���ߣ�    qiurenbo
//���ڣ�    2014-09-26
//���ܣ�    ����Ŀ��������Ļ���ͼ
//���������src		�����Ŀ�����ھ�����ʼ
//			srcstep �������������
//			sum		����ͼ����	(W+1)*(H+1)	
//			sumstep ����ͼ��������    
//			sqsum	ƽ����ͼ���� (W+1)*(H+1)	
//			sqsumstep ƽ����ͼ��������
//			size   ����������С W*H
//          
//          
//����ֵ��  void  
//�޸ļ�¼��
//==================================================================
void IntegralImage(ImgPtr src, int srcstep,
				   Mat32Ptr sum, int sumstep,     
				   Mat64Ptr sqsum, int sqsumstep,
				   Size size)
{
	int s = 0;
	__int64 sq = 0;
	//�ƶ�ָ�뵽����ͼ����һ��,��һ��ȫΪ0
	sum += sumstep + 1;     
	sqsum += sqsumstep + 1; 
	
	//y��������������������ʼ�ڼ���
	for(int y = 0; y < size.height; y++, src += srcstep,       
		sum += sumstep, sqsum += sqsumstep )    
	{   
		//sum��sqsumΪ(W+1)*(H+1)��С���󣬹ʽ���һ����Ϊ0
		sum[-1] = 0;                                        
		sqsum[-1] = 0;                                      
		
		for(int x = 0 ; x < size.width; x++ )    
		{                                                   
			int it = src[x];                           
			int t = (it);   
			
			//������ƽ��
			__int64	tq =  CV_8TO16U_SQR(it);  
            //s�������ϵ��ۼӺ�
			s += t;  
			//sq�������ϵ��ۼӺ�
			sq += tq;                                       
			t = sum[x - sumstep] + s;                       
			tq = sqsum[x - sqsumstep] + sq;                 
			sum[x] = t;                                     
			sqsum[x] = (__int64)tq;                                  
		}                                                   
    }                        
}

//==================================================================
//��������  Integral
//���ߣ�    qiurenbo
//���ڣ�    2014-09-26
//���ܣ�    ����Ŀ��������Ļ���ͼ
//���������image ͼ��
//			sumImage ����ͼָ��
//			sumSqImage ƽ����ͼָ��                 
//����ֵ��  void  
//�޸ļ�¼��
//==================================================================
void Integral(Image* image, Mat32* sumImage, Mat64* sumSqImage)
{

	//ȡ����ַ�ռ��Ѿ����䣬��������
	if (image == NULL || sumImage == NULL || sumSqImage == NULL)
		return;

    Image*src    =		(Image*)image;
    Mat32 *sum	 =		(Mat32*)sumImage;
    Mat64 *sqsum =		(Mat64*)sumSqImage;
   
	Size size;
	size.height = src->rows;
	size.width =  src->cols;

    IntegralImage(src->imgPtr, src->cols,
		sum->mat32Ptr, sum->cols,     
		sqsum->mat64Ptr, sqsum->cols,size);
    

}
//==================================================================
//��������  AlignPtr
//���ߣ�    qiurenbo
//���ڣ�    2014-10-03
//���ܣ�	��algin�ֽڶ���
//���������ptr Ҫ�����ָ��   
//			align ������ֽ���           
//����ֵ��  void*   
//�޸ļ�¼��
//==================================================================
//void* AlignPtr( const void* ptr, int align)
//{
//
//	return (void*)( ((unsigned int)ptr + align - 1) & ~(align-1) );
//}
//==================================================================
//��������  CreateHidHaarClassifierCascade
//���ߣ�    qiurenbo
//���ڣ�    2014-09-28
//���ܣ�    ������ʽ����ͼ�ӿ�����ٶ�
//���������cascade ����������ָ��              
//����ֵ��  static HidHaarClassifierCascade*   ����һ����ʽ����������ָ��
//�޸ļ�¼��
//==================================================================
static HidHaarClassifierCascade*
CreateHidHaarClassifierCascade(HaarClassifierCascade* cascade)
{



	
	cascade->hid_cascade = (struct HidHaarClassifierCascade *)HidCascade;
	//����ջ�ռ�
    HidHaarClassifierCascade* out = (struct HidHaarClassifierCascade *)HidCascade;
	const int icv_stage_threshold_bias = 419; //0.0001*(2^22)=419.4304

	HidHaarClassifier* haar_classifier_ptr;
    HidHaarTreeNode* haar_node_ptr;
    int i, j, l;
   
    //int total_classifiers = 2135;
    int total_classifiers = Classifier_face[0];
    int total_nodes = 0;
    
  
    int has_tilted_features = 0;
    int max_count = 0;




    /* ��ʼ��HidCascadeͷ */
    out->count = cascade->count;
    out->stage_classifier = (HidHaarStageClassifier*)(out + 1);
	//out->stage_classifier = (HidHaarStageClassifier*)AlignPtr(out + 1, 4);
	//classifier��ʼ��ַ
	haar_classifier_ptr = (HidHaarClassifier*)(out->stage_classifier + cascade->count);
	//haar_classifier_ptr = (HidHaarClassifier*)AlignPtr(out->stage_classifier + cascade->count, 4);
	//node��ʼ��ַ
    //haar_node_ptr = (HidHaarTreeNode*)AlignPtr(haar_classifier_ptr + total_classifiers, 4);
	haar_node_ptr = (HidHaarTreeNode*)(haar_classifier_ptr + total_classifiers);
    out->is_stump_based = 1;
    out->is_tree = 0;

    // ��cascade��ʼ��HidCascade
    for( i = 0; i < cascade->count; i++ )
    {

		//��cascades Stage��ʼ��HidCascade��Stage
        HaarStageClassifier* stage_classifier = cascade->stage_classifier + i;
        HidHaarStageClassifier* hid_stage_classifier = out->stage_classifier + i;

        hid_stage_classifier->count = stage_classifier->count;
        hid_stage_classifier->threshold = stage_classifier->threshold - icv_stage_threshold_bias;
        //hid_stage_classifier->classifier = (struct HidHaarClassifier *)&HidClassifiers[i];
		 hid_stage_classifier->classifier = haar_classifier_ptr;
		//��ʼ��Ϊ������������������ʵ����������1��0����������
        hid_stage_classifier->two_rects = 1;
		haar_classifier_ptr += stage_classifier->count;


		//Stage����һ���˻��Ķ�����������֧����ÿ��������ֻ��һ������
        hid_stage_classifier->parent = (stage_classifier->parent == -1)
            ? NULL : out->stage_classifier + stage_classifier->parent;
        hid_stage_classifier->next = (stage_classifier->next == -1)
            ? NULL :  out->stage_classifier + stage_classifier->next;
        hid_stage_classifier->child = (stage_classifier->child == -1)
            ? NULL : out->stage_classifier + stage_classifier->child ;
        
		//�жϸ�stage�Ƿ�Ϊ��״�ṹ�����֦��
        out->is_tree |= hid_stage_classifier->next != NULL;


		//��ֵclassifer����
        for( j = 0; j < stage_classifier->count; j++ )
        {
            HaarClassifier* classifier = stage_classifier->classifier + j;
            HidHaarClassifier* hid_classifier = hid_stage_classifier->classifier + j;
            int node_count = classifier->count;
			
            int* alpha_ptr = (int*)(haar_node_ptr + node_count);

			hid_classifier->count = node_count;
		    hid_classifier->node = haar_node_ptr;
            hid_classifier->alpha = alpha_ptr;
           
			//��ֵnode����
            for( l = 0; l < node_count; l++ )
            {
                HidHaarTreeNode* node =  hid_classifier->node + l;
                HaarFeature* feature = classifier->haar_feature + l;
                memset( node, -1, sizeof(*node) );
                node->threshold = classifier->threshold[l];
                node->left = classifier->left[l];
                node->right = classifier->right[l];
				
				//��������Ŀ�����ж�,����������������two_rectsΪ0
                if( (feature->rect[2].weight) == 0 ||
                    feature->rect[2].r.width == 0 ||
                    feature->rect[2].r.height == 0 )
                    memset( &(node->feature.rect[2]), 0, sizeof(node->feature.rect[2]) );
                else
                    hid_stage_classifier->two_rects = 0;
            }

			//��ֵalpha
            memcpy( hid_classifier->alpha, classifier->alpha, (node_count+1)*sizeof(hid_classifier->alpha[0]));
			haar_node_ptr = (HidHaarTreeNode*)(alpha_ptr+node_count + 1);
                

			//�ж�cascade�еķ������Ƿ�����׮��������ֻ�и����ľ�����
            out->is_stump_based &= node_count == 1;
        }
    }


    //cascade->hid_cascade = out;
    //assert( (char*)haar_node_ptr - (char*)out <= datasize );

   



    return out;
}

//==================================================================
//��������  SetImagesForHaarClassifierCascade
//���ߣ�    qiurenbo
//���ڣ�    2014-09-29
//���ܣ�    ���ݳ߶ȵ���Haar�����Ĵ�С��Ȩ��
//���������cascade ����������ָ�� 
//			sum     ����ͼ
//			sqsum   ƽ���ͻ���ͼ
//			scale32x �߶�             
//����ֵ��  ��
//�޸ļ�¼��
//==================================================================
void SetImagesForHaarClassifierCascade(HaarClassifierCascade* _cascade, Mat32* sum, Mat64* sqsum, int scale32x)
{



  
    HidHaarClassifierCascade* hidCascade;
    int coi0 = 0, coi1 = 0;
    int i;
    Rect equ_rect;
    int weight_scale;

	//���ݳ߶Ȼ�ȡ���ڴ�С
    _cascade->scale32x = scale32x;
    _cascade->real_window_size.width = (_cascade->orig_window_size.width * scale32x + 16)>>5 ;
    _cascade->real_window_size.height = (_cascade->orig_window_size.height * scale32x +16) >> 5;


	//������ʽ�����������Ļ���ͼ
	hidCascade = _cascade->hid_cascade;
    hidCascade->sum = sum;
    hidCascade->sqsum = sqsum;

	//���ݳ߶����û���ͼ��ʼ�����λ��
	equ_rect.x = equ_rect.y = (scale32x+16)>>5;    
    equ_rect.width = ((_cascade->orig_window_size.width-2)*scale32x + 16 ) >> 5;   //+0.5��Ϊ����������
    equ_rect.height = ((_cascade->orig_window_size.height-2)*scale32x + 16 ) >> 5;
    weight_scale = equ_rect.width*equ_rect.height;
    hidCascade->window_area = weight_scale; //�������
	
	//��ȡ����ͼ����ʼ�����ĸ����ص�����
    hidCascade->p0 = sum->mat32Ptr + (equ_rect.y) * sum->cols+ equ_rect.x;
    hidCascade->p1 = sum->mat32Ptr + (equ_rect.y) * sum->cols + equ_rect.x + equ_rect.width;
    hidCascade->p2 = sum->mat32Ptr + (equ_rect.y + equ_rect.height) * sum->cols + equ_rect.x;
    hidCascade->p3 = sum->mat32Ptr + (equ_rect.y + equ_rect.height) * sum->cols + equ_rect.x + equ_rect.width;

	//��ȡƽ���ͻ���ͼ����ʼ�����ĸ����ص�����
	hidCascade->pq0 = sqsum->mat64Ptr + (equ_rect.y) * sqsum->cols+ equ_rect.x;
    hidCascade->pq1 = sqsum->mat64Ptr + (equ_rect.y) * sqsum->cols+ equ_rect.x + equ_rect.width;
    hidCascade->pq2 = sqsum->mat64Ptr + (equ_rect.y + equ_rect.height) * sqsum->cols+ equ_rect.x;
    hidCascade->pq3 = sqsum->mat64Ptr + (equ_rect.y + equ_rect.height) * sqsum->cols+ equ_rect.x + equ_rect.width;

	//����ÿ��Classifer��ʹ�õ������������ǽ��г߶ȷŴ󣬲����ı��ֵ����HidCascade����ʽ����������
	for( i = 0; i < hidCascade->count; i++ )
    {
        int j, k,l;
        for( j = 0; j < hidCascade->stage_classifier[i].count; j++ )
        {
            for( l = 0; l < hidCascade->stage_classifier[i].classifier[j].count; l++ )
            {
                HaarFeature* feature = &_cascade->stage_classifier[i].classifier[j].haar_feature[0];
				
                HidHaarFeature* hidfeature = &hidCascade->stage_classifier[i].classifier[j].node[0].feature;
				int sum0 = 0, area0 = 0;
                Rect r[3];
				
				
				Rect tr;
                int correction_ratio;
				for( k = 0; k < CV_HAAR_FEATURE_MAX; k++ )
                {
                    if( !hidfeature->rect[k].p0 )
                        break;
					
                    r[k] = feature->rect[k].r;
					
					
					
					//���Ͻ�����;��󳤿����߶ȷŴ�
					tr.x = (r[k].x * scale32x + 16) >> 5;
                    tr.width = (r[k].width * scale32x + 16) >> 5;
                    tr.y = ( r[k].y * scale32x + 16 ) >> 5;
                    tr.height = ( r[k].height * scale32x +16 ) >> 5;
					
					
                    correction_ratio = weight_scale;
					
					//���þ����ĸ������ڻ���ͼ�е�λ�ã�Ϊ�˼����������㣩
					hidfeature->rect[k].p0 = sum->mat32Ptr + tr.y * sum->cols +  tr.x;
					hidfeature->rect[k].p1 = sum->mat32Ptr + tr.y * sum->cols +  tr.x + tr.width; 
					hidfeature->rect[k].p2 = sum->mat32Ptr + (tr.y + tr.height) *sum->cols +  tr.x; 
                    hidfeature->rect[k].p3 = sum->mat32Ptr + (tr.y + tr.height) *sum->cols +  tr.x + tr.width;
					
					//rect[1] = weight/area�� ����22λ��Ϊ�˱��⸡����㣬��Ȩֵ/��ⴰ��������������󣩣�����Ȩֵ
                    hidfeature->rect[k].weight = ((feature->rect[k].weight)<< NODE_THRESHOLD_SHIFT)/(correction_ratio);
					
                    if( k == 0 )
                        area0 = tr.width * tr.height;
					else
                        sum0 += hidfeature->rect[k].weight * tr.width * tr.height;
					
                }
				//rect[0].weight ,Ȩ�غ�������������ɷ���
				hidfeature->rect[0].weight = (int)(-sum0/area0);
				
            } /* l */
        } /* j */
    }

	
};


//==================================================================
//��������  RunHaarClassifierCascade
//���ߣ�    qiurenbo
//���ڣ�    2014-09-30
//���ܣ�    ��ָ�����ڷ�Χ��������
//���������_cascade	����������ָ�� 
//			pt			��ⴰ�����Ͻ�����
//			start_stage ��ʼstage�±�   
//����ֵ��  <=0			δ��⵽Ŀ������������
//			1			�ɹ���⵽Ŀ��
//�޸ļ�¼��
//====================================================================
int RunHaarClassifierCascade( HaarClassifierCascade* _cascade, Point& pt, int start_stage )
{
                           
    int result = -1;


    int p_offset, pq_offset;
    int i, j;
    __int64 rectsum, variance_factor;
    int variance_norm_factor;
    HidHaarClassifierCascade* hidCascade;

	if (_cascade == NULL)
		return -1;
  

    hidCascade = _cascade->hid_cascade;
    if( !hidCascade )
		return -1;
	

	//ȷ�����ε���Ч�ԣ�����ֹ���㴰�ڳ��߽�
	if( pt.x < 0 || pt.y < 0 ||
        pt.x + _cascade->real_window_size.width >= hidCascade->sum->cols-2 ||
        pt.y + _cascade->real_window_size.height >= hidCascade->sum->rows-2 )
        return -1;


	//�����������ڻ���ͼ�е�ƫ�ƣ��൱���ƶ�����
    p_offset = pt.y * (hidCascade->sum->cols) + pt.x;
    pq_offset = pt.y * (hidCascade->sqsum->cols) + pt.x;


	//�����ƶ����������ڵ�����ֵ
    rectsum = calc_sum(*hidCascade,p_offset);//*cascade->inv_window_area;
    variance_factor = hidCascade->pq0[pq_offset] - hidCascade->pq1[pq_offset] -
                           hidCascade->pq2[pq_offset] + hidCascade->pq3[pq_offset];
    variance_factor = (variance_factor - (rectsum*rectsum + (hidCascade->window_area>>1))/hidCascade->window_area + (hidCascade->window_area>>1))/hidCascade->window_area;
    variance_norm_factor = int(sqrt(float(variance_factor))+0.5f);//qmath

    if( variance_norm_factor < 0 )
        variance_norm_factor = 1;
	//LARGE_INTEGER t1,t2,tc;
   	//QueryPerformanceFrequency(&tc);
	//QueryPerformanceCounter(&t1);

	//����ÿ��classifier���õ����������������ֵ
	for( i = start_stage; i < hidCascade->count; i++ )
	//for( i = start_stage; i < hidCascade->count; i++ )
	{
 		double stage_sum = 0;
		

		
		//if( hidCascade->stage_classifier[i].two_rects )
		//{
		for( j = 0; j < hidCascade->stage_classifier[i].count; j++ )
		{
		
			HidHaarClassifier* classifier = hidCascade->stage_classifier[i].classifier + j;
			HidHaarTreeNode* node = hidCascade->stage_classifier[i].classifier[j].node;
			double sum, t = node->threshold*variance_norm_factor, a, b;
			
			
			//����Haar����
			sum = calc_sum(node->feature.rect[0],p_offset) * node->feature.rect[0].weight;
			sum += calc_sum(node->feature.rect[1],p_offset) * node->feature.rect[1].weight;
			
			//���������������ֿ�����
			if( node->feature.rect[2].p0 )
				sum += calc_sum(node->feature.rect[2],p_offset) * node->feature.rect[2].weight;


			a = classifier->alpha[0];
			b = classifier->alpha[1];
			stage_sum += sum < t ? a : b;
		}
		
		if( stage_sum < hidCascade->stage_classifier[i].threshold )
		{
			return -i;
			
		}
	}
    
   //QueryPerformanceCounter(&t2);
   //printf("FeatureDetectTime:%fms\n",(t2.QuadPart - t1.QuadPart)*1000.0/tc.QuadPart);

  

    return 1;
}

//=======================================================================================
//��������  HaarDetectObjects
//���ߣ�    qiurenbo
//���ڣ�    2014-09-30
//���ܣ�    ��ָ��ͼƬ�в���Ŀ��
//��������� _img				ͼƬָ��	    
//			cascade				����������ָ�� 
//			start_stage			��ʼstage�±� 
//			scale_factor32x		���ڱ仯�߶ȱ��� /32
//			min_neighbors		��С�ٽ�Ŀ�꣨min_neighbors�����ϵĺ�ѡĿ��������������Ŀ������
//			minSize				Ŀ����С�Ĵ�С
//����ֵ��  <=0					δ��⵽Ŀ������������
//			1					�ɹ���⵽Ŀ��
//�޸ļ�¼��
//=========================================================================================
void HaarDetectObjects(Image* _img,
					HaarClassifierCascade* cascade,   //ѵ���õļ���������
					char* storage, int scale_factor32x,
                    int min_neighbors, int flags, Size minSize,int tl_x,int tl_y,int beishu)//8.22
{
	
	
	//��һ�η����õ������stage
	//�ڶ��η����õ�����ʼstage
	int split_stage = 2;

   // ImgPtr stub, *img =  _img;
	Mat32		sum ;
	Mat64	    sqsum;
	Image	    tmp;

	//��������ѡ����
	Sequence    seq;
	
	//�����ѡ����
	Sequence    seq2;
	
	//���鼯�ϲ�����
	Sequence comps;

	memset(&seq, 0, sizeof(Sequence));
	memset(&comps, 0, sizeof(Sequence));
	memset(&seq2, 0, sizeof(Sequence));
	
    int i;
    
    int factor32x;
    int npass = 2;

    if( !cascade )
       return ;

	//��ȡ����ͼ��ƽ���ͻ���ͼ�ľ���
	GetMat(&sum , _img->rows + 1, _img->cols + 1, BITS32, 0);
	GetMat(&sqsum, _img->rows + 1, _img->cols + 1, BITS64, 0);
	GetMat(&tmp, _img->rows, _img->cols, BITS8, 1);

	//����������ʽ����ͼ�����ڼ��ټ��㣩���򴴽�һ��
	 if( !cascade->hid_cascade )
		CreateHidHaarClassifierCascade(cascade);

	//����ʽ����������д���ļ����Ƿ���ȷ
	//WriteCascadeToFile(cascade->hid_cascade);
	//LARGE_INTEGER t1,t2,tc;
	//QueryPerformanceFrequency(&tc);
	//QueryPerformanceCounter(&t1);
    //�������ͼ
    Integral(_img, &sum, &sqsum);
	//QueryPerformanceCounter(&t2);
	//printf("SetImageFeatureRunTime:%fms\n",(t2.QuadPart - t1.QuadPart)*1000.0/tc.QuadPart);
	//WriteSumAndSqSumToFile(sum, sqsum);
	//LARGE_INTEGER t1,t2,tc,t4,t5;
	//LONGLONG SetImage = 0;
	//LONGLONG RunImage = 0;
   	//QueryPerformanceFrequency(&tc);
	//QueryPerformanceCounter(&t1);
	//QueryPerformanceCounter(&t2);
	//LONGLONG overhead = t2.QuadPart - t1.QuadPart;
	//	QueryPerformanceCounter(&t1);
	//int count = 0;
	//int count2 = 0;
	//���ϵ������ڳ߶ȣ�ֱ������ͼ���Ե(_img->cols-10) ||(_img->rows - 10)
	//����ȷ���߶�С��3��(96)
    for( factor32x = 32; factor32x*cascade->orig_window_size.width < (_img->cols - 10)<<5 &&
		factor32x*cascade->orig_window_size.height < (_img->rows - 10)<<5
		&&factor32x<96;
	factor32x = (factor32x*scale_factor32x+16)>>5)
    {
		
        const int ystep32x = MAX(64, factor32x);

		//�����������ڳ߶�
        Size win_size;
		win_size.height = (cascade->orig_window_size.height * factor32x + 16)>>5;
		win_size.width = (cascade->orig_window_size.width * factor32x + 16 )>>5;
		
       //passָɨ�������stage_offsetָ�ڶ���ɨ��ʱ�ӵڼ���stage��ʼ
        int pass, stage_offset = 0;
		
		//ȷ�����������ڳ߶ȷŴ����Ȼ��ͼ����
        int stop_height =  ( ((_img->rows - win_size.height)<<5)+ (ystep32x>>1) ) / ystep32x;
		
		//ȷ���������ڴ���Ŀ�����С�ߴ�
        if( win_size.width < minSize.width || win_size.height < minSize.height )
            continue;
		//QueryPerformanceFrequency(&tc);
		//QueryPerformanceCounter(&t1);
		//���ݳ߶�������ʽ�����������е�������Ȩ�أ���������Щ�����ڻ���ͼ�е�λ�ã��Լ�������
        SetImagesForHaarClassifierCascade(cascade, &sum, &sqsum, factor32x );
		//QueryPerformanceCounter(&t2);
		//SetImage = (t2.QuadPart - t1.QuadPart) - overhead;
		//printf("SetImageFeatureRunTime:%fms\n",(t2.QuadPart - t1.QuadPart)*1000.0/tc.QuadPart);


		//���ôּ����ʹ�õ���ʼ������
		cascade->hid_cascade->count = split_stage;
	
		
		//�ü�ⴰ��ɨ������ͼ��:
		//��һ��ͨ����������stage���Զ�λĿ��������򣬶Ժ�ѡ������б궨������tmp����
		//�ڶ���Ա궨�ĺ�ѡ�����������ɸѡ������ѡ������õ�������
		for( pass = 0; pass < npass; pass++ )
		{

			for( int _iy = 0; _iy < stop_height; _iy++ )
			{	
				//��ⴰ�������경��Ϊ2�����ֲ���
				int iy = (_iy*ystep32x+16)>>5;
				int _ix, _xstep = 1;
				
				//stop_width��ָ_ix���������ޣ�_ix��Ҫ*ystep32x���������Ĵ�������
				int stop_width =( ((_img->cols - win_size.width)<<5) +ystep32x/2) / ystep32x;
				unsigned char* mask_row = tmp.imgPtr + tmp.cols* iy;
				
				
				for( _ix = 0; _ix < stop_width; _ix += _xstep )
				{
					
					//��ⴰ�ں����갴����Ϊ4��ʼ�ƶ�����û�м�⵽Ŀ�꣬��ı���һ�β���Ϊ2
					int ix = (_ix*ystep32x+16)>>5; // it really should be ystep
				
					//��ǰ��ⴰ�����Ͻ�����
					Point pt;
					pt.x = ix;
					pt.y = iy;

					//���Լ��
					if( pass == 0 )
					{
						
						int result = 0;
						_xstep = 2;
						//QueryPerformanceFrequency(&tc);
						//QueryPerformanceCounter(&t4);
						result = RunHaarClassifierCascade( cascade, pt, 0 );
						//count++;
						//QueryPerformanceCounter(&t5);
						//RunImage += (t5.QuadPart - t4.QuadPart) - overhead;
						//printf("RunHaarRunTime:%fms\n",(t2.QuadPart - t1.QuadPart)*1000.0/tc.QuadPart);
						//
						if( result > 0 )
						{
							if( pass < npass - 1 )
								mask_row[ix] = 1;
						
						}
						//û�м�⵽�ı䲽��Ϊ2����ix��ֵ��
						if( result < 0 )
							_xstep = 1;
					}
					//�ڶ��μ����ǰ�ֶ�λ������
					else if( mask_row[ix] )
					{
						//дMask�ļ����Ƿ���ȷ
						//WriteMaskRowToFile(tmp);
						/*FILE* fp;
						
						fp = fopen("..//MaskRowResult//maskRow2.txt","a+");
						fprintf(fp, "%d %d\n ", ix, iy);	
						fclose(fp);*/
						//QueryPerformanceFrequency(&tc);
						//QueryPerformanceCounter(&t4);
						
						int result = RunHaarClassifierCascade(cascade, pt, stage_offset);
						//QueryPerformanceCounter(&t5);
						//RunImage += (t5.QuadPart - t4.QuadPart) - overhead;
						//count2++;
						//int result = 0;
						if( result > 0 )
						{
						
					
							seq.rectQueue[seq.tail].height = win_size.height;
							seq.rectQueue[seq.tail].width = win_size.width;
							seq.rectQueue[seq.tail].x = ix;
							seq.rectQueue[seq.tail].y = iy;
							seq.total++;
							seq.tail++;
						}
						else
							mask_row[ix] = 0;
				}
			}

		
        }

		//��Ϊǰ����stage�ڵ�һ�μ���ʱ���Ѿ��ù���
		//�ڶ��μ���ʱ�򣬴ӵ�3��stage��ʼ���������ļ��
		stage_offset = cascade->hid_cascade->count;
		cascade->hid_cascade->count = cascade->count;
		//cascade->hid_cascade->count = 15;
	}
 }
	//cascade->hid_cascade->count = cascade->count;
	//QueryPerformanceCounter(&t2);
	//printf("HaarRunTime:%fms\n",(t2.QuadPart - t1.QuadPart)*1000.0/tc.QuadPart);
	if( min_neighbors != 0 )
    {
	
        //����ѡĿ�갴���ƶȹ��ɲ��鼯
		//����ֵ�����鼯���ĸ���
        int ncomp = SeqPartition(&seq);
	
		
		
        //�����ں�ѡ��������ۼӣ�Ϊ����ƽ���߽���׼��
        for( i = 0; i < seq.total; i++ )
        {
            Rect r1 = seq.rectQueue[i];
			PTreeNode* node = &PTreeNodes[i];
			while(node->parent)
				node = node->parent;
            int idx = (node - PTreeNodes);
           
			
            comps.neighbors[idx]++;
			
            comps.rectQueue[idx].x += r1.x;
            comps.rectQueue[idx].y += r1.y;
            comps.rectQueue[idx].width += r1.width;
            comps.rectQueue[idx].height += r1.height;
        }

		// ����ƽ��Ŀ��߽�
        for( i = 0; i < seq.total; i++ )
        {
            int n = comps.neighbors[i];

			//ֻ��������С�ٽӵĽ���������ս��
            if( n >= min_neighbors )
            {
                Rect* rect = &seq2.rectQueue[seq2.tail];
                rect->x = (comps.rectQueue[i].x*2 + n)/(2*n);
                rect->y = (comps.rectQueue[i].y*2 + n)/(2*n);
				rect->width = (comps.rectQueue[i].width*2 + n)/(2*n);
                rect->height = (comps.rectQueue[i].height*2 + n)/(2*n);
                seq2.neighbors[seq2.tail] = comps.neighbors[i];
				seq2.tail++;
                seq2.total++;
            }
        }

		//const char* p1="D:\\test.bmp";
		//IplImage * img=cvCreateImage(cvSize(752,480),8,1);
		//img->imageData = (char*)_img->imgPtr;
		//cvSaveImage(p1,img);

		//�Ӻ�ѡ�����еõ����ľ���
        for( i = 0; i < seq2.total; i++ )
        {
			Rect r1 = seq2.rectQueue[i];
            int r1_neighbor = seq2.neighbors[i];
            int j, flag = 1;
			
            for( j = 0; j < seq2.total; j++ )
            {
                Rect r2 = seq2.rectQueue[j];
				int r2_neighbor = seq2.neighbors[j];
                int distance = (r2.width *2+5)/10;//cvRound( r2.rect.width * 0.2 );
				
                if( i != j &&
                    r1.x >= r2.x - distance &&
                    r1.y >= r2.y - distance &&
                    r1.x + r1.width <= r2.x + r2.width + distance &&
                    r1.y + r1.height <= r2.y + r2.height + distance &&
                    (r2_neighbor > MAX( 3, r1_neighbor ) || r1_neighbor < 3) )
                {
                    flag = 0;
                    break;
                }
            }
			
            if( flag )
            {
				r1.x=beishu*r1.x+tl_x;
				r1.y=beishu*r1.y+tl_y;
				r1.width=beishu*r1.width;
				r1.height=beishu*r1.height;
				result_seq.rectQueue[result_seq.tail] = r1;
				result_seq.tail++;
				result_seq.total++;
            }//8.22
        }
		
	}

	//printf("SetImageTime:%fms\n",SetImage*1000.0/tc.QuadPart);
	//printf("RunImageTime:%fms\n",RunImage*1000.0/tc.QuadPart);
}

void DownSample(Image* pImage, int factor)
{
	int i = 0;
	int j = 0;
	int counti = 0;
	int countj = 0;

	int step = pImage->cols / factor;
	for (i =0; i < pImage->rows; i+= factor)
	{
		countj++;
		for (j =0; j < pImage->cols; j += factor)
		{
			*(pImage->imgPtr + i*step/factor + j/factor) = *(pImage->imgPtr + i*pImage->cols + j);
			counti++;
		}
		counti = 0;
	}
	
	pImage->cols /= factor;
	pImage->rows /= factor;
}

void Get_roi(Image* _img,char * srcdata,int tl_x,int tl_y,int roi_width,int roi_height)
{
	uchar * dst_line;
	uchar * src_line;

	_img->cols=roi_width;
	_img->rows=roi_height;
	dst_line =_img->imgPtr;
	src_line = (uchar*)srcdata+tl_x+tl_y*752;
	for(int i=0;i<roi_height;i++)
	{
		for(int j=0;j<roi_width;j++)
			dst_line[j]=src_line[j];
		dst_line +=roi_width;
		src_line +=752;
	}
}
