#include "featurecontrol.h"
#include "ofxJsonSettings.h"

template <typename E>
constexpr typename std::underlying_type<E>::type to_ut(E e) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(e);
}
void readSettingsFeatures(string t, vector<vector<int>>& indexes){
    std::stringstream ss(t);
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> vstrings(begin, end);

    for(auto fname: vstrings){

        map<string, int>::const_iterator it = featureIndexMap.find(fname);
        if(it != featureIndexMap.end())
        {
            vector<int> tv;
            tv.push_back(it->second);
            indexes.push_back(tv);
        }
        else{
            ofLogError() << "ERROR IN SETTINGS - WRONG IDLE FEATURE NAME : "<<fname<<endl;
            ofExit();
        }
    }
}

template<typename StringFunction>
void splitString(const std::string &str, char delimiter, StringFunction f) {
  std::size_t from = 0;
  for (std::size_t i = 0; i < str.size(); ++i) {
    if (str[i] == delimiter) {
      f(str, from, i);
      from = i + 1;
    }
  }
  if (from <= str.size())
    f(str, from, str.size());
}
void readSettingsFeaturePairs(string t, vector<vector<int>>& indexes){
    std::stringstream ss(t);
    vector<string> groups;
    string group;
    while(std::getline(ss, group, ',')){
        groups.push_back(group);
    }

    for(auto group: groups){
        std::stringstream gss(group);
        std::istream_iterator<std::string> begin(gss);
        std::istream_iterator<std::string> end;
        std::vector<std::string> vstrings(begin, end);
        vector<int> tv;

        for (auto fname: vstrings){
            map<string, int>::const_iterator it = featureIndexMap.find(fname);
            if(it != featureIndexMap.end())
            {
                tv.push_back(it->second);
            }
            else{
                ofLogError() << "ERROR IN SETTINGS - WRONG IDLE FEATURE NAME : "<<fname<<endl;
                ofExit();
            }
        }
        indexes.push_back(tv);
    }
}


FeatureControl::FeatureControl(DatabaseLoader *dbl, CommunicationManager  *coms,vector<unique_ptr<CircleFeatureGuiElement>> *guiElements, PointCloudRenderer* pcr)
{
    this->dbl = dbl;
    this->coms = coms;
    this->fge = guiElements;
    this->pcr = pcr;
    this->fSearch = new FeatureSearch(dbl);
    Settings::get().load("settings.json");
    idleTimeout =  Settings::getInt("idle_timeout"); //seconds
    idleMinVideos=  Settings::getInt("idle_videos"); //seconds
    featureTimeout = Settings::getFloat("feature_timeout");

    featureDecayRate = 1./(60.*Settings::getFloat("feature_decay_sec"));
    idleActivityInterval = Settings::getFloat("idle_activity_min_interval"); //seconds
    idleActivityNumUpdates= Settings::getInt("idle_activity_num_updates"); //seconds

    idle_activity_min_duration = Settings::getFloat("idle_activity_min_duration"); //seconds
    idle_activity_max_duration = Settings::getFloat("idle_activity_max_duration"); //seconds

    auto t = Settings::getString("idle_features");
    readSettingsFeatures(t, idleFeatureIndexes);
    t = Settings::getString("idle_active_features");
    readSettingsFeatures(t, idleActiveFeatureIndexes);

    t = Settings::getString("idle_feature_pairs");
    readSettingsFeaturePairs(t, idleFeatureIndexes);

    t = Settings::getString("idle_active_feature_pairs");
    readSettingsFeaturePairs(t, idleActiveFeatureIndexes);

    state = IDLE;
    int num_features =21;

    //Initialize weight to zero
    featureValues.resize(num_features);
    lastFeatureValues.resize(num_features);
    featureWeights = vector<float>(num_features, 0.);
    inactiveCounter.resize(num_features,0);
    targetFeatureValues.resize(num_features, 0.);
    featureActive.resize(num_features, false);
    lastFeatureWeights.resize(num_features, 0.);
    videoCycleIndex = 0;
    blink =false;
    lastLightUpdateTime = 0.;
    lightsUpdated = true;

    //Make loudness the only active feature
    featureWeights[0]= 1.;
    playingVideo = pair<string, int>("Empty", -1);
    videoMaxIndex = 1;
    activityType = ActivityType::NONE;
    activityModifier = ActivityModifier::NONE;
    input_activity_flag = 0;
    blinkTimer =0;
    setSpeed(0);
    toIdle();
    updateDurationLight();

}

