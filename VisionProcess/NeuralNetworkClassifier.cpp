#include "NeuralNetworkClassifier.h"

string NeuralNetworkClassifier::predictClass(Mat image)
{
	/*
	Get class string of one prediction
	Returns: strin predClass.
	*/
	// Remove alpha channel and convert to RGB.
	cvtColor(image, imageRGB, COLOR_BGRA2BGR);
	cvtColor(imageRGB, imageNET, COLOR_BGR2RGB);

	// Resize image to feed the network.
	resize(imageNET, resizedImg, Size(inImgH, inImgW));

	// Load model.
	auto network = readNetFromTensorflow(networkDir, networkDirtxt);

	// Make prediction with the image.
	auto blob = blobFromImage(image, 1, Size(inImgH, inImgW));
	network.setInput(blob);

	Mat prediction = network.forward();
	vector<float>vecPrediction(prediction.begin<float>(), prediction.end<float>());

	// prediction class index - OneHot.
	int maxLoc = static_cast<int>(std::distance(vecPrediction.begin(), max_element(vecPrediction.begin(), vecPrediction.end())));
	// Obtain class string with index.
	string predClass = classes[maxLoc];

	return predClass;
}
