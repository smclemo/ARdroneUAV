#include "CRecognition.h"

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

CRecognition::CRecognition()
{
	memset(learned,0,sizeof(unsigned char)*3);
	memset(colorArray,0,64*64*64);
	learned[0] = 10;
	learned[1] = 10;
	learned[2] = 60;
	tolerance = 30; 
	debug = false; 
}

CRecognition::~CRecognition()
{
}

//tolerance treshold increase
void CRecognition::increaseTolerance()
{
	tolerance+=5;
	if (tolerance > 400) tolerance = 400;
	fprintf(stdout,"Tolerance: %i\n",tolerance);
}

//tolerance treshold decrease
void CRecognition::decreaseTolerance()
{
	tolerance-=5;
	if (tolerance < 0) tolerance = 0;
	fprintf(stdout,"Tolerance: %i\n",tolerance);
}

//index table reset 
void CRecognition::resetColorMap()
{
	memset(colorArray,0,64*64*64);
}

//learns a given pixel
void CRecognition::learnPixel(unsigned char* a)
{
	//saves the pixel
	for (int i =0;i<3;i++) learned[i] = a[i];

	//transforms it to HSV
	rgbToHsv(learned[0],learned[1],learned[2],&learnedHue,&learnedSaturation,&learnedValue);

	//creates an indexing table
	unsigned char u[3];
	for (u[0]=0;u[0]<252;u[0]=u[0]+4){
		for (u[1] = 0;u[1]<252;u[1]=u[1]+4){
			for (u[2] = 0;u[2]<252;u[2]=u[2]+4){
				int i = ((u[0]/4)*64+u[1]/4)*64+u[2]/4;
				if (colorArray[i] == 0 ) colorArray[i] = evaluatePixel3(u);
			}
		}
	}
	fprintf(stdout,"Learned RGB: %i %i %i, HSV: %i %i %i\n",learned[0],learned[1],learned[2],learnedHue,learnedSaturation,learnedValue);
}

//pixel classification by index table 
int CRecognition::evaluatePixelFast(unsigned char *a)
{
	int b = ((a[0]/4)*64+a[1]/4)*64+a[2]/4;
	return colorArray[b];
}

//pixel classification
float CRecognition::evaluatePixel1(unsigned char* a)
{
	float result = 1;
	for (int i =0;i<3;i++){ 	
		result += pow((int)a[i]-(int)learned[i],2);
	}
	return 1/result;
}

//pixel classification
float CRecognition::evaluatePixel2(unsigned char* a)
{
	float result = 0;
	for (int i =0;i<3;i++){ 	
		result += pow((int)a[i]-(int)learned[i],2);
	}
	result = sqrt(result);
	if (result > tolerance) result = 0; else result = 1;
	return result;
}

//pixel classification
float CRecognition::evaluatePixel3(unsigned char* a)
{
	float result = 0;
	unsigned int h;
	unsigned char s,v;
	rgbToHsv(a[0],a[1],a[2],&h,&s,&v);
	if (v > 50 && s > 50){
		result = result + pow((int)h-(int)learnedHue,2);
		result = result + pow((int)s-(int)learnedSaturation,2)/4;
		result = result + pow((int)v-(int)learnedValue,2)/16;
	}else{
		return 0;
	}
	result = sqrt(result);
	if (result > tolerance) result = 0; else result = 1;
	return result;
}

//image segmentation
SPixelPosition CRecognition::findSegment(CRawImage* image)
{
	SPixelPosition result;
	result.x = image->width/2;
	result.y = image->height/2;

	int expand[4] = {image->width,-image->width,1,-1};
	int stack[image->width*image->height];
	int stackPosition = 0;

	int numSegments = 0;
	int buffer[image->width*image->height];
	int len = image->width*image->height;

	//pixel classification
	for (int i = 0;i<len;i++) buffer[i] = -evaluatePixelFast(&image->data[3*i]);

	//image borders cutoff
	int pos =  (image->height-1)*image->width;
	for (int i = 0;i<image->width;i++){
			 buffer[i] = 0;	
			 buffer[pos+i] = 0;
	}
	for (int i = 0;i<image->height;i++){
			 buffer[image->width*i] = 0;	
			 buffer[image->width*i+image->width-1] = 0;
	}

	//segmentation start
	int position = 0; 
	for (int i = 0;i<len;i++){

		//if a new segment is found 
		if (buffer[i] < 0 && numSegments < MAX_SEGMENTS){
			//we create a new struct for it
			buffer[i] = ++numSegments;
			segmentArray[numSegments-1].size = 1; 
			segmentArray[numSegments-1].x = i%image->width; 
			segmentArray[numSegments-1].y = i/image->width; 
			//and put its coords at the top of a stack
			stack[stackPosition++] = i;
			//and until is the stack not empty  
			while (stackPosition > 0){
				//we pop the position of a pixel from the top of the stack
				position = stack[--stackPosition];
				//adn search its neighbours
				for (int j =0;j<4;j++){
					pos = position+expand[j];
					//and if they are classified as valid
					if (buffer[pos] < 0){
						//we push them onto a stack,
						stack[stackPosition++] = pos;
						segmentArray[numSegments-1].x += pos%image->width; 
						segmentArray[numSegments-1].y += pos/image->width; 
						//and increase the segment size 
						segmentArray[numSegments-1].size++; 
						buffer[pos] = numSegments;
					}
				}
			}
			//as soon as the stack is empty, we compute its center
			segmentArray[numSegments-1].x = segmentArray[numSegments-1].x/segmentArray[numSegments-1].size; 
			segmentArray[numSegments-1].y = segmentArray[numSegments-1].y/segmentArray[numSegments-1].size; 
		}
	}

	//we find the largest segment
	int maxSize = 0;
	int index = 0;
	int i;
	for (i =0;i<numSegments;i++){
		if (maxSize < segmentArray[i].size){
			index = i;
			maxSize = segmentArray[i].size;
		}			
	}
	if (debug) fprintf(stdout,"Largest segment is %i %i %i %i\n",index,segmentArray[index].size,segmentArray[index].x,segmentArray[index].y);

	if (maxSize > 20){
		result.x = segmentArray[index].x;
		result.y = segmentArray[index].y;
	} 

	//and draw the results
	int j = 0;
	for (int i = 0;i<len;i++){
		j = buffer[i];
		if (j > 0){
                        image->data[i*3+j%3] = 0;
                        image->data[i*3+(j+1)%3] = 255;
                        image->data[i*3+(j+2)%3] = 255;
		}
	
	}

	return result;
}

//transf RGB -> HSV, taken from some www
void CRecognition::rgbToHsv(unsigned char r, unsigned char  g, unsigned char b, unsigned int *hue, unsigned char *saturation, unsigned char *value )
{
	float min, max, delta;
	float h,s,v;   

	h=s=v=0; 
	*saturation = (unsigned char) s;
	*value = (unsigned char) v;
	*hue = (unsigned int) h;

	min = min( r, min(g, b) );
	max = max( r, max(g, b) );
	v = max;			

	delta = max - min;

	if( max != 0 )
		s = min(delta*255 / max,255);	
	else {
		s = 0;
		h = -1;
		return;
	}

	if( r == max )
		h = ( g - b ) / delta;		// between yellow & magenta
	else if( g == max )
		h = 2 + ( b - r ) / delta;	// between cyan & yellow
	else
		h = 4 + ( r - g ) / delta;	// between magenta & cyan
	h = h*60;
	if (h<0) h+=360;
	*saturation = (unsigned char) s;
	*value = (unsigned char) v;
	*hue = (unsigned int) h;
}

