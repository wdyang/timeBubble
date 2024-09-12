#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup(){
	//ofSetVerticalSync(true);
	//Use ps3, if ps3 is available
    //Other wise use firewire
    //otherwise use buildin webcam

	int numPs3Eye = ps3eye.listDevices().size();
    cout<<"Number of PS3 Eyes connected "<<numPs3Eye<<endl;
    if(numPs3Eye>0){
        bUsePs3Eye = true;
        ps3eye.setDesiredFrameRate(30);
        ps3eye.initGrabber(camWidth,camHeight);
        
        ps3eye.setAutoGainAndShutter(false); // otherwise we can't set gain or shutter
        ps3eye.setGain(ps3Gain);
        ps3eye.setShutter(ps3Shutter);
        ps3eye.setGamma(0.5);
        ps3eye.setBrightness(0.6);
        ps3eye.setContrast(1.0);
        ps3eye.setHue(0.0);
        
        ps3eye.setFlicker(1);
    }else{
        bUsePs3Eye = false;
        vidGrabber.setVerbose(true);
        cout<<"listing devices"<<endl;
        int numCam = vidGrabber.listDevices().size();
        cout<<"end listing devices, num devices: "<<numCam<<endl;
        vidGrabber.setDeviceID(numCam-1); //use the last cam in the chain, so if a firewire cam is attached, it will be used
        vidGrabber.setDesiredFrameRate(30);
        vidGrabber.initGrabber(camWidth,camHeight);
        ofSetVerticalSync(true);
    }
    ofSetFrameRate(120);
    
    grayLastImage.allocate(camWidth, camHeight);
    grayDiffImage.allocate(camWidth, camHeight);
    grayTimeDepthImage.allocate(camWidth, camHeight);
    grayWarpImage.allocate(camWidth, camHeight);
    grayTimeDepthImage_x40y40.allocate(40, 40);
    grayTimeDepthImage_x8y8.allocate(8, 8);
    grayTimeDepthImage_x6y2.allocate(6, 2);
    grayTimeDepthImage_x4y1.allocate(4, 1);
    
//    grayPaintImage.allocate(camWidth, camHeight);
//    grayPaintImage.set(0);
    
    grayLastImage.set(0);
    grayDiffImage.set(0);
    grayTimeDepthImage.set(0);
    grayTimeDepthImage_x40y40.set(0);
    grayTimeDepthImage_x8y8.set(0);
    grayTimeDepthImage_x6y2.set(0);
    grayTimeDepthImage_x4y1.set(0);
    
    for(int i=0; i<bufferDepth; i++){
        ofxCvGrayscaleImage tempImage;
        tempImage.allocate(camWidth, camHeight);
        grayArray.push_back(tempImage);
    }
    
    //    Setup TouchOSC
    ipadReceiver.setup(PORT_FROM_IPAD);
    //    ipadSender.setup(HOST_IPAD, PORT_TO_IPAD);
    
    music.loadSound("StreakCamera.mp3");
    
    //syphon
    outputSyphonServer.setName("TimeBubble");
    mClient.setup();
    mClient.set("","Simple Server");
}


