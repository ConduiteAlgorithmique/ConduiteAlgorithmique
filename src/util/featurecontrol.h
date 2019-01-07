#ifndef FEATURECONTROL_H
#define FEATURECONTROL_H

#include "ofMain.h"
#include "util/featuresearch.h"
#include "util/databaseloader.h"
#include "util/communication.h"
#include "gui/elements/uielements.h"
#include "gui/pointcloudrenderer.h"


const std::map<string, int> featureIndexMap{
    {"loudness", 0},
    {"pitch", 1},
    {"percussiveness", 2},
    {"spec_band", 3},
    {"speed", 4},
    {"instability", 5},
    {"rpm", 6},
    {"temperature", 7},
    {"roll", 8},
    {"hue", 9},
    {"lightness", 10},
    {"uncertainty", 11},
    {"building", 12},
    {"pavement", 13},
    {"road", 14},
    {"sky", 15},
    {"trees", 16},
    {"vehicle", 17},
    {"signage", 18},
    {"fence_pole", 19},
    {"bike_ped", 20},
};

const int IDLE_ACTIVE_TRANSITION = 0;
const int IDLE_ACTIVE_STABLE =  1;
const int COLOR_FEATURE_INDEX = 9;
const int TILT_FEATURE_INDEX = 8;
//const int NUM_SPEEDS= 14;
const int COLOR_ELEMENT_INDEX = 11;

class FeatureControl
{
public:
    FeatureControl(DatabaseLoader *, CommunicationManager*, vector<unique_ptr<CircleFeatureGuiElement>>*, PointCloudRenderer*);

    string stateStrings [3]= {"IDLE", "IDLE_ACTIVE", "HUMAN_ACTIVE"};
    enum BehaviourState{
        IDLE,
        IDLE_ACTIVE,
        HUMAN_ACTIVE,
    };

    string activityTypeStrings [5]= {"NONE","ASCENDING", "UP_DOWN", "DESCENDING", "DOWN_UP"};
    enum class ActivityType{
        NONE,
        ASCENDING,
        UP_DOWN,

        first = NONE,
        last = UP_DOWN
        //        DESCENDING,
        //        DOWN_UP,
    };

    string activitymodifierStrings [4]= {"NONE", "ACCELERATE", "DECELERATE", "RANDOM_TIMINGS"};
    enum class ActivityModifier{
        NONE,
        ACCELERATE,
        DECELERATE,
//        RANDOM_TIMINGS,
        first = NONE,
        last = DECELERATE
    };

    int secondsToFrames = 60.;
    vector<int> idleFeatureIndexes;
    vector<int> idleActiveFeatureIndexes;

    BehaviourState state;
    ActivityType activityType;
    FeatureSearch* fSearch;
    DatabaseLoader* dbl;
    PointCloudRenderer* pcr;
    CommunicationManager* coms;
    vector<unique_ptr<CircleFeatureGuiElement>>* fge;


    void initialize(DatabaseLoader * dbl);
    void update();

    void incrementFeatureTarget(int index, float step);
    void toggleFeatureTarget(int index);
    void updateActiveFeature(int index,int timeoutOffset, bool trigger);
    bool shouldSlowdown();
    void registerInputActivity();
    void draw();
    void playRandomVideo();
    void incrementSearchRadius(int step);
    void incrementSpeed(int step);
    void setSpeed(int speed);

    vector<float> getValues(){
        return featureValues;
    }

    vector<float> getWeights(){
        return featureWeights;
    }
    vector<float> getTargetValues(){
        return targetFeatureValues;
    }

    float getWeight(int index){
        return featureWeights[index];
    }
    float getValue(int index){
        return featureValues[index];
    }
    float getLastValue(int index){
        return lastFeatureValues[index];
    }
    float getTargetValue(int index){
        return targetFeatureValues[index];
    }

    float getActive(int index){
        return featureActive[index];
    }

    bool weightChanged(int index);

    bool weightsChanged();
    int numFeatures(){
        return featureValues.size();
    }

    void updateFeatureElements();

     pair<string, int> getPlayingVideo(){
        return playingVideo;
    }
    vector<int> getVideoIndexes(){
        vector<int> t;
        for (int i=0;i<videoMaxIndex; i++){
            t.push_back(videoIndexes[i]);
        }
        return t;
    }

    void toggleNeighbours();
    void toggleSpeed();
    void setSearchRadius(int);

    //State functions
    void updateState();
    void toIdle();
    void toIdleActive();
    void toHumanActive();

    static const int NUM_SPEEDS = 16;
    const string SPEEDSTRINGS [17]= {"Max", "4", "3", "2", "1.5", "1", "2/3", "1/2","1/3","1/4","1/6", "1/8", "1/10", "1/12", "1/16", "1/24", "1/32"};
    const float SPEEDS [17]=        {-1,     4.,   3., 2., 1.5,   1., 2./3.,  0.5,  1./3., 1./4., 1./6., 1./8., 1./10., 1./12, 1./16, 1./24., 1./32.};


private:

    //Update feature functions
    void setIdleFeature(int index);
    void updateFeatureWeights(bool ignoreIdle);
    void updateFeatureValues(vector<float> fv);
    void timeoutOtherFeatures(int index);

    //Light functions
    void updateLights();
    void blinkOn();
    void blinkOff();
    void updateDurationLight();

    float lastLightUpdateTime;
    bool lightsUpdated;

    //Video search/playback functions
    void getNewVideos(bool play= true);
    void playVideo();
    void cycleVideo();

    //Timeout parameters
    float featureTimeout;
    float featureDecayRate;
    float idleActivityInterval;
    float idleActivityTransitionDuration;
    float idleTimeout;

    //Idle activity variables
    float idle_activity_min_duration;
    float idle_activity_max_duration;
    int idleActivityNumUpdates;
    deque<float> idleActivityValues;
    deque<float> idleActivityTimings;
    ActivityModifier activityModifier;

    //Timers
    float lastActivityTime;
    float idleTimeCounter;
    float idleActivatedTime;
    float videoCycleTimer;
    float videoStartTime;

    float slowdownTime;
    float currentTime;
    float lastActiveCycle;

    float blinkTimer;
    bool blink;
    int idleMinVideos;
    int playedIdleVideos;
    int numVideosInRange = 0;

    //Feature indexes
    int currentActiveFeatureIndex;
    int activeFeatureIndex;

    bool input_activity_flag;

    vector<bool> featureActive;
    vector<int> inactiveCounter;
    vector<int> videoIndexes;

    vector<float> targetFeatureValues;
    vector<float> featureValues;
    vector<float> lastFeatureValues;

    vector<float> featureWeights;
    vector<float> lastFeatureWeights;

    vector<float> searchDistances;

    vector<pair<string, int>> videos;
    pair<string, int> playingVideo;

    int videoCycleIndex;
    int videoMaxIndex;
    int speedSetting;
    float videoLength;
    int idle_active_state;



};

#endif // FEATURECONTROL_H
