//version: 0.3
#include <float.h>
#include <hls_stream.h>
#include "ImageStyleTransform.hpp"

typedef unsigned int uint;
typedef hls::stream<float> pipe;

#define WIDTH 245
#define HEIGHT 155

extern "C"{

void pipe_input_conv2d_0(const float* in,pipe& out){
    //in: input
    //out: stream_input_conv2d_0
    //sequence:
    //[0][3][6]
    //[1][4][7]
    //[2][5][8]

    //[0][0][0]
    //[0][?][?]
    //[0][?][?]
    for(int i=0;i<128*4;i++){
        out<<0;
    }
    for(int i=0;i<128;i++){
        out<<in[i];
    }
    for(int i=0;i<128;i++){
        out<<in[WIDTH*128+i];
    }
    for(int i=0;i<128;i++){
        out<<0;
    }
    for(int i=0;i<128;i++){
        out<<in[i+128];
    }
    for(int i=0;i<128;i++){
        out<<in[WIDTH*128+128+i];
    }
    //(0)(0)[0]
    //(?)(?)[?]
    //(?)(?)[?]
    for(int c=2;c<WIDTH;c++){
        for(int i=0;i<128;i++){
            out<<0;
        }
        for(int i=0;i<128;i++){
            out<<in[c*128+i];
        }
        for(int i=0;i<128;i++){
            out<<in[(c+WIDTH)*128+i];
        }
    }
    //(0)(0)[0]
    //(?)(?)[0]
    //(?)(?)[0]
    for(int i=0;i<128;i++){
        out<<0;
    }
    for(int r=1;r<HEIGHT-1;r++){
        //[0][?][?]
        //[0][?][?]
        //[0][?][?]
        for(int i=0;i<3*128;i++){
            out<<0;
        }
        for(int i=0;i<128;i++){
            out<<in[(r-1)*WIDTH*128];
        }
        for(int i=0;i<128;i++){
            out<<in[r*WIDTH*128];
        }
        for(int i=0;i<128;i++){
            out<<in[(r+1)*WIDTH*128];
        }
        for(int i=0;i<128;i++){
            out<<in[(r-1)*WIDTH*128+128];
        }
        for(int i=0;i<128;i++){
            out<<in[r*WIDTH*128+128];
        }
        for(int i=0;i<128;i++){
            out<<in[(r+1)*WIDTH*128+128];
        }
        //(?)(?)[?]
        //(?)(?)[?]
        //(?)(?)[?]
        for(int c=2;c<WIDTH;c++){
            for(int i=0;i<128;i++){
                out<<in[(r-1)*WIDTH*128+c*128];
            }
            for(int i=0;i<128;i++){
                out<<in[r*WIDTH*128+c*128];
            }
            for(int i=0;i<128;i++){
                out<<in[(r+1)*WIDTH*128+c*128];
            }
        }
        //(?)(?)[0]
        //(?)(?)[0]
        //(?)(?)[0]
        for(int i=0;i<3*128;i++){
            out<<0;
        }
    }

    //[0][?][?]
    //[0][?][?]
    //[0][0][0]
    for(int i=0;i<128*3;i++){
        out<<0;
    }
    for(int i=0;i<128;i++){
        out<<in[(HEIGHT-2)*WIDTH*128+i];
    }
    for(int i=0;i<128;i++){
        out<<in[(HEIGHT-1)*WIDTH*128+i];
    }
    for(int i=0;i<128;i++){
        out<<0;
    }
    for(int i=0;i<128;i++){
        out<<in[((HEIGHT-2)*WIDTH+1)*128+i];
    }
    for(int i=0;i<128;i++){
        out<<in[((HEIGHT-1)*WIDTH+1)*128+i];
    }
    //(?)(?)[?]
    //(?)(?)[?]
    //(0)(0)[0]
    for(int c=2;c<WIDTH;c++){
        for(int i=0;i<128;i++){
            out<<in[((HEIGHT-2)*WIDTH+c)*128+i];
        }
        for(int i=0;i<128;i++){
            out<<in[((HEIGHT-1)*WIDTH+c)*128+i];
        }
        for(int i=0;i<128;i++){
            out<<0;
        }
    }
    //(?)(?)[0]
    //(?)(?)[0]
    //(0)(0)[0]
    for(int i=0;i<3*128;i++){
        out<<0;
    }
}
void pipe_param_conv2d_0(const float* in,pipe& out){
    //in: param
    //out: stream_param_conv2d_0
    for(int och=0;och<128;och++){
        for(int c=0;c<3;c++){
            for(int r=0;r<3;r++){
                for(int ich=0;ich<128;ich++){
                    out<<in[((r*3+c)*128+ich)*128+och];
                }
            }
        }
    }
}

void pipe_conv2d_0_relu(pipe& in,pipe& out){
    //in: stream_output_conv2d_0
    //out: stream_input_relu
    float tmp;
    for(uint i=0;i<HEIGHT*WIDTH*128;i++){
        in >> tmp;
        out<< tmp;
    }
}

void pipe_param_conv2d_1(const float* in,pipe& out){
    //in: param
    //out: stream_param_conv2d_1
    for(int och=0;och<128;och++){
        for(int c=0;c<3;c++){
            for(int r=0;r<3;r++){
                for(int ich=0;ich<128;ich++){
                    out<<in[((r*3+c)*128+ich)*128+och+3*3*128*128];
                }
            }
        }
    }
}
void pipe_relu_conv2d_1(pipe& in,pipe& out){
    //in: stream_output_relu
    //out: stream_input_conv2d_1
    float buffer[3][WIDTH][128];
    uint roll;
    for(int c=0;c<WIDTH;c++){
        for(int i=0;i<128;i++){
            buffer[0][c][i]=0;
        }
    }
    for(int r=1;r<2;r++){
        for(int c=0;c<WIDTH;c++){
            for(int i=0;i<128;i++){
                in>>buffer[r][c][i];
            }
        }
    }

    roll=0;
    uint r0;
    uint r1;
    uint r2;
    for(int j=0;j<HEIGHT;j++){
        if(roll==0){
            r0=0;
            r1=1;
            r2=2;
        }
        else if(roll==1){
            r0=1;
            r1=2;
            r2=0;
        }
        else{
            r0=2;
            r1=0;
            r2=1;
        }
        for(int c=0;c<WIDTH;c++){
            for(int i=0;i<128;i++){
                in>>buffer[r2][c][i];
            }
        }
        for(int i=0;i<3*128;i++){
            out<<0;
        }
        for(int c=0;c<WIDTH;c++){
            for(int i=0;i<128;i++){
                out<<buffer[r0][c][i];
            }
            for(int i=0;i<128;i++){
                out<<buffer[r1][c][i];
            }
            for(int i=0;i<128;i++){
                out<<buffer[r2][c][i];
            }
        }
        for(int i=0;i<3*128;i++){
            out<<0;
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
    for(int i=0;i<3*128;i++){
        out<<0;
    }
    for(int c=0;c<WIDTH;c++){
        for(int i=0;i<128;i++){
            out<<buffer[r1][c][i];
        }
        for(int i=0;i<128;i++){
            out<<buffer[r2][c][i];
        }
        for(int i=0;i<128;i++){
            out<<0;
        }
    }
    for(int i=0;i<3*128;i++){
        out<<0;
    }
}

void pipe_input_add(const float* in,pipe& out){
    //in: input
    //out: stream_input0_add
    for(uint i=0;i<HEIGHT*WIDTH*128;i++){
        out<<in[i];
    }
}
void pipe_conv2d_1_add(pipe& in,pipe& out){
    //in: stream_output_conv2d_1
    //out: stream_input1_add
    float tmp;
    for(uint i=0;i<HEIGHT*WIDTH*128;i++){
        in>>tmp;
        out<<tmp;
    }
}

void pipe_add_output(pipe& in,float* out){
    //in: stream_output_add
    //out: output
    for(uint i=0;i<HEIGHT*WIDTH*128;i++){
        in>>out[i];
    }
}

}