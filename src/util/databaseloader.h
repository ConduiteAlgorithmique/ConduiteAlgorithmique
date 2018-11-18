#ifndef DATABASELOADER_H
#define DATABASELOADER_H

#include "ofMain.h"
#include "ofxHDF5.h"

#define STRING_LENGTH 40

class DatabaseLoader
{
public:
    DatabaseLoader();
    void loadHDF5Data(string);
    const vector<vector<float>>* getFeatures();
    int getNumFeatures(){return num_features;}

    pair<string, int> getRandomVideo(){
        int randIndex = (int) ofRandom(0,video_names.size());
        return pair<string, int> (video_names[randIndex], randIndex);
    }

    const vector<string>  getFeatureNames();

    vector<string>  getVideoNames();
    vector<string>  getVideoNamesFromIndexes(vector<int> index);

    vector<ofVec2f> getCoordinates();

    vector<float> getFeaturesFromName(string name);
    vector<float> getFeaturesFromindex(int index);
    int getVideoIndexFromName(string name);
    float getVideoLength(int index);
    void getSaturationBrightness(int index, float &, float &);
    vector<pair<string, int>> getVideoPairsFromIndexes(vector<int> indexes);

    vector<string> feature_names;
    vector<string> video_names;
    vector<float>  video_durations;

    vector<ofColor> colors;
    vector<vector<float> > feature_values;
    vector<ofVec2f> coordinates;
    vector<ofVec3f> dimension_reduction;
    vector<ofVec3f> dimension_reduction_2D;
    pair<float, float> color_min_max;



private:
    ofxHDF5DataSetPtr feature_values_ptr;
    ofxHDF5DataSetPtr feature_names_ptr;
    ofxHDF5DataSetPtr video_names_ptr;
    ofxHDF5DataSetPtr colors_ptr;
    ofxHDF5DataSetPtr coordinates_ptr;


    int num_videos;
    int num_features;
    ofxHDF5File hdf5File;



};

#endif // DATABASELOADER_H
