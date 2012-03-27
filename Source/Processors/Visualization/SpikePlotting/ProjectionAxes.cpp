#include "ProjectionAxes.h"

ProjectionAxes::ProjectionAxes():
					GenericAxes(),
					drawGrid(false),
					overlay(false),
					convertLabelUnits(true),
					buffIdx(-1),
					totalSpikes(0),
					newSpike(false),
					isTextureValid(false), 
					allSpikesNextRender(false)
{	
	GenericAxes::type = PROJ1x2;
	GenericAxes::gotFirstSpike = false;
	
	ylims[0] = 0;
	ylims[1] = 1;
	setPointColor(1.0,1.0,1.0);
	n2ProjIdx(type, &ampDim1, &ampDim2);

	clearOnNextDraw = false;
}

ProjectionAxes::ProjectionAxes(int x, int y, double w, double h, int t):
					GenericAxes(x,y,w,h,t),
					drawGrid(true),
					overlay(false),
					convertLabelUnits(true),
					buffIdx(-1),
					totalSpikes(0),
					newSpike(false),
					isTextureValid(false),
					allSpikesNextRender(false)
{	
	GenericAxes::gotFirstSpike = false;

	setPointColor(1.0,1.0,1.0);

	n2ProjIdx(type, &ampDim1, &ampDim2);

	clearOnNextDraw = false;
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
	//setViewportRange(xlims[0], ylims[0], xlims[1], ylims[1]);

	GLenum errCode;
	const GLubyte *errString;

	// Should we plot all spikes to the texture or just the newest one
	bool allSpikes = false;

	if (!isTextureValid){
		std::cout<<"ProjectionAxes::plot() --> Texture is invalid regenerating it!"<<std::endl;
        createTexture();
		allSpikes = true;
    }

	if (clearOnNextDraw){
		clearTexture();
		clearOnNextDraw = false;
	}
	
	drawSpikesToTexture(allSpikes);
	drawTexturedQuad();
	plotNewestSpike();

	// if there has been an openGL error we need to rerender the texture and replot everything
	// errors occur when the openGL context has been destroyed and recreated. I'm not sure how to 
	// explicitly catch that event so instead we check for a drawing error.
	if ((errCode = glGetError()) != GL_NO_ERROR) {
	   errString = gluErrorString(errCode);
	   std::cout<<"OpenGL Error:"<< errString << "! Invalidating and rerendering the texture!" << std::endl;
	   
	   invalidateTexture();
	   plot();
	   return;
	}
}

void ProjectionAxes::plotOldSpikes(bool allSpikes){
	std::cout<<"ProjectionAxes::plotOldSpikes() allSpikes:"<<allSpikes<<std::endl;
	
	//set the viewport to the size of the texture
	glViewport(0,0,texWidth, texHeight);
	
	//set the plotting range for the viewport to the limits of the plot
	setViewportRange(xlims[0], ylims[0], xlims[1], ylims[1]);
	
	// if allSpikes plot start with 1 else start with buffIdx
	int startIdx = (allSpikes) ? 1 : buffIdx;
	// either plot to totalSpikes or the end of the buffer if total spikes has wrapped
	int stopIdx = (totalSpikes > AMP_BUFF_MAX_SIZE) ? AMP_BUFF_MAX_SIZE : buffIdx;

	// if (allSpikes)
	// 	std::cout<<"\tUpdating texture with all spikes: "<< stopIdx - startIdx + 1 <<std::endl;

	glColor3f(1.0, 1.0, 1.0);
	glPointSize(1);
	glBegin(GL_POINTS);
		for (int i=startIdx; i<=stopIdx; i++)		
			glVertex2i(ampBuffer[0][i], ampBuffer[1][i]);
	glEnd();

}


