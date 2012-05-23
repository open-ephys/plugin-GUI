#include "PlotUtils.h"

void checkGlError(){
	GLenum errCode;
	const GLubyte *errString;
	if ((errCode = glGetError()) != GL_NO_ERROR) {
	    errString = gluErrorString(errCode);
	   	fprintf (stderr, "OpenGL Error: %s\n", errString);
		exit(1);
	}
	else
		std::cout<<"OpenGL Okay!"<<std::endl;
}

void drawString(float x, float y, void *f, const char *string){
	glRasterPos2f(x, y);
	int len = strlen(string);
	// glColor3f(1.0, 1.0, 1.0);
	for (int i = 0; i < len; i++) {
 		//glutBitmapCharacter(f, string[i]);
	}
}

void drawString(float x, float y, int size, String s, FTPixmapFont* f){
	
	glRasterPos2f(x, y);

	f->FaceSize(size);
	f->Render(s);
}

void drawViewportEdge(){
	
	glPushMatrix();
	glLoadIdentity();
	
	glBegin(GL_LINE_LOOP);
		glVertex2f(-.995, -.995);
		glVertex2f( .995, -.995);
		glVertex2f( .995, .995);
		glVertex2f(-.995, .995);
	glEnd();
	
	glPopMatrix();
}

void drawViewportCross(){

	glColor3f(0.0,1.0,1.0);
	
	glPushMatrix();
	glLoadIdentity();
	
	glBegin(GL_LINE_LOOP);
		glVertex2f(-.995, -.995);
		glVertex2f( .995, .995);
		glVertex2f( .995, -.995);
		glVertex2f(-.995, .995);
	glEnd();
	
	glPopMatrix();
}

void setViewportRange(int xMin,int yMin,int xMax,int yMax){
	
	float dx = xMax-xMin;
	float dy = yMax-yMin;
	
//	printf("Setting viewport to:%d,%d %d,%d with dims%d,%d %d,%d\n", x,y,w,h, xMin, xMin, xMax,yMax);
//	printf("Dx:%f Dy:%f, Scaling viewport by:%f,%f \n", dx,dy,2.0/dx, 2.0/dy);
	glLoadIdentity();
	glTranslatef(-1.0,-1.0,0.0); 
	glScalef(2.0f/dx, 2.0f/dy, 1.0);
	glTranslatef(0-xMin, 0-yMin, 0);
	
}
int roundUp(int numToRound, int multiple) 
{ 
	if(multiple == 0) 
 	{ 
  		return numToRound; 
 	} 

 	int remainder = numToRound % multiple;
 	if (remainder == 0)
  		return numToRound;
 	return numToRound + multiple - remainder;
}

double ad16ToUv(int x, int gain){	
	int result =  (double)(x * 20e6) / (double)(gain * pow(2,16));
	return result;
}

void makeLabel(int val, int gain, bool convert, char * s){
	if (convert){
		double volt = ad16ToUv(val, gain)/1000.;
		if (abs(val)>1e6){
			//val = val/(1e6);
			sprintf(s, "%.2fV", volt);
		}
		else if(abs(val)>1e3){
			//val = val/(1e3);
			sprintf(s, "%.2fmV", volt);
		}
		else
			sprintf(s, "%.2fuV", volt);
	}
	else
		sprintf(s,"%d", (int)val);		
}

void n2ProjIdx(int proj, int *p1, int *p2){
    int d1, d2;
	if (proj==PROJ1x2){
		d1 = 0;
		d2 = 1;
	}
	else if(proj==PROJ1x3){
		d1 = 0;
		d2 = 2;
	}
	else if(proj==PROJ1x4){
		d1 = 0;
		d2 = 3;
	}
	else if(proj==PROJ2x3){
		d1 = 1;
		d2 = 2;
	}
	else if(proj==PROJ2x4){
		d1 = 1;
		d2 = 3;
	}
	else if (proj==PROJ3x4){
		d1 = 2;
		d2 = 3;
	}
	else{
		std::cout<<"Invalid projection:"<<proj<<"! Cannot determine d1 and d2"<<std::endl;
        *p1 = -1;
        *p2 = -1;
		return;
	}
    *p1 = d1;
    *p2 = d2;
}


bool isFrameBufferExtensionSupported(){

    std::cout<<"Checking to see if the OpenGL Frame Buffer Extension is Supported"<<std::endl;
    
    char* str = 0;
    char* tok = 0;

    std::string fboExt = "GL_EXT_framebuffer_object";

    str = (char*)glGetString(GL_EXTENSIONS);
           
    if(str)
    {
        std::vector <std::string> extensions;
        tok = strtok((char*)str, " ");
        while(tok)
        {          

            std::string ext = tok;

            if (ext == fboExt)
                return true;

            tok = strtok(0, " ");         
        }
        return false;
    }
    else
        return false;
}

bool checkFramebufferStatus()
{
    // check FBO status
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    return status == GL_FRAMEBUFFER_COMPLETE_EXT;

    switch(status)
    {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
        std::cout << "Framebuffer complete." << std::endl;
        return true;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Attachment is NOT complete." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: No image is attached to FBO." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Attached images have different dimensions." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Color attached images have different internal formats." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Draw buffer." << std::endl;
        return false;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        std::cout << "[ERROR] Framebuffer incomplete: Read buffer." << std::endl;
        return false;

    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        std::cout << "[ERROR] Unsupported by FBO implementation." << std::endl;
        return false;

    default:
        std::cout << "[ERROR] Unknown error." << std::endl;
        return false;
    }
}

// std::addressof was introduced in C++11, an equivalent function is defined below
// definition from http://en.cppreference.com/w/cpp/memory/addressof