void FeatureControl::update(){
    if (input_activity_flag){
        lastActivityTime = ofGetElapsedTimef();
    }

    currentTime = ofGetElapsedTimef();
    idleTimeCounter = currentTime - lastActivityTime;

    if ((currentTime -videoStartTime) > videoCycleTimer){
        cycleVideo();
    }

    if ((currentTime - blinkTimer) > 0.05 && blink){
        blinkOff();
    }

    updateState();

    switch (state){
    case HUMAN_ACTIVE:
        //Todo
        updateFeatureWeights();
        break;

    case IDLE:
        updateFeatureWeights();
        if (weightsChanged()){
            getNewVideos(false);
        }
        break;

    case IDLE_ACTIVE:
        updateFeatureWeights();
        float idleTime = currentTime -idleActivatedTime;
        if (idle_active_state == IDLE_ACTIVE_TRANSITION){
            lastActiveCycle = currentTime;

            videoCycleTimer = idleActivityTimings[videoCycleIndex];
            videoLength = dbl->getVideoLength(playingVideo.second)-1;
            (*fge)[0]->setValue(videoCycleTimer);


            if (videoCycleIndex >= (videoIndexes.size()-1)){
                idle_active_state = IDLE_ACTIVE_STABLE;
                setSpeed(0);
            }
        }
        else if ( idle_active_state == IDLE_ACTIVE_STABLE){
            float vidlen = dbl->getVideoLength(playingVideo.second)-1;
            (*fge)[0]->setValue(vidlen);
            if ((currentTime - lastActiveCycle)>= vidlen ){
                toIdle();
            }
        }
        break;
    }
    if (weightsChanged()){
        updateLights();
    }
    input_activity_flag = false;
    lastFeatureWeights =featureWeights;
}


void FeatureControl::updateState(){
    switch (state){
    case HUMAN_ACTIVE:
        if (idleTimeCounter> idleTimeout){
            toIdle();
            break;
        }
        break;
    case IDLE:
        if (input_activity_flag){
            toHumanActive();
            break;
        }
        if (idleTimeCounter > idleActivityInterval && playedIdleVideos >= idleMinVideos){
            toIdleActive();
            break;
        }
        break;
    case IDLE_ACTIVE:
        if (input_activity_flag){
            activityType = ActivityType::NONE;
            toHumanActive();
            break;
        }
        break;
    }
}

void FeatureControl::toIdle(){
    idleMinVideos=  Settings::getInt("idle_videos"); //number of videos to play in idle mode
    playedIdleVideos =0;
    lastActivityTime = ofGetElapsedTimef();
    slowdownTime = ofGetElapsedTimef();
    setIdleFeature(idleFeatureIndexes[rand()%idleFeatureIndexes.size()]);
    state= IDLE;
}

