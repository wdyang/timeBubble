#include "touchOsc.h"


void testApp::parseIpadOSCMessage(){
    ofxOscMessage m;
    ipadReceiver.getNextMessage(&m);
    
    if(!bSenderLive){
        ipadIP = m.getRemoteIp();
        ipadSender.setup(ipadIP, PORT_TO_IPAD);
    }
    
    
    string msg_string="";
    string raw_address;
    
    raw_address = m.getAddress();
//    cout<<raw_address<<endl;

    if(raw_address=="/1/connect"){
        int val = m.getArgAsInt32(0);
        cout<<"connect request received: "<<val<<endl;
        if(val==0){
            oscSendInitConfig();
        }
    }else if(raw_address=="/1/showVideo"){
        bShowVideo = m.getArgAsInt32(0);
    }else if(raw_address=="/1/showScanImage"){
//        bShowScanImage = m.getArgAsInt32(0);
    }
    
    else if(raw_address=="/1/snapImage"){
        int val=m.getArgAsInt32(0);
        if(val==1) saveImage();
    }else if(raw_address=="/1/invertSource"){
        bSourceInvert = m.getArgAsInt32(0);
    }else if(raw_address=="/1/flipVertical"){
        bFlipVertically = m.getArgAsInt32(0);
    }else if(raw_address=="/1/flipHorizontal"){
        bFlipHorizontally = m.getArgAsInt32(0);
    }

    // Music player
// Add musicPosition, music msToTime() in testApp.h
    else if(raw_address=="/1/musicVolume"){
        music.setVolume(m.getArgAsFloat(0));
        oscSendFloat("/1/labelVolume", music.getVolume());
    }else if(raw_address=="/1/musicPlaying"){
        bool playing = m.getArgAsInt32(0);
        if(playing){
            music.play();
            music.setPosition(musicPosition);
        }else{
            musicPosition = music.getPosition();
            music.stop();
        }
    }else if(raw_address=="/1/musicReset"){
        if(m.getArgAsInt32(0)==0){
            musicPosition = 0.f;
            music.setPosition(musicPosition);
            oscSendFloat("/1/musicPosition", 0.f);
            oscSendFloat("/1/labelMusicPositionMs", 0.f);
        }
    }else if(raw_address=="/1/musicPosition"){
        musicPosition = m.getArgAsFloat(0);
        music.setPosition(musicPosition);
        oscSendFloat("/1/MusicPosition", musicPosition);
        oscSendString("/1/labelMusicPositionMs", msToTime(music.getPositionMS()));
        // End Music Player
    }
    else{
        cout<<"not handled: "<<raw_address<<endl;
    }

}

void testApp::oscSendInitConfig(){
    oscSendString("/1/connect/color", "green");
    oscSendInt("/1/showVideo", bShowVideo);
//    oscSendInt("/1/showScanImage", bShowScanImage);

    oscSendInt("/1/invertSource", bSourceInvert);
    oscSendInt("/1/flipVertical", bFlipVertically);
    oscSendInt("/1/flipHorizontal", bFlipHorizontally);
        
    oscSendFloat("/1/musicVolume", music.getVolume());
    oscSendInt("/1/musicPlaying", music.getIsPlaying());
    oscSendFloat("/1/labelMusicPositionMs", music.getPositionMS());
    oscSendFloat("/1/musicPosition", music.getPosition());
}

void testApp::oscSendString(const string &address, const string &msg){
    ofxOscMessage m;
    m.setAddress(address);
    m.addStringArg(msg);
    ipadSender.sendMessage(m);
}

void testApp::oscSendFloat(const string &address, float msg){
    ofxOscMessage m;
    m.setAddress(address);
    m.addFloatArg(msg);
    ipadSender.sendMessage(m);
}

void testApp::oscSendFormatedFloat(const string &address, float msg, int precision){
    char st[24], formatst[20];
    sprintf(formatst, "%%.%df", precision);
    sprintf(st, formatst, msg);
    cout<<formatst<<" "<<st<<endl;
    ofxOscMessage m;
    m.setAddress(address);
    m.addStringArg(st);
    ipadSender.sendMessage(m);
}


void testApp::oscSendInt(const string &address, int msg){
    ofxOscMessage m;
    m.setAddress(address);
    m.addIntArg(msg);
    ipadSender.sendMessage(m);
}



string testApp::msToTime(int ms){
    
    int sec = ms/1000;
    std::ostringstream s;
    s<<sec/60<<":"<<sec%60;
    
    return s.str();
}

