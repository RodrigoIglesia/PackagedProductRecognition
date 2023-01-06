#pragma once
#ifndef NEURALNETWORKCLASSIFIER_H
#define NEURALNETWORKCLASSIFIER_H

#include <opencv2/core/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>


using namespace std;
using namespace cv;
using namespace dnn;

class NeuralNetworkClassifier
{
    /*
    Class for Deep Neural Network prediction.
    Load model trainded in Keras with OpenCV.
    */
private:
    // Image to feed the network height and width
    int inImgH = 256, inImgW = 256;
    // Trained model (Keras) paths.
    string networkDir = "product_classifier\\models\\optmized_graph.pb";
    string networkDirtxt = "product_classifier\\models\\optmized_graph.pbtxt";

public:
    // Vector of classes for classification.
    vector<string> classes = { "Class1", "Class2"};
    Mat imageRGB;
    Mat imageNET;
    Mat resizedImg;

    // Methods:
    string predictClass(Mat image);
};

#endif