//--------------------------------------------------------------
void testApp::update(){
    bool bNewFrameReady = false;
    unsigned char * pixels;
    if(bUsePs3Eye){
        ps3eye.update();
        if(ps3eye.isFrameNew()) {
            bNewFrameReady = true;
            pixels = ps3eye.getPixels();
        }
    }else{
        vidGrabber.update();
        if(vidGrabber.isFrameNew()){
            bNewFrameReady = true;
            pixels = vidGrabber.getPixels();
        }
    }

    if(bNewFrameReady){
        colorImage.setFromPixels(pixels, camWidth, camHeight);
        grayImage=colorImage;

        if(bSourceInvert) grayImage.invert();
        
        grayImage.mirror(bFlipVertically, bFlipHorizontally);
        grayDiffImage.absDiff(grayImage, grayLastImage);
        grayDiffImage.brightnessContrast(0.2, 0.3);
//        grayDiffImage.threshold(30);
        grayDiffImage.blurHeavily();
        grayDiffImage.blurHeavily();
        
        grayTimeDepthImage-=4;
//        grayDiffImage-=1;
        grayTimeDepthImage+=grayDiffImage;

        grayTimeDepthImage_x40y40.scaleIntoMe(grayTimeDepthImage);
        grayTimeDepthImage_x8y8.scaleIntoMe(grayTimeDepthImage_x40y40);
        grayTimeDepthImage_x6y2.scaleIntoMe(grayTimeDepthImage);
        grayTimeDepthImage_x4y1.scaleIntoMe(grayTimeDepthImage_x8y8);

        setGrayArray();
        switch(mode){
            case 0:
                setGrayWarpImage();
                break;
            case 1:
                setGrayWarpImage_dxy(40, 40);
                break;
            case 2:
                setGrayWarpImage_dxy(8, 8);
                break;
            case 3:
                setGrayWarpImage_dxy(6, 2);
                break;
            case 4:
                setGrayWarpImage_dxy(4, 1);
                break;
            case 5:
                paintImage();
                break;
            case 6:
                drawDelayImage();
                break;
            default:
                cout<<"error in mode number\n";
                break;
        }
//        setGrayWarpImage_dxy(40, 40);//xsize, ysize
//        setGrayWarpImage_dxy(8, 8);

        grayLastImage = grayImage;
        numFrames++;
	}
        
#ifdef USE_PS3
    else ps3eye.setLed(false);
#endif

    while(ipadReceiver.hasWaitingMessages()){
        parseIpadOSCMessage();
    }
    
    if(music.getIsPlaying()){
        oscSendString("/1/labelMusicPositionMs", msToTime(music.getPositionMS()));
        oscSendFloat("/1/musicPosition", music.getPosition());
    }

}

//--------------------------------------------------------------

void testApp::draw(){
    ofBackground(0,0,0);
	ofSetHexColor(0xffffff);
//	ofSetHexColor(0x000000);

//    grayArray[lastFrame].draw(0, 0, camWidth/2, camHeight/2);
//    grayWarpImage.draw(20+camWidth/2, 0, camWidth/2, camHeight/2);
//    if(mode<7){
    grayWarpImage.draw(0, 0, displayWidth, displayHeight);
//    }else{ //mode 5 is painting
//        grayPaintImage.draw(0,0,displayWidth, displayHeight);
//    }
	
    if(bShowVideo){
        if(bUsePs3Eye){
            ps3eye.draw(20,20+displayHeight, camWidth/2, camHeight/2);
            ofDrawBitmapString("Ps3Eye FPS: "+ ofToString(ps3eye.getRealFrameRate()), 20,15+displayHeight);
        }else{
            vidGrabber.draw(20,20+displayHeight, camWidth/2, camHeight/2);
            ofDrawBitmapString("App FPS: "+ ofToString(ofGetFrameRate())  + " "+ ofToString(22%10), 20,15+displayHeight);
        }
        grayDiffImage.draw(40+ camWidth/2, 20+displayHeight, camWidth/2, camHeight/2);
        grayTimeDepthImage.draw(60+ camWidth, 20+displayHeight, camWidth/2, camHeight/2);
        grayTimeDepthImage_x40y40.draw(80+camWidth*3/2, 620);
        grayArray[nextFrame].draw(40+displayWidth, 200, camWidth/2, camHeight/2);
    
    }
//    mClient.draw(0,0);
    outputSyphonServer.publishScreen();


    int x0=40+displayWidth, y0=15;
    ofDrawBitmapString("Mode(m): "+ ofToString(mode)  + " cutRatio(+-) "+ ofToString(cutRatio)+ " reverseTime(r) "+ ofToString(reverseTime), x0,y0);
    ofDrawBitmapString("Mode: 0:raw, 1:40x40, 2:8x8; 3:6x2, 4:4x1, 5:painting, 6:No Bubble", x0, y0+=20);
    ofDrawBitmapString("CutRatio: +: increase time depth, -:reduces it ", x0, y0+=20);
    ofDrawBitmapString("ReverseTime: 1: movement brings back the past, 0: movement => present", x0, y0+=20);
    ofDrawBitmapString("(h): mirror, ( ) save image, (v) show videos", x0, y0+=20);
    ofDrawBitmapString("(f): full screen, (w): switch display size", x0, y0+=20);
    ofDrawBitmapString("fps: "+ ofToString(ofGetFrameRate()), x0, y0+=20);

    if(bUsePs3Eye){
        ofDrawBitmapString("(a): gain + , (z) gain -, gain: " + ofToString(ps3Gain), x0, y0+=20);
        ofDrawBitmapString("(s): shutter + , (x) shutter -, shutter: " + ofToString(ps3Shutter), x0, y0+=20);
    }
    
//    tex.

//    mClient.draw(50,50);
//    outputSyphonServer.publishTexture(&texSyphon);
//    mainOutputSyphonServer.publishScreen();
}

    
void testApp::setGrayArray(){
//    copy current video image to buffer array
    unsigned char* ptrNewImage = grayImage.getPixels();
//    cout<<currentFrame<<endl;
    grayArray[currentFrame].setFromPixels(ptrNewImage, camWidth, camHeight);
    lastFrame = currentFrame;
    currentFrame++;
    
    if(currentFrame>=bufferDepth) {currentFrame = 0;}
    nextFrame = currentFrame;
    
//    cout<<"last: "<<lastFrame<<", cur: "<<currentFrame<<", next: "<<nextFrame<<endl;
}
    

