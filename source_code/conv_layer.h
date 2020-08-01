#include <string.h>
#include <vector>
#include <iostream>
#include <string>
#include <math.h>
#include <fstream>

vector <vector<vector<float>>> padding(vector <vector<vector<float>>>x, int kernel);
vector <vector<vector<float>>> conv_2d(vector <vector<vector<float>>>x, vector <vector<vector<vector<float>>>>w, int input_filters, int output_filters, int kernel, int strides);
vector<float> mymean(vector<vector<vector<float>>> x) 
vector<double > mystd(vector<vector<vector<float>>> x, vector<float> mean_x)
vector<vector<vector<float>>> norm(vector<vector<vector<float>>> x)
vector<vector<vector<float>>> relu(vector<vector<vector<float>>> x)
vector <vector<vector<float>>> cov_layer(vector<vector<vector<float>>> x, vector <vector<vector<vector<float>>>>w, int input_filters, int output_filters, int kernel, int strides