#include "host.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
using namespace std;

int main(int argc, char** argv)
{

    cout<<"Program: ImageStyleTransform"<<endl;

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <XCLBIN File>" << std::endl;
		return EXIT_FAILURE;
	}

    std::string binaryFile = argv[1];
    
    cl_int err;
    unsigned fileBufSize;
    // Allocate Memory in Host Memory
    const int size_input=(1*155*245*128);
    const int size_output=(1*155*245*128);
    const int size_param=(3*3*128*128+3*3*128*128);//weight + bias
    std::vector<float,aligned_allocator<float>> source_input(size_input);
    std::vector<float,aligned_allocator<float>> source_param(size_param);
    const size_t vector_size_bytes_input = sizeof(float) * size_input;
    const size_t vector_size_bytes_param = sizeof(float) * size_param;
    std::vector<float,aligned_allocator<float>> source_hw_results(size_output);
    std::vector<float,aligned_allocator<float>> source_sw_results(size_output);
    const size_t vector_size_bytes_result = sizeof(float) * size_output;

    // const string cwd("D:/Workspace/Xilinx-Summer-Camp.prj/HLS");
    const string cwd("/home/ailab/zzy_remote_workspace/ImageStyleTransform/src");
    // Create the test data --------------------------------------
    // create input data-----------------------
    ifstream input_file;
    input_file.open(cwd+"/data/interdata/wave/conv3_output.record",ios::in);
    //WARNING: if this file does not exist, VITIS will NOT report an error!
    //So you might never find it......
    string s;
    for(int i=0;i<size_input;i++){
        getline(input_file,s);
        source_input[i]=atof(s.data());
    }
    input_file.close();
    // create param data-----------------------
    int offset;
    offset=0;
    int pace;
    ifstream param_file_res1_conv0_weights;
    param_file_res1_conv0_weights.open(cwd+"/data/weights/wave/res1_conv0_weights.dump",ios::in);
    pace=3*3*128*128;
    for(int i=0;i<pace;i++){
        getline(param_file_res1_conv0_weights,s);
        source_param[offset+i]=atof(s.data());
    }
    offset+=pace;
    ifstream param_file_res1_conv1_weights;
    param_file_res1_conv1_weights.open(cwd+"/data/weights/wave/res1_conv1_weights.dump",ios::in);
    pace=3*3*128*128;
    for(int i=0;i<pace;i++){
        getline(param_file_res1_conv1_weights,s);
        source_param[offset+i]=atof(s.data());
    }
    offset+=pace;
    // create output verification data-----------------------
    // ifstream result_file;
    // result_file.open(cwd+"fc3_output.record",ios::in);
    // for(int i=0;i<size_output;i++){
    //     getline(result_file,s);
    //     source_sw_results[i]=atof(s.data());
    // }
    // result_file.close();
    ifstream result_file;
    result_file.open(cwd+"/data/interdata/wave/res1_output.record",ios::in);
    for(int i=0;i<size_output;i++){
        getline(result_file,s);
        if(i<10){
            cout<<"debug: "<<s<<endl;
        }
        source_sw_results[i]=atof(s.data());
    }
    result_file.close();


    // OPENCL HOST CODE AREA START
    std::vector<cl::Device> devices = get_devices("Xilinx");
    devices.resize(1);
    cl::Device device = devices[0];
    OCL_CHECK(err, cl::Context context(device, NULL, NULL, NULL, &err));
    OCL_CHECK(err, cl::CommandQueue q(context, device, CL_QUEUE_PROFILING_ENABLE, &err));	
    char* fileBuf = read_binary_file(binaryFile, fileBufSize);
    cl::Program::Binaries bins{{fileBuf, fileBufSize}};	
    OCL_CHECK(err, cl::Program program(context, devices, bins, NULL, &err));
    OCL_CHECK(err, cl::Kernel krnl_ImageStyleTransform(program,"ImageStyleTransform", &err));
    OCL_CHECK(err, cl::Buffer buffer_input   (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, 
            vector_size_bytes_input, source_input.data(), &err));
    OCL_CHECK(err, cl::Buffer buffer_param   (context,CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY, 
            vector_size_bytes_param, source_param.data(), &err));
    OCL_CHECK(err, cl::Buffer buffer_output(context,CL_MEM_USE_HOST_PTR | CL_MEM_WRITE_ONLY, 
            vector_size_bytes_result, source_hw_results.data(), &err));
    OCL_CHECK(err, err = krnl_ImageStyleTransform.setArg(0, buffer_input));
    OCL_CHECK(err, err = krnl_ImageStyleTransform.setArg(1, buffer_param));
    OCL_CHECK(err, err = krnl_ImageStyleTransform.setArg(2, buffer_output));
    OCL_CHECK(err, err = krnl_ImageStyleTransform.setArg(3, size_input));
    OCL_CHECK(err, err = krnl_ImageStyleTransform.setArg(4, size_param));
    OCL_CHECK(err, err = krnl_ImageStyleTransform.setArg(5, size_output));
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_input,buffer_param},0/* 0 means from host*/));	
    OCL_CHECK(err, err = q.enqueueTask(krnl_ImageStyleTransform));
    OCL_CHECK(err, err = q.enqueueMigrateMemObjects({buffer_output},CL_MIGRATE_MEM_OBJECT_HOST));

    q.finish();
	