void testApp::setGrayWarpImage(){
    unsigned char* ptr = grayWarpImage.getPixels();
    unsigned char* ptrDepth = grayTimeDepthImage.getPixels();
    for(int y=0; y<camHeight; y++){
        int idx = y*camWidth;
        for(int x=0; x<camWidth;x++){
            idx++;
            int depth=0;
            if(reverseTime){
                depth = (lastFrame - ptrDepth[idx]/cutRatio*bufferMulti);// % (256*bufferMulti);
            }else{
                depth = (lastFrame - (256 - ptrDepth[idx])/cutRatio*bufferMulti);// % (256*bufferMulti);
            }
            if(depth<0){depth+= bufferDepth;}
            //            printf("%d\n", depth);
            unsigned char* tptr = grayArray[depth].getPixels();
            ptr[idx]=tptr[idx];
        }
    }
    grayWarpImage.setFromPixels(ptr, camWidth, camHeight);
}

void testApp::setGrayWarpImage_dxy(int x_size, int y_size){
    unsigned char* ptr = grayWarpImage.getPixels();
    unsigned char* ptrDepth;
    switch(mode){
        case 1:
            ptrDepth= grayTimeDepthImage_x40y40.getPixels();
            break;
        case 2:
            ptrDepth= grayTimeDepthImage_x8y8.getPixels();
            break;
        case 3:
            ptrDepth= grayTimeDepthImage_x6y2.getPixels();
            break;
        case 4:
            ptrDepth= grayTimeDepthImage_x4y1.getPixels();
            break;
        default:
            cout<< "error in mode\n";
            break;
    }
    
    
    int delta_x = camWidth/x_size;
    int delta_y = camHeight/y_size;
    for(int y=0; y<camHeight; y++){
        int idx = y*camWidth;
        int idx_y = (y/delta_x)*x_size;
        for(int x=0; x<camWidth;x++){
            idx++;
            int idx_x = x / delta_y;
            int idx_depth = idx_x + idx_y;
            int depth = 0;
            if(reverseTime){
                depth = (currentFrame -1 - ptrDepth[idx_depth]/cutRatio);// % bufferDepth;
                
            }else{
                depth = (currentFrame -1 - (bufferDepth - ptrDepth[idx_depth])/cutRatio);// % bufferDepth;
            }
            if(depth<0){depth+= bufferDepth;}
//            printf("%d\n", depth);
            unsigned char* tptr = grayArray[depth].getPixels();
            ptr[idx]=tptr[idx];
        }
    }
    grayWarpImage.setFromPixels(ptr, camWidth, camHeight);
}



