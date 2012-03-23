#include "ProjectionAxes.h"

ProjectionAxes::ProjectionAxes():
					GenericAxes(),
					drawGrid(false),
					overlay(false),
					convertLabelUnits(true),
					buffIdx(-1),
					totalSpikes(0),
					newSpike(false),
					isTextureValid(false)
{	
	GenericAxes::type = PROJ1x2;
	GenericAxes::gotFirstSpike = false;
	GenericAxes::resizedFlag = false;
	
	ylims[0] = 0;
	ylims[1] = 1;
	setPointColor(1.0,1.0,1.0);
	n2ProjIdx(type, &ampDim1, &ampDim2);

	texWidth = BaseUIElement::width;
	texHeight = BaseUIElement::height;
}

ProjectionAxes::ProjectionAxes(int x, int y, double w, double h, int t):
					GenericAxes(x,y,w,h,t),
					drawGrid(true),
					overlay(false),
					convertLabelUnits(true),
					buffIdx(-1),
					totalSpikes(0),
					newSpike(false),
					isTextureValid(false)
{	
	GenericAxes::gotFirstSpike = false;
	GenericAxes::resizedFlag = false;

	setPointColor(1.0,1.0,1.0);

	n2ProjIdx(type, &ampDim1, &ampDim2);

	texWidth = BaseUIElement::width;
	texHeight = BaseUIElement::height;
}

void ProjectionAxes::updateSpikeData(SpikeObject s){
	//std::cout<<"ProjectionAxes::updateSpikeData()"<<std::endl;
	GenericAxes::updateSpikeData(s);

	buffIdx++;
	if (buffIdx >= AMP_BUFF_MAX_SIZE)
		buffIdx %= AMP_BUFF_MAX_SIZE;

	int idx1, idx2;
	calcWaveformPeakIdx(ampDim1,ampDim2,&idx1, &idx2);

	ampBuffer[0][buffIdx] = s.data[idx1];
	ampBuffer[1][buffIdx] = s.data[idx2];
	newSpike = true;
}

void ProjectionAxes::redraw(){
	
	BaseUIElement::redraw();
	
	plot();
	
	BaseUIElement::drawElementEdges();
}


void ProjectionAxes::plot(){

	setViewportRange(xlims[0], ylims[0], xlims[1], ylims[1]);

	drawSpikesToTexture(false);
	
	drawTexturedQuad();

	plotNewestSpike();

}

void ProjectionAxes::plotOldSpikes(bool allSpikes){
	glColor3f(1.0, 1.0, 1.0);
	glPointSize(1);
	
	int startIdx = (allSpikes) ? 0 : buffIdx;
	int stopIdx = (totalSpikes > AMP_BUFF_MAX_SIZE) ? AMP_BUFF_MAX_SIZE : buffIdx;
	std::cout<<"ProjectionAxes::plotOldSpikes() Ploting:"<<stopIdx - startIdx + 1<<" spikes."<<std::endl;
	glBegin(GL_POINTS);
	
	for (int i=startIdx; i<=stopIdx; i++)
		glVertex2i(ampBuffer[0][i], ampBuffer[1][i]);
	glEnd();

}
void ProjectionAxes::plotNewestSpike(){
	BaseUIElement::setGlViewport();
	setViewportRange(xlims[0], ylims[0], xlims[1], ylims[1]);

	glColor3f(1.0, 1.0, 0.0);
	glPointSize(4);

	glBegin(GL_POINTS);
        glVertex2i(ampBuffer[0][buffIdx], ampBuffer[1][buffIdx]);
	glEnd();

	newSpike = false;
}

 void ProjectionAxes::calcWaveformPeakIdx(int d1, int d2, int *idx1, int *idx2){

	int max1 = -1*pow(2,15);
	int max2 = max1;
	
	for (int i=0; i<s.nSamples ; i++){
		if (s.data[d1*s.nSamples + i] > max1)
		{	
			*idx1 = d1*s.nSamples+i;
			max1 = s.data[*idx1];
		}
		if (s.data[d2*s.nSamples+i] > max2)
		{	
			*idx2 = d2*s.nSamples+i;
			max2 = s.data[*idx2];
		}
	}
}

void ProjectionAxes::setPointColor(GLfloat r, GLfloat g, GLfloat b){
	pointColor[0] = r;
	pointColor[1] = g;
	pointColor[2] = b;
}
void ProjectionAxes::setGridColor(GLfloat r, GLfloat g, GLfloat b){
	gridColor[0] = r;
	gridColor[1] = g;
	gridColor[2] = b;
}


void ProjectionAxes::createTexture(){
	
  	std::cout<<"Creating a new texture of size:"<<texWidth<<"x"<<texHeight<<std::endl;
    //glDeleteTextures(1, &textureId);
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap generation included in OpenGL v1.4
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    isTextureValid = true;
    createFBO();
}

void ProjectionAxes::createFBO(){
	std::cout<<"Creating a new frame buffer object"<<std::endl;

	if (!isTextureValid)
		createTexture();

	glDeleteFramebuffers(1, &fboId);
	glDeleteRenderbuffers(1, &rboId);
	glGenFramebuffersEXT(1, &fboId);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);

	glGenRenderbuffersEXT(1, &rboId);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rboId);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, texWidth, texHeight);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, textureId, 0);

	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rboId);

	bool status = checkFramebufferStatus();
	if(!status){
	    std::cout<<"FrameBufferObject not created! Quitting!"<<std::endl;
	    exit(1);
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void ProjectionAxes::drawSpikesToTexture(bool allSpikes){

	std::cout<<"Populating the texture()"<<std::endl;
	if (!isTextureValid){
        createTexture();
        allSpikes = true;
    }

	glViewport(0,0,texWidth, texHeight);
	glLoadIdentity();
	setViewportRange(xlims[0], ylims[0], xlims[1], ylims[1]);
	// set the rendering destination to FBO
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);    
    // plot to the texture
    plotOldSpikes(allSpikes);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); 

}


void ProjectionAxes::drawTexturedQuad(){
	//glViewport(BaseUIElement::xpos, BaseUIElement::ypos + BaseUIElement::height, texWidth, texHeight);
	BaseUIElement::setGlViewport();
    glLoadIdentity();
   
    glBindTexture(GL_TEXTURE_2D, textureId);

    int size = 1;
    int texS = 1;

    glBegin(GL_QUADS);
        glColor4f(1, 1, 1, 1);
        glTexCoord2f(texS,  texS);  glVertex3f(size,        size,0);
        glTexCoord2f(0,     texS);  glVertex3f(-1 * size ,  size,0);
        glTexCoord2f(0,     0);     glVertex3f(-1 * size,   -1 * size,0);
        glTexCoord2f(texS,  0);     glVertex3f(size,        -1 * size,0);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, 0);

}