#include "happlayermanager.h"
#include "ofxJsonSettings.h"

HapPlayerManager::HapPlayerManager(deque<PlayingInfo> *pq, ofMutex *pm,  VideoDatabaseLoader *dbl)
{
    this->playingQueue= pq;
    this->playingMutex = pm;
    playingVideoIndex = 0;
    lastVideoIndex =0;
    loadIndex =0;
    OVERLAY= false;

    switchTimer = ofGetElapsedTimeMillis();
    call_time = ofGetElapsedTimeMillis();

    DEV_MODE = Settings::getBool("dev_mode");
    videoPath = Settings::getString("video_path");
    int num_videos = -1;

    if (DEV_MODE){
         //DEV MODE -- loads less video
         num_videos = Settings::getInt("n_dev_videos");;
    }

    samplePlayer.init(dbl->getAudioPaths(), num_videos);
    loadVideoPaths(dbl->getVideoPaths(), num_videos);

    playNow(dbl->getVideoPaths()[0]);
}

// Destructor
HapPlayerManager::~HapPlayerManager() {
//todo : stop + close all videos

}

#include <sys/stat.h>
#include <fstream>
#include <unistd.h>

inline bool exists_test_why(const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}


inline bool exists_test (const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}


void HapPlayerManager::loadVideoPaths(vector<string> filepaths, int num_videos){
   if (num_videos==-1){
        num_videos = filepaths.size();
   }

   players.resize(num_videos);
   ofFbo loadFbo;
   loadFbo.begin();
   for (int i = 0; i<num_videos;i++){
       string movieFile = filepaths[i];

       size_t lastindex = movieFile.find_last_of(".");
       string rawname = movieFile.substr(0, lastindex);

       string fullpath =videoPath + movieFile;

       ofLogNotice(ofToString(ofGetElapsedTimef(),3)) << "Loading [" <<i <<"/"<<num_videos <<"] " << fullpath;

       if (!exists_test(fullpath)){
           ofLogError(ofToString(ofGetElapsedTimef(),3))<< "BROKEN damn FILE - "<< fullpath;
           continue;
       }
       players[i].status  = loading;
       players[i].loadTime = ofGetElapsedTimeMillis();
       players[i].video.setLoopState(OF_LOOP_NORMAL);
       players[i].videoID  = rawname;
       players[i].filePath  = fullpath;

       players[i].video.load(fullpath);
       players[i].loadTime = ofGetElapsedTimeMillis()-players[i].loadTime;
       players[i].video.play();


       while(players[i].video.getPosition()*players[i].video.getDuration() < 0.05 )
       {
           ofEventArgs e;
           players[i].video.update(e);
           players[i].video.draw(0,0);
           ofSleepMillis(10);
//            players[i].video.nextFrame();
       }
       players[i].video.setPosition(0);
       players[i].video.setPaused(true);
   }
   loadFbo.end();

}

void HapPlayerManager::setSpeed(int speed) {
    switch_ms = speed;
}

void HapPlayerManager::receiveVideo(string path){
    ofLogVerbose() << "Received : " << path;
    HapPlayerManager::command c;
    c.type = "load";
    c.path = path;
    if (lock()){
        queue.push_front(c);
        unlock();
    }
    else {
        ofLogError() << "x loadVideo: couldn't acquire lock";
    }
}

string HapPlayerManager::getFileName(string s){

    std::size_t botDirPos = s.find_last_of("/");
    std::string file = s.substr(botDirPos+1, s.length());
    return file;
}

void HapPlayerManager::readToPlay(vector<string>toPlay){\
    string toPlayId;
    toPlayVideoIndexes.clear();
    for (int vIndex = 0; vIndex<players.size();vIndex++){
        for (int i = 0; i< toPlay.size(); i++){
            if (strcmp(toPlay[i].c_str(), players[vIndex].videoID.c_str()) ==0){
                toPlayVideoIndexes.push_back(vIndex);
            }
        }
    }
}

int HapPlayerManager::getNextPlayerIndex(){
    if (toPlayVideoIndex < (toPlayVideoIndexes.size() -1)){
        toPlayVideoIndex++;
        return toPlayVideoIndexes[toPlayVideoIndex];

    }
    else if (toPlayVideoIndexes.size() ==0){
        ofLogError(ofToString(ofGetElapsedTimef(),3)) << "novideo files to play";
        return 0;
    }
    else{
        toPlayVideoIndex =0;
        //ofRandomize(toPlayVideoIndexes);
        return toPlayVideoIndexes[0];
    }
}

void HapPlayerManager::_playNextVideoLoaded(){
    int playerIndex =playingVideoIndex;
    int nextIndex = getNextPlayerIndex();

    lastVideoIndex = playingVideoIndex;
    if(!players[lastVideoIndex].video.isPaused()){
       players[lastVideoIndex].status=played;
       players[lastVideoIndex].video.setPaused(true);
       players[lastVideoIndex].video.setPosition(0);
    }
    playingVideoIndex=nextIndex;

    int frame = players[playingVideoIndex].video.getPosition()*players[playingVideoIndex].video.getDuration()/0.033 ;
    samplePlayer.playFile(players[playingVideoIndex].videoID, frame);

    players[playingVideoIndex].video.setVolume(0.0f);
    players[playingVideoIndex].video.setPaused(false);
    players[playingVideoIndex].status = playing;

    std::unique_lock<std::mutex> lock(*playingMutex, std::try_to_lock);
    if(!lock.owns_lock()){
        ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Couldn't update playing video";
        return;
    }
    PlayingInfo pi(players[playingVideoIndex].videoID, players[playingVideoIndex].video.getDuration() -1. );
    playingQueue->push_front(pi);

}