void FeatureControl::toIdleActive(){
    state = IDLE_ACTIVE;
    idle_active_state = IDLE_ACTIVE_TRANSITION;
    setSpeed(0);
    activityType = ActivityType(rand()% (to_ut(ActivityType::last)) +1);
    activityModifier = ActivityModifier(rand()% (to_ut(ActivityModifier::last)+1));

    float activityDuration = ofRandom(idle_activity_min_duration, idle_activity_max_duration);

    currentIdleActiveFeatureIndexes =idleActiveFeatureIndexes[rand()%idleActiveFeatureIndexes.size()];

    for(auto idx: currentIdleActiveFeatureIndexes){
        targetFeatureValues[idx] = 0.;
        updateActiveFeature(idx, 0., false);
    }
    timeoutOtherFeatures(currentIdleActiveFeatureIndexes);


    idleActivityValues.clear();
    idleActivityTimings.clear();
    int num_steps =idleActivityNumUpdates*activityDuration;

    if (activityType == ActivityType::UP_DOWN){
        num_steps = num_steps;
    }

    float linear_time_step = 1./num_steps;
    float time =0.;

    for (int step =0; step <= num_steps; step++){
        idleActivityValues.push_back(step*1./(num_steps));

        if (activityModifier == ActivityModifier::ACCELERATE){
             idleActivityTimings.push_back(sqrt(time)*activityDuration);
        }
        else if (activityModifier == ActivityModifier::DECELERATE){
            idleActivityTimings.push_back(pow(time,2)*activityDuration);
        }
        else{
            //NONE
            idleActivityTimings.push_back(time*activityDuration);
        }
        time +=linear_time_step;
    }

    for (int step =0; step <= num_steps-1; step++){
        idleActivityTimings[step] = idleActivityTimings[step+1] -idleActivityTimings[step];
    }

    if (activityType == ActivityType::UP_DOWN){
        vector<float> rev (idleActivityValues.begin(),idleActivityValues.end());
        std::reverse(rev.begin(), rev.end());
        rev.erase(rev.begin());
        idleActivityValues.insert(idleActivityValues.end(), rev.begin(), rev.end());

        vector<float> rev_times (idleActivityTimings.begin(),idleActivityTimings.end());
        std::reverse(rev_times.begin(), rev_times.end());
        rev_times.erase(rev_times.begin());
        idleActivityTimings.insert(idleActivityTimings.end(), rev_times.begin(), rev_times.end());
    }

    lastActivityTime = ofGetElapsedTimef();
    idleActivatedTime = ofGetElapsedTimef();

    //Create a set of videos along the feature value trajectory
    videoIndexes.clear();
    videoCycleIndex =0;

    vector<float> tempSearchvalues = targetFeatureValues;
    vector<float> tempFeatureWeights;
    tempFeatureWeights.resize(tempSearchvalues.size(), 0);
    for(auto idx: currentIdleActiveFeatureIndexes){
        tempFeatureWeights[idx]=1.;
    }

    for (auto value: idleActivityValues){
        for(auto idx: currentIdleActiveFeatureIndexes){
            tempSearchvalues[idx] = value;
        }
        fSearch->getKNN(tempSearchvalues, tempFeatureWeights, searchDistances);
        vector<int> results =fSearch->getSearchResultsDistance(32,true, numVideosInRange);
        int index = results[0];
        int i =0;
        bool already_in_list =std::find(videoIndexes.begin(), videoIndexes.end(), index) != videoIndexes.end();
        while (already_in_list && i<results.size()){
            i++;
            index= results[i];
            already_in_list=std::find(videoIndexes.begin(), videoIndexes.end(), index) != videoIndexes.end();
        }
        videoIndexes.push_back(index);
    }

    idleActivityTimings[num_steps] = dbl->getVideoLength(videoIndexes[num_steps])-1;

    videos =  dbl->getVideoPairsFromIndexes(videoIndexes);
    videoMaxIndex = videos.size();
    (*fge)[1]->setValue(32);
    updateDurationLight();
}

void FeatureControl::toHumanActive(){
    state = HUMAN_ACTIVE;
    lastActivityTime = ofGetElapsedTimef();
}

void FeatureControl::setIdleFeature(vector<int> indexes){
    currentIdleActiveFeatureIndexes = indexes;
    for(auto idx: currentIdleActiveFeatureIndexes){
        targetFeatureValues[idx] = 0.;
        updateActiveFeature(idx, 1., false);
    }
    getNewVideos();
}

void FeatureControl::getNewVideos(bool play){

    if (state == HUMAN_ACTIVE){
        //dont do so often
    }
    fSearch->getKNN(targetFeatureValues, featureWeights, searchDistances);
    videoIndexes = fSearch->getSearchResultsDistance(32,true, numVideosInRange);

    if (state ==IDLE || state ==IDLE_ACTIVE){
        videoMaxIndex = CLAMP(videoIndexes.size(), 1, 32);
    }

    (*fge)[1]->setValue(videoMaxIndex);

    videos = dbl->getVideoPairsFromIndexes(videoIndexes);

    if (playingVideo.second != videos[0].second && play){
        videoCycleIndex =0;
        playingVideo =videos[0];
        playVideo();
    }
}

