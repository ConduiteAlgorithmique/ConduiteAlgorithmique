#ifndef OFXTWOCHANNELMIXER_H
#define OFXTWOCHANNELMIXER_H

#pragma once

#include "ofMain.h"
#include "ofxSoundObject.h"
#include "ofxSoundPlayerObject.h"

/**
 * This class represents a simple mixer which adds together the output
 * of multiple chains of ofxSoundObjects
 */
class ofxTwoChannelFader: public ofxSoundObject {
public:
    ofxTwoChannelFader();
    virtual ~ofxTwoChannelFader();

     ofxSoundObject* getChannelSource(int channelNumber);
    int getNumChannels();

    /// sets output volume multiplier.
    /// a volume of 1 means "full volume", 0 is muted.
    void setMasterVolume(float vol);
    float getMasterVolume();



    void audioOut(ofSoundBuffer &output);
    bool isConnectedTo(ofxSoundPlayerObject& obj);

    void setChannelVolume(int channelNumber, float vol);
    float getChannelVolume(int channelNumber);

    ofParameter<float> masterVol;
    void  fadeTo(ofxSoundPlayerObject *obj);
    void create_fades(int buffer_size, int num_channels);
    void getAudio(ofSoundBuffer &input);


protected:
    void setInput(ofxSoundObject *obj);
    void disconnectInput(ofxSoundPlayerObject * input);
    void masterVolChanged(float& f);
    void copyLiveAudio(ofSoundBuffer &output);

    vector<ofxSoundPlayerObject*>channels;
    vector<float> channelVolume;
    vector<float> fadeIn;
    vector<float> fadeOut;
    vector<float> fadeInSolo;


    float masterVolume;
    ofMutex mutex;

    bool fade;
    bool same_input =true;
    bool same_same =false;
    int current_channel;
    int buffersize;

    ofSoundBuffer audioCopyBuffer;
    ofMutex copyLock;



};


#endif // OFXTWOCHANNELMIXER_H