void testApp::paintImage(){
    unsigned char* ptr = grayWarpImage.getPixels();
    unsigned char* ptrDepth = grayTimeDepthImage.getPixels();
    for(int y=0; y<camHeight; y++){
        int idx = y*camWidth;
        for(int x=0; x<camWidth;x++){
            idx++;
            ptr[idx]=ptrDepth[idx];
        }
    }
    grayWarpImage.setFromPixels(ptr, camWidth, camHeight);
}


void testApp::drawDelayImage(){
    unsigned char* ptr = grayWarpImage.getPixels();
    int depth=(lastFrame - 256/cutRatio*bufferMulti);// % (256*bufferMulti);
    if(depth<0){depth+= bufferDepth;}
    unsigned char* tptr = grayArray[depth].getPixels();
    
    for(int y=0; y<camHeight; y++){
        int idx = y*camWidth;
        for(int x=0; x<camWidth;x++){
            idx++;
            ptr[idx]=tptr[idx];
        }
    }
    grayWarpImage.setFromPixels(ptr, camWidth, camHeight);
}


void testApp::saveImage(){
    time_t now;
    char the_date[24];
    
    the_date[0] = '\0';
    
    now = time(NULL);
    
    if (now != -1)
    {
        strftime(the_date, 24, "%y_%m_%d_%H_%M_%S", gmtime(&now));
    }
    string fname;
    fname = "capture/"+string(the_date) + ".jpg";
    cout<<fname<<endl;
    
    ofImage tempImage;
    tempImage.setFromPixels(grayWarpImage.getPixels(), camWidth, camHeight, OF_IMAGE_GRAYSCALE);
    tempImage.saveImage(fname);
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){ 	
    
	switch(key){
        case 'v':
            bShowVideo = !bShowVideo;
            break;
        case 'h':
            bFlipHorizontally = !bFlipHorizontally;
            break;
        case 'f':
            ofToggleFullscreen();
            break;
        case ' ':
            saveImage();
            break;
        case '+':
            if(cutRatio>1) cutRatio/=2;
            cout<<"cutRaio is "<<cutRatio<<endl;
            break;
        case '-':
            if(cutRatio<128) cutRatio*=2;
            cout<<"cutRaio is "<<cutRatio<<endl;
            break;
        case 'm':
            mode++;
            if(mode>maxMode) mode=0;
            cout<<"mode is "<<mode<<endl;
            break;
        case 'r':
            reverseTime = !reverseTime;
            break;
        case 'w':
            if(displayWidth==800){
                displayWidth = 1024;
                displayHeight = 768;
            }else{
                displayWidth=800;
                displayHeight=600;
            }
            break;
        case 'a':
            if(bUsePs3Eye){
                if(ps3Gain<1) ps3Gain+=0.1;
                ps3eye.setGain(ps3Gain);
                cout<<"ps3Gain is "<<ps3Gain<<endl;
            }
            break;
        case 'z':
            if(bUsePs3Eye){
                if(ps3Gain>0) ps3Gain-=0.1;
                ps3eye.setGain(ps3Gain);
                cout<<"ps3Gain is "<<ps3Gain<<endl;
            }
            break;
        case 's':
            if(bUsePs3Eye){
                if(ps3Shutter<=1) ps3Shutter+=0.1;
                ps3eye.setShutter(ps3Shutter);
                cout<<"ps3Shutter is "<<ps3Shutter<<endl;
            }
            break;
        case 'x':
            if(bUsePs3Eye){
                if(ps3Shutter>0) ps3Shutter-=0.1;
                ps3eye.setShutter(ps3Shutter);
                cout<<"ps3Shutter is "<<ps3Shutter<<endl;
            }
            break;
    }
		
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){ 
	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