void FeatureControl::playVideo(){
    //Todo: Let main loop know
    videoLength = dbl->getVideoLength(playingVideo.second)-1;

    if (speedSetting == 0){
        videoCycleTimer = videoLength;
    }

    (*fge)[0]->reset();
    (*fge)[0]->setValue(videoCycleTimer);
    (*fge)[COLOR_ELEMENT_INDEX]->setWeightColor(dbl->colors[playingVideo.second]);
    videoStartTime = ofGetElapsedTimef();
    coms->publishVideoNow( playingVideo.first, true);
    updateFeatureValues(dbl->getFeaturesFromindex(playingVideo.second));
    blinkOn();

    pcr->updateLine(playingVideo.second);
    pcr->updateLine(videos[(videoCycleIndex+1)%videoMaxIndex].second);

}

void FeatureControl::cycleVideo(){
    videoCycleIndex = (videoCycleIndex+1)%videoMaxIndex;
    playingVideo = videos[videoCycleIndex];
    playedIdleVideos ++;
    if (state ==IDLE_ACTIVE){
        for(auto idx: currentIdleActiveFeatureIndexes){
            targetFeatureValues[idx] = idleActivityValues[videoCycleIndex];
        }
    }
    if (shouldSlowdown()){
        incrementSpeed(-1);
        idleMinVideos+=1;
    }
    playVideo();
}

void FeatureControl::playRandomVideo(){
    playingVideo= this->dbl->getRandomVideo();
    playVideo();
}

void FeatureControl::updateFeatureWeights(){
    for (int i = 0; i <featureWeights.size(); i++){
        //Ignore all active boyos in idle states
        if (state==IDLE_ACTIVE ||state ==IDLE){
            vector<int> v = currentIdleActiveFeatureIndexes;
            if (std::find(v.begin(), v.end(), i) != v.end()) continue;
        }
        //only ignore currentActiveFeatureIndex when human control
        else{
            if (i ==currentActiveFeatureIndex){
                continue;
            }
        }

        if (featureWeights[i] >0.){
            inactiveCounter[i]+=1;
        }

        //After [idleTimeout] seconds of inactivity decrement the weight by 0.1 every 0.5 seconds
        if (inactiveCounter[i]>(featureTimeout*secondsToFrames) && featureWeights[i] >0){
            featureWeights[i] = CLAMP(featureWeights[i] - featureDecayRate, 0., 1.);
        }

        else if (inactiveCounter[i]>featureTimeout*secondsToFrames && featureWeights[i] ==0.){
            featureActive[i]=false;
        }
    }
}

void FeatureControl::updateFeatureValues(vector<float> fv){
    lastFeatureValues = featureValues;
    featureValues = fv;

    for (int i =0; i <featureValues.size(); i++){
        if (!featureActive[i]){
            targetFeatureValues[i] = featureValues[i];
        }
    }
}

void FeatureControl::incrementFeatureTarget(int index, float step){
    currentActiveFeatureIndex = index;
    if (index ==COLOR_FEATURE_INDEX) {
        float t;
        float t2 = targetFeatureValues[index];
        targetFeatureValues[index] = modf(t2+step, &t );
        if (targetFeatureValues[index] <0.){
            targetFeatureValues[index] = 1+targetFeatureValues[index];
        }
    }
    else if (index ==TILT_FEATURE_INDEX) {
        targetFeatureValues[index] = CLAMP(targetFeatureValues[index]+step, -0.5, 0.5);
    }
    else{
        targetFeatureValues[index] =CLAMP(targetFeatureValues[index]+step, 0., 1.);

    }
    updateActiveFeature(index, 1, true);
}