void HapPlayerManager::playNow(string toPlay){
    this->switchTimer =0;
    this->toPlay.clear();
    this->toPlay.push_back(toPlay);

    toPlayVideoIndexes.clear();
    for (int vIndex = 0; vIndex<players.size();vIndex++){
        if (strcmp(toPlay.c_str(), players[vIndex].videoID.c_str()) ==0){
            toPlayVideoIndexes.push_back(vIndex);
        }

    }
    if (toPlayVideoIndexes.size()==0){
        toPlayVideoIndexes.push_back(0);
    }
    this->_playNextVideoLoaded();
}

void HapPlayerManager::toggleOverlay(){
    OVERLAY =!OVERLAY;
}

void HapPlayerManager::playRandom(){
    int index=  int(ofRandom(players.size()));
    toPlayVideoIndexes.push_back(index);
    this->_playNextVideoLoaded();
}

void HapPlayerManager::update(){
    //ofLogError()<<"Call time : " <<ofGetElapsedTimeMillis() - call_time;
    //uint64_t update_time = ofGetElapsedTimeMillis();

    //Loop the video
    if (players[playingVideoIndex].video.getPosition()*players[playingVideoIndex].video.getDuration() >= (players[playingVideoIndex].video.getDuration()-1.)  ){
        playNow(players[playingVideoIndex].videoID);
    }
}
bool HapPlayerManager::draw(int x, int y,int w, int h){

    static bool isDrawing = false;
    int volumeSteps =1;
    ofPushStyle();
    ofSetColor(255);
    players[playingVideoIndex].video.draw(x, y, w,h);
    ofDisableBlendMode();
    ofPopStyle();
    isDrawing = true;

    if (!OVERLAY) return isDrawing;
    ofPushStyle();
    ofSetColor(0, 0, 0, 150);
    ofDrawRectangle(w-w/4-2, 0, w/4+2, h);
    ofNoFill();
    ofSetColor(255, 0, 0);


    float current_pos =  players[playingVideoIndex].video.getPosition()*players[lastVideoIndex].video.getDuration();
    float total_pos =  players[playingVideoIndex].video.getDuration();

    ostringstream os;
    os << "Current video"  << endl;
    os << "Index    : " << playingVideoIndex << endl;
    os << "Load    : " << players[playingVideoIndex].loadTime << endl;
    os << "Time   : " << current_pos << "/" << total_pos << endl;
    os << "ID      : " <<  getFileName(players[playingVideoIndex].videoID) <<endl;
    os << "State   : " << state_string[players[playingVideoIndex].status ]<< endl << "------------------" << endl;


    os << "Last video"  << endl;
    os << "Index    : " << lastVideoIndex << endl;
    os << "ID      : " <<  getFileName(players[lastVideoIndex].videoID)<< endl;
    os << "Load    : " << players[lastVideoIndex].loadTime << endl;
    os << "Time   : " << players[lastVideoIndex].video.getPosition()*players[lastVideoIndex].video.getDuration() << "/" << players[lastVideoIndex].video.getDuration() << endl;
    os << "Playing : " << players[lastVideoIndex].video.isPlaying() << endl;
    os << "State   : " << state_string[ players[lastVideoIndex].status] << endl << "------------------" << endl;

//    for (int i=0; i<MAX_VIDEOS; i++){
//        os << "Player  :"  << i<<endl;
//        os << "State   : " << state_string[ players[i]->status] << endl;
//        os << "Path    : " << players[i]->filePath << endl;
//        os << "ID      : " <<  getFileName(players[i]->videoID) <<endl;
//        os << "Position   : " << players[i]->video.getPosition() << "/" << players[i]->video.getDuration() << endl << "------------------" << endl;

//    }
    ofDrawBitmapString(os.str(), w-w/4+2, 50);
    ofPopStyle();

    return isDrawing;

}

void HapPlayerManager::audioOut(ofSoundBuffer& buffer){

}



void HapPlayerManager::loadAllVideos(ofDirectory dir, int num_videos){
    if (num_videos==-1){
        num_videos = dir.size();
    }

   players.resize(num_videos);
   for (int i = 0; i<num_videos;i++){
       string movieFile = dir.getName(i);

       size_t lastindex = movieFile.find_last_of(".");
       string rawname = movieFile.substr(0, lastindex);
       string fullpath =videoPath + movieFile;
       ofLogNotice(ofToString(ofGetElapsedTimef(),3)) << "[Loading " <<i <<"/"<<num_videos <<"] " << fullpath;

       if (!exists_test(fullpath)){
           ofLogError(ofToString(ofGetElapsedTimef(),3))<< "BROKEN damn FILE - "<< fullpath;
           continue;
       }

       players[i].status  = loading;
       players[i].loadTime = ofGetElapsedTimeMillis();
       players[i].video.setLoopState(OF_LOOP_NORMAL);
       players[i].videoID  = rawname;
       players[i].filePath  = fullpath;

       players[i].video.load(fullpath);
       players[i].loadTime = ofGetElapsedTimeMillis()-players[i].loadTime;
       players[i].video.play();

       ofFbo loadFbo;
       loadFbo.begin();
       while(players[i].video.getPosition()*players[i].video.getDuration() < 0.05 )
       {
           ofEventArgs e;
           players[i].video.update(e);
           players[i].video.draw(0,0);
           ofSleepMillis(10);
//            players[i].video.nextFrame();
       }
       loadFbo.end();
       players[i].video.setPaused(true);
       toPlayVideoIndexes.push_back(i);
   }
}
