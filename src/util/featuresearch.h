#ifndef FEATURESEARCH_H
#define FEATURESEARCH_H
#include "ofMain.h"
#include "util/databaseloader.h"
#include <random>

class FeatureSearch
{
public:
    struct VideoItem{
        VideoItem(string s, int i){
            videoName = s;
            videoIndex = i;
        }
        string videoName;
        int videoIndex;
    };

    FeatureSearch(DatabaseLoader*);
    bool compareVideoItems(const VideoItem& first, const VideoItem& second);
    float getDistance(const VideoItem);
    vector<int> getSearchResultsDistance(int, bool, int & );
    void getKNN(vector<float> , vector<float>,  vector<float> &);
    float getColorDistance(int);
    float getColorDistance2(int);


    vector<float> searchDistances;
    vector<VideoItem> videos;
    vector<int> nearest_videos;
    int COLOR_FEATURE_INDEX = 9;
    int numSearchPoints = 64;
    float threshold_distance = 0.05;
    std::default_random_engine rng;

    vector<float> targetFeatures;
    vector<float> featureWeights;

    DatabaseLoader*dbl;

};

#endif // FEATURESEARCH_H
