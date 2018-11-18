
#ifndef HAPPLAYERMANAGER_H
#define HAPPLAYERMANAGER_H

#include "ofMain.h"
#include "ofxHapPlayer.h"
#include "simplesampleplayer.h"
#include "videodatabaseloader.h"
#include "ofApp.h"
class HapPlayerManager:public ofThread{

public:
    struct PlayingInfo{
        PlayingInfo(string s, float d){
            durationS = d;
            fileName =s;
            isLoop = false;
        }
        bool isLoop;
        float durationS;
        string fileName;
    };

    HapPlayerManager(deque<PlayingInfo>*, ofMutex*, VideoDatabaseLoader* );
    ~HapPlayerManager();
    void receiveVideo(string path);
    void setToPlay(vector<string> toPlay);
    void readToPlay(vector<string>toPlay);

    void setSpeed(int speed);
    void loadAllVideos(ofDirectory dir, int n);
    void loadVideoPaths(vector<string> paths, int n);
    void update();
    void playRandom();

    bool draw(int x, int y, int w, int h);

    void audioOut(ofSoundBuffer& buffer);
    float getWidth() {return players[playingVideoIndex].video.isLoaded() ? players[playingVideoIndex].video.getWidth() : 0;}
    float getHeight() {return players[playingVideoIndex].video.isLoaded() ? players[playingVideoIndex].video.getHeight() : 0;}
    void toggleOverlay();
    void playNow(string toPlay);
    SimpleSamplePlayer * getSamplePlayer();


    int playingVideoIndex;
    int lastVideoIndex;



private:
    /* Fades */

    const string state_string[7] = {"empty", "loading", "priming", "ready", "playing", "played", };

    enum PStatus { empty, loading,priming, ready, playing, played};

    struct player {
        player(){

        }

        player(const player& p){
            fade = p.fade;
            loadTime = p.loadTime;
            status = p.status;
            videoID = p.videoID;
            filePath = p.filePath;
        }


        ofxHapPlayer  video;
        bool          fade;
        int           loadTime;
        PStatus       status;
        string        videoID;
        string        filePath;

    };

    struct command {
        string type;
        string path;
        bool fade;
    };
    deque<PlayingInfo>* playingQueue;
    ofMutex* playingMutex;

    deque<command> queue;
    vector<string> toPlay;

    vector<player> players;
    vector<int> toPlayVideoIndexes;
    int toPlayVideoIndex =0;

    SimpleSamplePlayer samplePlayer;

    string videoPath;
    uint64_t switchTimer;
    int switch_ms=330;

    int loadIndex;
    uint64_t call_time;
    int internal_counter = 0;

    bool OVERLAY =false;
    bool PLAYERS_INITIALIZED = false;
    bool DEV_MODE = false;

    bool alreadyLoaded(string _path);
    void _playNextVideo();
    string getFileName(string );
    bool loadVideo(string _path);

    int getNextPlayerFromIndex(int playerIndex);
    int getFreePlayerFromIndex(int playerIndex);
    void emptyOldVideos(vector<string> toPlay);
    void setAllVolumes(float);
    int getNextPlayerIndex();
    void addVideoPlayer(string _path, bool load_async );
    void _playNextVideoLoaded();





};

#endif // OFXTHREADEDVIDEOPLAYERMANAGER_H