void ProjectionAxes::plotNewestSpike(){
	
	BaseUIElement::setGlViewport();
	setViewportRange(xlims[0], ylims[0], xlims[1], ylims[1]);

	// draw the newest spike as a big red point so it stands out against the old spikes
	glColor3f(1.0, 0.0, 0.0);
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

void ProjectionAxes::setPosition(int x, int y, int w, int h){
	std::cout<<"ProjectionAxes::setPosition()"<<std::endl;
	
	// only invalidate the texture if its size has actually changed
	if (w!=GenericAxes::width || h!=GenericAxes::height)
		invalidateTexture();

	GenericAxes::setPosition(x,y,w,h);
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
		
	texWidth = BaseUIElement::width;
	texHeight = BaseUIElement::height;

  	std::cout<<"Creating a new texture of size:"<<texWidth<<"x"<<texHeight<<std::endl;
  	// Delete the old texture
    glDeleteTextures(1, &textureId);
    // Generate a new texture 
    glGenTextures(1, &textureId);
    // Bind the texture, and set the appropriate parameters
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    // generate a new FrameBufferObject
    createFBO();

    // the texture should now be valid, set the flag appropriately
    isTextureValid = true;

}

void ProjectionAxes::createFBO(){
	std::cout<<"Creating a new frame buffer object"<<std::endl;

	// if (!isTextureValid)
	// 	createTexture();
	// Delete the old frame buffer, render buffer
	glDeleteFramebuffers(1, &fboId);
	glDeleteRenderbuffers(1, &rboId);

	// Generate and Bind the frame buffer
	glGenFramebuffersEXT(1, &fboId);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);

	// Generate and bind the new Render Buffer
	glGenRenderbuffersEXT(1, &rboId);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rboId);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, texWidth, texHeight);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

	// Attach the texture to the framebuffer
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, textureId, 0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, rboId);

	// If the FrameBuffer wasn't created then we have a bigger problem. Abort the program.
	if(!checkFramebufferStatus()){
	    std::cout<<"FrameBufferObject not created! Are you running the newest version of OpenGL?"<<std::endl;
	    std::cout<<"FrameBufferObjects are REQUIRED! Quitting!"<<std::endl;
	    exit(1);
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void ProjectionAxes::drawSpikesToTexture(bool allSpikes){
	
	std::cout<<"ProjectionAxes::drawSpikesToTexture() plotting all spikes:"<<allSpikes<<std::endl;
	

	// For some reason if we want to plot ALL the spikes to a texture we must plot two draw cycles
	// in a row, perhaps this has to do with double buffering, i'm not sure why... investigae this!
	
	// if the allSpikes flag is set we set the allSpikesNextRender as true so we plot next
	// all spikes next render cycle too, if only the allSpikesNextRender is true we set all spikes
	// to true and allSpikes next render to false.  

	//Basically this logic ensures that if allSpikes is ever set to true it will be set to true
	//on the next call to drawSpikesToTexture() regardless of what value it is actually set to
	if (allSpikes)
		allSpikesNextRender = true;
	else if (!allSpikes && allSpikesNextRender)
	{
		allSpikes = true;
		allSpikesNextRender = false;
	}
	// set the rendering destination to FBO
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);    
	
    // plot to the texture
    plotOldSpikes(allSpikes);

    // bind the original FrameBuffer
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); 
}

void ProjectionAxes::invalidateTexture(){
	isTextureValid = false;
}


void ProjectionAxes::drawTexturedQuad(){
	BaseUIElement::setGlViewport();
	// We need to scale the viewport in this case because we want to fill it with a quad. 
	// if we load identity then we can use a quad bound by (-1, 1);
    glLoadIdentity();

    // Bind the texture to render
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Build the quad
    int size = 1;
    int texS = 1;
    glBegin(GL_QUADS);
        glColor4f(1, 1, 1, 1);
        glTexCoord2f(texS,  texS);  glVertex3f(size,        size,0);
        glTexCoord2f(0,     texS);  glVertex3f(-1 * size ,  size,0);
        glTexCoord2f(0,     0);     glVertex3f(-1 * size,   -1 * size,0);
        glTexCoord2f(texS,  0);     glVertex3f(size,        -1 * size,0);
    glEnd();
    
    // Unbind the texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ProjectionAxes::clear(){
	
	//reset buffIDx and totalSpikes
	buffIdx = 0;
	totalSpikes = 0;
	
	// set flag to clear on next draw
	clearOnNextDraw = true;	
}

void ProjectionAxes::clearTexture(){
	std::cout<<"ProjectinAxes::clearTexture() --> Clearing the Texture!"<<std::endl;

	glViewport(0,0,texWidth, texHeight);
	setViewportRange(xlims[0], ylims[0], xlims[1], ylims[1]);

	// set the rendering destination to FBO
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);    
    // Clear the framebufferObject
  	glClearColor (0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); 
}