//version: 0.3
#include <float.h>
#include <hls_stream.h>
#include "pipes.cpp"

typedef unsigned int uint;
typedef hls::stream<float> pipe;

extern "C" {

void conv2d(
    pipe& input_stream,
    pipe& param_stream,
    pipe& output_stream
){
    float input_buffer[3][3][128];
    float weight_buffer[3][3][128][128];//[r][c][ich][och]
    uint roll;
    uint bias0;
    uint bias1;
    uint bias2;
    for(int och=0;och<128;och++){
        for(int c=0;c<3;c++){
            for(int r=0;r<3;r++){
                for(int ich=0;ich<128;ich++){
                    param_stream>>weight_buffer[r][c][ich][och];
                }
            }
        }
    }
    for(int rr=0;rr<HEIGHT;rr++){
        for(int c=0;c<2;c++){
            for(int r=0;r<3;r++){
                for(int ch=0;ch<128;ch++){
                    input_stream>>input_buffer[r][c][ch];
                }
            }
        }
        roll=0;
        for(int cc=0;cc<WIDTH;cc++){
            if(roll==0){
                bias0=0;
                bias1=1;
                bias2=2;
            }
            else if(roll==1){
                bias0=1;
                bias1=2;
                bias2=0;
            }
            else{
                bias0=2;
                bias1=0;
                bias2=1;
            }
            for(int r=0;r<3;r++){
                for(int ich=0;ich<128;ich++){
                    input_stream>>input_buffer[r][bias2][ich];
                }
            }
            for(int och=0;och<128;och++){
                float tmp=0;
                for(int r=0;r<3;r++){
                    for(int ich=0;ich<128;ich++){
                        tmp+=input_buffer[r][bias0][ich]*weight_buffer[r][0][ich][och];
                        tmp+=input_buffer[r][bias1][ich]*weight_buffer[r][1][ich][och];
                        tmp+=input_buffer[r][bias2][ich]*weight_buffer[r][2][ich][och];
                    }
                }
                output_stream<<tmp;
            }

            if(roll==0){
                roll=1;
            }
            else if(roll==1){
                roll=2;
            }
            else{
                roll=0;
            }
        }
    }

}

void relu(
    pipe& input_stream,
    pipe& output_stream
){
    float tmp;
    for(uint i=0;i<WIDTH*HEIGHT*128;i++){
        input_stream>>tmp;
        output_stream<<((tmp>0)?tmp:0);
    }
}

void add(
    pipe& input1_stream,
    pipe& input2_stream,
    pipe& output_stream
){
    float input1,input2;
    for(uint i=0;i<WIDTH*HEIGHT*128;i++){
        input1_stream>>input1;
        input2_stream>>input2;
        output_stream<<(input1+input2);
    }
}

void ImageStyleTransform(
    const float *input,
    const float *param,
    float *output,
    int size_input,
    int size_param,
    int size_output
){
    #pragma HLS INTERFACE m_axi port=input  offset=slave bundle=gmem
    #pragma HLS INTERFACE m_axi port=param  offset=slave bundle=gmem
    #pragma HLS INTERFACE m_axi port=output offset=slave bundle=gmem
    #pragma HLS INTERFACE s_axilite port=input  bundle=control
    #pragma HLS INTERFACE s_axilite port=param  bundle=control
    #pragma HLS INTERFACE s_axilite port=output bundle=control
    #pragma HLS INTERFACE s_axilite port=size_input bundle=control
    #pragma HLS INTERFACE s_axilite port=size_param bundle=control
    #pragma HLS INTERFACE s_axilite port=size_output bundle=control
    #pragma HLS INTERFACE s_axilite port=return bundle=control

    pipe stream_input_conv2d_0("0");
    pipe stream_param_conv2d_0("1");
    pipe stream_output_conv2d_0("2");
    pipe stream_input_relu("3");
    pipe stream_output_relu("4");
    pipe stream_input_conv2d_1("5");
    pipe stream_param_conv2d_1("6");
    pipe stream_output_conv2d_1("7");
    pipe stream_input0_add("8");
    pipe stream_input1_add("9");
    pipe stream_output_add("10");

    pipe_input_conv2d_0(input,stream_input_conv2d_0);
    pipe_param_conv2d_0(param,stream_param_conv2d_0);
    conv2d(stream_input_conv2d_0,stream_param_conv2d_0,stream_output_conv2d_0);
    pipe_conv2d_0_relu(stream_output_conv2d_0,stream_input_relu);
    relu(stream_input_relu,stream_output_relu);
    pipe_param_conv2d_1(param,stream_param_conv2d_1);
    pipe_relu_conv2d_1(stream_output_relu,stream_input_conv2d_1);
    conv2d(stream_input_conv2d_1,stream_param_conv2d_1,stream_output_conv2d_1);
    pipe_input_add(input,stream_input0_add);
    pipe_conv2d_1_add(stream_output_conv2d_1,stream_input1_add);
    add(stream_input0_add,stream_input1_add,stream_output_add);
    pipe_add_output(stream_output_add,output);

}
}//end extern "C"
