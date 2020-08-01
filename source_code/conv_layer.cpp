//version 0.3

#include <conv_layer.h>
using namespace std;

vector <vector<vector<float>>> padding(vector <vector<vector<float>>>x, int kernel) {

	int length = x.size();
	int width = x[0].size();
	int ch = x[0][0].size();
	int pad_len = kernel / 2;
	vector <vector<vector<float>>> bak_x(length + 2 * pad_len, vector<vector<float>>(width + 2 * pad_len, vector<float>(ch, 0)));
	for (int i = 0; i < length; i++) {
		for (int j = 0; j < width; j++) {
			for (int c = 0; c < ch; c++) {
				bak_x[i + pad_len][j + pad_len][c] = x[i][j][c];
			}

		}

	}
	for (int c = 0; c < ch; c++) {
		for (int i = 0; i < length; i++) {
			for (int p = 0; p < pad_len; p++) {
				bak_x[i + pad_len][p][c] = x[i][pad_len - p][c];
				bak_x[i + pad_len][p + pad_len + width][c] = x[i][width - 2 - p][c];

			}
		}
		for (int p = 0; p < pad_len; p++) {
			for (int j = 0; j < width + 2 * pad_len; j++) {
				bak_x[p][j][c] = bak_x[pad_len + pad_len - p][j][c];
				bak_x[p + length + pad_len][j][c] = bak_x[pad_len + length - p - 2][j][c];
			}

		}
	}
	return bak_x;
}

vector <vector<vector<float>>> conv_2d(vector <vector<vector<float>>>x, vector <vector<vector<vector<float>>>>w, int input_filters, int output_filters, int kernel, int strides) {

	vector <vector<vector<float>>> new_x = padding(x, kernel);
	int pad_len = kernel / 2;
	int R = (x.size() + pad_len * 2 - kernel) / strides + 1;
	int C = (x[0].size() + pad_len * 2 - kernel) / strides + 1;

	vector <vector<vector<float>>> out_x(R, vector<vector<float>>(C, vector<float>(output_filters, 0)));
	for (int kr = 0; kr < kernel; kr++) {
		for (int kc = 0; kc < kernel; kc++) {
			for (int r = 0; r < R; r++) {
				for (int c = 0; c < C; c++) {
					for (int cho = 0; cho < output_filters; cho++) {
						for (int chi = 0; chi < input_filters; chi++) {
							out_x[r][c][cho] += new_x[r * strides + kr][c * strides + kc][chi] * w[kr][kc][chi][cho];
						}
					}
				}
			}
		}
	}
	return out_x;
}


vector<float> mymean(vector<vector<vector<float>>> x) {
	int R = x.size();
	int C = x[0].size();
	int CH = x[0][0].size();
	vector<float> sum(CH, 0);

	for (int ch = 0; ch < CH; ch++) {
		for (int r = 0; r < R; r++) {
			for (int c = 0; c < C; c++) {
				sum[ch] += x[r][c][ch];
			}
		}
		sum[ch] = sum[ch] / (R * C);
	}
	return sum;
}

vector<double > mystd(vector<vector<vector<float>>> x, vector<float> mean_x) {
	int CH = x[0][0].size();
	int R = x.size();
	int C = x[0].size();
	float epsilon = 1e-9;
	vector<double > std_x(CH, 0);

	for (int ch = 0; ch < CH; ch++) {
		for (int r = 0; r < R; r++) {
			for (int c = 0; c < C; c++) {
				std_x[ch] += pow((x[r][c][ch] - mean_x[ch]), 2);
			}
		}
		std_x[ch] = sqrt(std_x[ch] / (R * C)+epsilon);
	}
	return std_x;
}

vector<vector<vector<float>>> norm(vector<vector<vector<float>>> x) {
	vector<float> mean_x = mymean(x);
	vector<double> std_x = mystd(x, mean_x);
	int CH = x[0][0].size();
	int R = x.size();
	int C = x[0].size();
	vector<vector<vector<float>>> out_x(R, vector<vector<float>>(C, vector<float>(CH, 0)));
	for (int ch = 0; ch < CH; ch++) {
		for (int r = 0; r < R; r++) {
			for (int c = 0; c < C; c++) {
				out_x[r][c][ch] = (x[r][c][ch] - mean_x[ch]) / std_x[ch];
			}
		}
	}
	return out_x;
}

vector<vector<vector<float>>> relu(vector<vector<vector<float>>> x) {
	int CH = x[0][0].size();
	int R = x.size();
	int C = x[0].size();
	vector<vector<vector<float>>> out_x(R, vector<vector<float>>(C, vector<float>(CH, 0)));
	for (int ch = 0; ch < CH; ch++) {
		for (int r = 0; r < R; r++) {
			for (int c = 0; c < C; c++) {
				if (x[r][c][ch] > 0)
				{
					out_x[r][c][ch] = x[r][c][ch];
				}
				else {
					out_x[r][c][ch] = 0;
				}
			}
		}
	}
	return out_x;
}

vector <vector<vector<float>>> cov_layer(vector<vector<vector<float>>> x, vector <vector<vector<vector<float>>>>w, int input_filters, int output_filters, int kernel, int strides) {
	return relu(norm(conv_2d(x, w, input_filters, output_filters, kernel, strides)));
}

