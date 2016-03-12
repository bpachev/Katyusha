#ifndef reluLayer_h
#define reluLayer_h

public class reluLayer{
public:
  int inputs;
  int outputs;
  float * _biases;
  float * _weights;
  ~reluLayer();
  //return a pointer to the array of activations
  //this pointer is only good untill the next time someone tries to activate the network
  //so copy the data if needed elsewhere 
  float * activate(float * input_arr);
  //weights should be inputs rows, outputs cols 
  void reluLayer(inputs, outputs, float ** weights, float * biases);
  void setParams(float ** weights, float * biases);

private:
  // this is so that we don't have to malloc space to store the output array each time
  float * output_arr;
  
};

#endif