void FeatureControl::toggleFeatureTarget(int index){
    float v = targetFeatureValues[index];
    currentActiveFeatureIndex = index;

    if (index ==COLOR_FEATURE_INDEX){
        float t;
        float target_value = modf(v +0.5, &t);
        targetFeatureValues[index] =target_value;
    }
    else if (index ==TILT_FEATURE_INDEX){
        float target_value = -0.5;
        if (v<.5) target_value = 0.5;
        targetFeatureValues[index] =CLAMP(target_value, -0.5, 0.5);
    }
    else{
        float target_value = 0.;
        if (v<.5) target_value = 1.;
        targetFeatureValues[index] =CLAMP(target_value, 0., 1.);
    }

    updateActiveFeature(index, 1, true);
}

void FeatureControl::updateActiveFeature(int index, int timeoutOffset, bool trigger){
    //Set light to max
    if(featureWeights[index]<1.){
        coms->sendLightControl(index+3, 4096);
    }
    featureActive[index]=true;
    featureWeights[index]=1.0;
    inactiveCounter[index] = 0;

    if (trigger){
        getNewVideos();
    }
}

void FeatureControl::timeoutOtherFeatures(vector<int> indexes){
    //Timeout the other features
    for(int i=0; i<this->inactiveCounter.size(); i++){
        if (std::find(indexes.begin(), indexes.end(), i) != indexes.end()) continue;
        //Set other active feautres to timeout limit - offset
        else if (featureActive[i]){
            inactiveCounter[i] = featureTimeout*secondsToFrames ;
        }
    }
}



void FeatureControl::registerInputActivity(){
    input_activity_flag = true;
}

bool FeatureControl::shouldSlowdown(){
    if (state ==IDLE){
        if (speedSetting >0){
            return true;
        }
    }
    return false;
}

void FeatureControl::incrementSearchRadius(int step){
    setSearchRadius(videoMaxIndex +step);
}

void FeatureControl::setSearchRadius(int value){
     int t =  CLAMP(value, 1, videos.size());
     t =  CLAMP(t, 1, 32);
     videoMaxIndex =t;
     (*fge)[1]->setValue(videoMaxIndex);
     updateDurationLight();
 }

void FeatureControl::incrementSpeed(int step){
    setSpeed( CLAMP(speedSetting+step, 0, NUM_SPEEDS));
}

void FeatureControl::toggleSpeed(){
    if (speedSetting < NUM_SPEEDS){
        setSpeed(NUM_SPEEDS);
    }
    else if (speedSetting == NUM_SPEEDS){
        setSpeed(0);
    }
}

void FeatureControl::toggleNeighbours(){
    if (videoMaxIndex > 16){
        setSearchRadius(1);
    }
    else {
        setSearchRadius(32);
    }
}

void FeatureControl::setSpeed(int value){
    speedSetting = value;
    videoCycleTimer = SPEEDS[speedSetting];
    videoLength = dbl->getVideoLength(playingVideo.second)-1;
    if (speedSetting == 0){
        videoCycleTimer = videoLength;
    }

    (*fge)[0]->setValue(videoCycleTimer);
    (*fge)[0]->setSecondary(speedSetting);
}


void FeatureControl::updateLights(){
        for (int index = 0 ; index < featureValues.size(); index ++){
            if (weightChanged(index)){
                int lightValue = 4095*0.75 * featureWeights[index];
                coms->sendLightControl(index +3, lightValue);
            }
        }

        lastLightUpdateTime = currentTime;
        lightsUpdated =true;
}

void FeatureControl::updateDurationLight(){
    float v = 32. -CLAMP(videoMaxIndex, 1, 32);
    coms->sendLightControl(2, v/32. *4095);
}


void FeatureControl::blinkOn(){
    coms->sendLightControl(1, 4095);
    blink = true;
    blinkTimer= currentTime;
}

void FeatureControl::blinkOff(){
    coms->sendLightControl(1, 0);
    blink = false;
}

template <typename T>
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

void FeatureControl::updateFeatureElements(){
    for (int i = 0; i <numFeatures(); i++){
        //Do some interpolation on the value
        float diff = featureValues[i] - lastFeatureValues[i];
        float inc = sgn(diff)*diff*diff;

        //Special  case for colours
        if (i == COLOR_FEATURE_INDEX){
            inc = diff;
        }

        lastFeatureValues[i]+=inc;

        (*fge)[i+2]->setWeight(getWeight(i));
        (*fge)[i+2]->setValue(getLastValue(i));
        (*fge)[i+2]->setTarget(getTargetValue(i));
        (*fge)[i+2]->setActive(getActive(i));
    }
}

