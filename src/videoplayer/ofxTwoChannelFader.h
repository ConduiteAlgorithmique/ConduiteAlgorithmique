#ifndef OFXTWOCHANNELMIXER_H
#define OFXTWOCHANNELMIXER_H

#pragma once

#include "ofMain.h"
#include "ofxSoundObject.h"
#include "ofxSoundPlayerObject.h"
#include "gui/audiowaveform.h"
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


protected:
    void setInput(ofxSoundObject *obj);
    void masterVolChanged(float& f);
    void disconnectInput(ofxSoundPlayerObject * input);
    vector<ofxSoundPlayerObject*>channels;
    vector<float> channelVolume;
    vector<float> fadeIn;
    vector<float> fadeOut;

    float masterVolume;
    ofMutex mutex;

    bool fade;
    bool same_input =true;
    int current_channel;

};


#endif // OFXTWOCHANNELMIXER_H
