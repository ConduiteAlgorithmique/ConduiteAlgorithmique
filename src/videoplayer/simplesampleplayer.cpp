#include "simplesampleplayer.h"
#include "ofxJsonSettings.h"

SimpleSamplePlayer::SimpleSamplePlayer()
{
    stream.listDevices();
    stream.setDeviceID( Settings::getInt("audio_device_id")); //Is computer-specific
    int num_channels = 2;
    int bufferlength = 1024;
    stream.setup(num_channels  , 0, 48000, bufferlength, 2);
    stream.setOutput(fader);
    ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Sample player audio init"<<endl;
    fader.create_fades(bufferlength,num_channels);
    current_index =0;
}

void SimpleSamplePlayer::init(vector<string> audioFilePaths, int num_files = -1){
    if (num_files == -1){
        num_files = audioFilePaths.size();
    }

    players.resize(num_files);
    playerFileIndexes.resize(num_files);

    for (int i = 0; i<num_files;i++){

        string path = audioFilePaths[i];
        ofLogNotice() << "Opening audio file " << (i+1)<<"/" <<num_files<<" - " <<path<<endl;
        players[i].load(path);
//      players[i].setMultiPlay(true);
        players[i].setPosition(0.);
        players[i].setPaused(true);
        players[i].setVolume(1.);
        playerFileIndexes[i] = ofFile(path).getBaseName(); // filename without extension
    }
}

ptrdiff_t  SimpleSamplePlayer::getSampleIndexFromName(string name){
    ptrdiff_t pos = find(playerFileIndexes.begin(), playerFileIndexes.end(), name) - playerFileIndexes.begin();
    if(pos >= playerFileIndexes.size()) {
      ofLogError(ofToString(ofGetElapsedTimef(),3)) << "Audio file " << name << " not found"<<endl;
      return 0;
    }
    return pos;
}

void SimpleSamplePlayer::playFromIndex(int i){
       players[i].connectTo(fader);
}

//The fader controls the playback
void SimpleSamplePlayer::playFile(string sample_name, int start_frame){
    ptrdiff_t index= getSampleIndexFromName(sample_name);
    players[index].connectTo(fader);
}

//The fader controls the playback
void SimpleSamplePlayer::loop(int index){
    players[index].setPosition(0);
}