void FeatureControl::draw(){
    ofPushMatrix();
    ostringstream oss;
    for (int i = 0; i<videoMaxIndex; i++){
        oss<< searchDistances[i] <<endl;
    }
    ofDrawBitmapString(oss.str(),0,0);

    oss.str("");
    for (int i = 0; i<videoMaxIndex; i++){
        oss<< videoIndexes[i] <<endl;
    }
    ofTranslate(100, 0);
    ofDrawBitmapString(oss.str(),0,0);

    oss.str("");
    oss << "State" <<endl;
    oss << stateStrings[state]<<endl;
    oss << activityTypeStrings[to_ut(activityType)]<<endl;
    oss << activitymodifierStrings[to_ut(activityModifier)]<<endl;

    ofTranslate(30, 0);
    ofDrawBitmapString(oss.str(), 0,0);

    oss.str("");
    oss << "" <<endl;
    oss << "Idle timer"<<endl;
    oss << "Last Act."<<endl;
    oss << "Act. index" <<endl;
    oss << "Idle Index" <<endl;
    oss << "Video name"<<endl;
    oss << "Video index" <<endl;
    oss << "Video time" <<endl;
    oss << "Video length" <<endl;
    oss << "Cycle index"<<endl;
    oss << "Cycle time" <<endl;
    oss << "Idle played" <<endl;
    oss << "Idle min vid" <<endl;
    oss << "Videos found" <<endl;
    oss << "Vid. max ind." <<endl;
    ofTranslate(100, 0);
    ofDrawBitmapString(oss.str(), 0,0);

    oss.str("");
    oss << "Tnfo" <<endl;
    oss << idleTimeCounter <<endl;
    oss << lastActivityTime<<endl;
    oss << currentActiveFeatureIndex <<endl;
    for (auto afi:currentIdleActiveFeatureIndexes){
        oss << afi << " ";
    }
    oss <<endl;
    oss << playingVideo.first <<endl;
    oss << playingVideo.second <<endl;
    oss << currentTime -videoStartTime <<endl;
    oss << videoLength <<endl;
    oss << videoCycleIndex <<endl;
    oss << videoCycleTimer <<endl;

    oss << playedIdleVideos <<endl;
    oss << idleMinVideos <<endl;
    oss<< numVideosInRange<<endl;
    oss<< videoMaxIndex<<endl;


    ofTranslate(100, 0);
    ofDrawBitmapString(oss.str(), 0,0);

    oss.str("");
    oss << "Target" <<endl;
    for (auto featureTarget : targetFeatureValues){
        oss << setprecision(2) << featureTarget <<endl;
    }
    ofTranslate(150, 0);
    ofDrawBitmapString(oss.str(), 0,0);

    oss.str("");
    oss << "Value" <<endl;
    for (auto featureV: featureValues){
        oss << setprecision(2) << featureV <<endl;
    }

    ofTranslate(50, 0);
    ofDrawBitmapString(oss.str(), 0,0);

    oss.str("");
    oss << "Weight" <<endl;
    for (auto featureW: featureWeights){
        oss << setprecision(2) <<featureW <<endl;
    }
    ofTranslate(50, 0);
    ofDrawBitmapString(oss.str(), 0,0);

    oss.str("");
    oss << "Active" <<endl;
    for (auto featureA: featureActive){
        oss << featureA <<endl;
    }
    ofTranslate(50, 0);
    ofDrawBitmapString(oss.str(), 0,0);

    ofPopMatrix();
}

bool FeatureControl::weightChanged(int index){
    if (lastFeatureWeights[index]!= featureWeights[index]){
        return true;
    }
    return false;
}

bool FeatureControl::weightsChanged(){
    for (int i=0; i <featureWeights.size();i++){
        if (lastFeatureWeights[i]!= featureWeights[i]){
            return true;
        }
    }
    return false;
}
