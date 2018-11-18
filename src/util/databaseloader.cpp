#include "databaseloader.h"

DatabaseLoader::DatabaseLoader()
{

}

void DatabaseLoader::loadHDF5Data(string database_path){
//    ofSetLogLevel(OF_LOG_VERBOSE);
    hdf5File.open(database_path, true);
    cout << "File '" << database_path << "' has " << hdf5File.getNumGroups() << " groups and " << hdf5File.getNumDataSets() << " datasets" << endl;
    for (int i = 0; i < hdf5File.getNumGroups(); ++i) {
        cout << "  Group " << i << ": " << hdf5File.getGroupName(i) << endl;
    }
    for (int i = 0; i < hdf5File.getNumDataSets(); ++i) {
        cout << "  DataSet " << i << ": " << hdf5File.getDataSetName(i) << endl;
    }

    feature_values_ptr= hdf5File.loadDataSet("feature_values");
    size_t  data_size = feature_values_ptr->getDataSize();

    num_videos = feature_values_ptr->getDimensionSize(0);
    num_features = feature_values_ptr->getDimensionSize(1);
    auto values= new float[num_videos*num_features];
    feature_values_ptr->read(values);
    for( int row =0; row<num_videos; row++){
        auto value_row = new float[num_features];

        std::copy(values +row*num_features,values +(row+1)*num_features,value_row );
        vector<float> v(value_row, value_row +num_features );
        feature_values.push_back(v);
    }


    colors_ptr= hdf5File.loadDataSet("colors");
    int num_colors = colors_ptr->getDimensionSize(0);
    int color_dim = colors_ptr->getDimensionSize(1);
    auto color_values= new uint8_t[num_colors*color_dim];
    colors_ptr->read(color_values);
    for( int row =0; row<num_colors; row++){
        uint8_t h,s,v;
        h = int(color_values[row*color_dim]/180. *255);
        s = color_values[row*color_dim+1];
        v = color_values[row*color_dim+2];
        ofColor c = c.fromHsb(h,s,v);
        colors.push_back(c);
    }

    coordinates_ptr= hdf5File.loadDataSet("coordinates");
    int num_coords = coordinates_ptr->getDimensionSize(0);
    int coord_dim = coordinates_ptr->getDimensionSize(1);
    auto coordinateValues= new float[num_coords*coord_dim];
    coordinates_ptr->read(coordinateValues);
    for( int row =0; row<num_coords; row++){
        ofVec2f coords;
        coords.x = coordinateValues[row*coord_dim];
        coords.y = coordinateValues[row*coord_dim+1];
        coordinates.push_back(coords);
    }

    ofxHDF5DataSetPtr data_ptr= hdf5File.loadDataSet("color_limits");
    int num_values = data_ptr->getDimensionSize(0);
    auto color_extrema= new float[num_values];
    data_ptr->read(color_extrema);
    color_min_max.first = color_extrema[0];
    color_min_max.second = color_extrema[1];


    ofxHDF5DataSetPtr dr_ptr= hdf5File.loadDataSet("dimension_reduction");
    int num_points = dr_ptr->getDimensionSize(0);
    int points_dim = dr_ptr->getDimensionSize(1);
    auto dimValues= new float[num_points*points_dim];
    dr_ptr->read(dimValues);
    for( int row =0; row<num_points; row++){
        ofVec3f p;
        p.x = dimValues[row*points_dim] -0.5;
        p.y = dimValues[row*points_dim+1]-0.5;
        p.z = dimValues[row*points_dim+2]-0.5;

        dimension_reduction.push_back(p);
    }

    dr_ptr= hdf5File.loadDataSet("dimension_reduction_2D");
    num_points = dr_ptr->getDimensionSize(0);
    points_dim = dr_ptr->getDimensionSize(1);
    dimValues= new float[num_points*points_dim];
    dr_ptr->read(dimValues);
    for( int row =0; row<num_points; row++){
        ofVec3f p;
        p.x = dimValues[row*points_dim] -0.5;
        p.y = dimValues[row*points_dim+1]-0.5;
        p.z = 0.;

        dimension_reduction_2D.push_back(p);
    }

    feature_names_ptr= hdf5File.loadDataSet("feature_names");
    size_t  name_size = feature_names_ptr->getDataSize();
    int num_names = feature_names_ptr->getDimensionSize(0);

    auto characters = new uint8_t[num_names*STRING_LENGTH];
    feature_names_ptr->read(characters);
    auto name = new char[STRING_LENGTH];

    for( int row =0; row<num_names; row++){
        std::copy(characters +row*STRING_LENGTH,characters +(row+1)*STRING_LENGTH,name );
        feature_names.push_back(name);
    }

    video_names_ptr= hdf5File.loadDataSet("video_names");
    num_videos = video_names_ptr->getDimensionSize(0);

    characters = new uint8_t[num_videos*STRING_LENGTH];
    video_names_ptr->read(characters);
    for( int row =0; row<num_videos; row++){
        std::copy(characters +row*STRING_LENGTH,characters +(row+1)*STRING_LENGTH,name );
        video_names.push_back(name);
    }

    ofxHDF5DataSetPtr duration_ptr= hdf5File.loadDataSet("video_durations");
    num_videos = duration_ptr->getDimensionSize(0);
    auto durationValues= new float[num_videos];
    duration_ptr->read(durationValues);
    for( int row =0; row<num_videos; row++){
        video_durations.push_back(durationValues[row]);
    }
    hdf5File.close();
}

const vector<vector<float>>* DatabaseLoader::getFeatures(){
    return &feature_values;
}

const vector<string>  DatabaseLoader::getFeatureNames(){
    return feature_names;
}

vector<string>   DatabaseLoader::getVideoNames(){
    return video_names;
}

vector<string>   DatabaseLoader::getVideoNamesFromIndexes(vector<int> indexes){
    vector<string>  names;
    for (std::size_t i = 0; i < indexes.size(); i++)
    {
        names.push_back(video_names[indexes[i]]);
    }
    return names;
}

vector<pair<string, int>> DatabaseLoader::getVideoPairsFromIndexes(vector<int> indexes){
    vector<string>  names = getVideoNamesFromIndexes(indexes);
    vector<pair<string, int>>  pairs;
    for (std::size_t i = 0; i < indexes.size(); i++)
    {
        pairs.push_back(pair<string, int>(names[i], indexes[i]));
    }
    return pairs;
}

vector<float>  DatabaseLoader::getFeaturesFromName(string name){
    return feature_values[getVideoIndexFromName(name)];
}

vector<float>  DatabaseLoader::getFeaturesFromindex(int index){
    return feature_values[index];
}

int DatabaseLoader::getVideoIndexFromName(string name){
    auto it = std::find(video_names.begin(), video_names.end(), name);
    if (it == video_names.end())
    {
        ofLogError() <<"Received fault video name: "<< name<<endl;
        return 0;
    } else
    {
      auto index = std::distance(video_names.begin(), it);
      return index;
    }
}

vector<ofVec2f> DatabaseLoader::getCoordinates(){
    return this->coordinates;
}

float DatabaseLoader::getVideoLength(int index){
    return this->video_durations[index];
}


void  DatabaseLoader::getSaturationBrightness(int index, float &s, float &b){
    float h;
    colors[index].getHsb(h,s,b);
    s = s/255.;
    b = b/255.;
    return;
}