// OPENCL HOST CODE AREA END

    // Compare the results of the Device to the simulation
    int mis_match = 0;
    // float diff;
    // const float tolerance=0.001;
    for (int i = 0 ; i < size_output ; i++){
        // diff=source_hw_results[i] - source_sw_results[i];
        // if (diff>tolerance or diff<-tolerance){
        //     std::cout << "Error: Result mismatch" << std::endl;
        //     std::cout << "i = " << i << " CPU result = " << source_sw_results[i]
        //         << " Device result = " << source_hw_results[i] << std::endl;
        //     mis_match+=1;
        // }
        // else{
        //     std::cout << "i = " << i << " CPU result = " << source_sw_results[i]
        //         << " Device result = " << source_hw_results[i] << std::endl;
        // }
        if(source_sw_results[i]!=source_hw_results[i]){
            mis_match+=1;
        }
    }
    for (int i = 0 ; i < min(min(mis_match,size_output),100) ; i++){
        // diff=source_hw_results[i] - source_sw_results[i];
        // if (diff>tolerance or diff<-tolerance){
        //     std::cout << "Error: Result mismatch" << std::endl;
        //     std::cout << "i = " << i << " CPU result = " << source_sw_results[i]
        //         << " Device result = " << source_hw_results[i] << std::endl;
        //     mis_match+=1;
        // }
        // else{
        //     std::cout << "i = " << i << " CPU result = " << source_sw_results[i]
        //         << " Device result = " << source_hw_results[i] << std::endl;
        // }
        if(source_sw_results[i]!=source_hw_results[i]){
            std::cout<<"ERROR: Result mismatch"<<std::endl;
            mis_match+=1;
        }
        std::cout << "i = " << i << " CPU result = " << source_sw_results[i]
            << " Device result = " << source_hw_results[i] << std::endl;
    }
    std::cout << "mis_match : " << mis_match << std::endl;
    std::cout << "TEST " << (mis_match==0 ? "PASSED" : "FAILED") << std::endl; 
    std::cout << "INPUT_SIZE " << source_input.size() << std::endl;
    std::cout << "PARAM_SIZE " << source_param.size() << std::endl;
    std::cout << "OUTPUT_SIZE " << source_hw_results.size() << std::endl;
    std::cout << "version : 0.2" << std::endl;
	
// Release Allocated Resources
    delete[] fileBuf;

    return (mis_match==0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

// version: conv1, relu1, pooling1
// waiting for test