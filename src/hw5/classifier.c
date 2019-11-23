#include <math.h>
#include <stdlib.h>
#include "image.h"
#include "matrix.h"

// Run an activation function on each element in a matrix,
// modifies the matrix in place
// matrix m: Input to activation function
// ACTIVATION a: function to run
void activate_matrix(matrix m, ACTIVATION a)
{
    int i, j;
    for (i = 0; i < m.rows; ++i) {
        double sum = 0;
        for (j = 0; j < m.cols; ++j) {
            double x = m.data[i][j];
            if (a == LOGISTIC) {
                m.data[i][j] = 1 / (1 + exp(-x));
            } else if (a == RELU) {
                m.data[i][j] = (x > 0) ? x : 0;
            } else if (a == LRELU) {
                m.data[i][j] = x * ((x > 0) ? 1 : 0.1);
            } else if (a == SOFTMAX) {
                m.data[i][j] = exp(x);
            }
            sum += m.data[i][j];
        }

        if (a == SOFTMAX) {
            for (j = 0; j < m.cols; ++j) {
                m.data[i][j] /= sum;
            }
        }
    }
}

// Calculates the gradient of an activation function and multiplies it into
// the delta for a layer
// matrix m: an activated layer output
// ACTIVATION a: activation function for a layer
// matrix d: delta before activation gradient
void gradient_matrix(matrix m, ACTIVATION a, matrix d)
{
    int i, j;
    for(i = 0; i < m.rows; ++i){
        for(j = 0; j < m.cols; ++j){
            double x = m.data[i][j];
            if (a == LOGISTIC) {
                d.data[i][j] *= x * (1 - x);
            } else if (a == RELU) {
                d.data[i][j] = (x > 0) ? d.data[i][j] : 0;
            } else if (a == LRELU) {
                d.data[i][j] = d.data[i][j] * ((x > 0) ? 1 : 0.1);
            }
        }
    }
}

// Forward propagate information through a layer
// layer *l: pointer to the layer
// matrix in: input to layer
// returns: matrix that is output of the layer
matrix forward_layer(layer *l, matrix in)
{
    l->in = in;  // Save the input for backpropagation

    matrix out = matrix_mult_matrix(in, l->w);
    activate_matrix(out, l->activation);
    free_matrix(l->out);// free the old output
    l->out = out;       // Save the current output for gradient calculation

    return out;
}

// Backward propagate derivatives through a layer
// layer *l: pointer to the layer
// matrix delta: partial derivative of loss w.r.t. output of layer
// returns: matrix, partial derivative of loss w.r.t. input to layer
matrix backward_layer(layer *l, matrix delta)
{
    gradient_matrix(l->out, l->activation, delta);

    free_matrix(l->dw);
    matrix in_transpose = transpose_matrix(l->in);
    matrix dw = matrix_mult_matrix(in_transpose, delta); // replace this
    l->dw = dw;
    free_matrix(in_transpose);

    matrix w_transpose = transpose_matrix(l->w);
    matrix dx = matrix_mult_matrix(delta, w_transpose); // replace this
    free_matrix(w_transpose);

    return dx;
}

// Update the weights at layer l
// layer *l: pointer to the layer
// double rate: learning rate
// double momentum: amount of momentum to use
// double decay: value for weight decay
void update_layer(layer *l, double rate, double momentum, double decay)
{
    // Update l->v
    matrix v = copy_matrix(l->dw);
    for (int i = 0; i < v.rows; i++) {
        for (int j = 0; j < v.cols; j++) {
            v.data[i][j] -= decay * l->w.data[i][j] - momentum * l->v.data[i][j];
        }
    }
    free_matrix(l->v);
    l->v = v;

    // Update l->w
    matrix weights_copy = copy_matrix(l->w);
    for (int i = 0; i < weights_copy.rows; i++) {
        for (int j = 0; j < weights_copy.cols; j++) {
            weights_copy.data[i][j] += rate * l->v.data[i][j];
        }
    }
    free_matrix(l->w);
    l->w = weights_copy;
}

// Make a new layer for our model
// int input: number of inputs to the layer
// int output: number of outputs from the layer
// ACTIVATION activation: the activation function to use
layer make_layer(int input, int output, ACTIVATION activation)
{
    layer l;
    l.in  = make_matrix(1,1);
    l.out = make_matrix(1,1);
    l.w   = random_matrix(input, output, sqrt(2./input));
    l.v   = make_matrix(input, output);
    l.dw  = make_matrix(input, output);
    l.activation = activation;
    return l;
}

// Run a model on input X
// model m: model to run
// matrix X: input to model
// returns: result matrix
matrix forward_model(model m, matrix X)
{
    int i;
    for(i = 0; i < m.n; ++i){
        X = forward_layer(m.layers + i, X);
    }
    return X;
}

// Run a model backward given gradient dL
// model m: model to run
// matrix dL: partial derivative of loss w.r.t. model output dL/dy
void backward_model(model m, matrix dL)
{
    matrix d = copy_matrix(dL);
    int i;
    for(i = m.n-1; i >= 0; --i){
        matrix prev = backward_layer(m.layers + i, d);
        free_matrix(d);
        d = prev;
    }
    free_matrix(d);
}

// Update the model weights
// model m: model to update
// double rate: learning rate
// double momentum: amount of momentum to use
// double decay: value for weight decay
void update_model(model m, double rate, double momentum, double decay)
{
    int i;
    for(i = 0; i < m.n; ++i){
        update_layer(m.layers + i, rate, momentum, decay);
    }
}

// Find the index of the maximum element in an array
// double *a: array
// int n: size of a, |a|
// returns: index of maximum element
int max_index(double *a, int n)
{
    if(n <= 0) return -1;
    int i;
    int max_i = 0;
    double max = a[0];
    for (i = 1; i < n; ++i) {
        if (a[i] > max){
            max = a[i];
            max_i = i;
        }
    }
    return max_i;
}

// Calculate the accuracy of a model on some data d
// model m: model to run
// data d: data to run on
// returns: accuracy, number correct / total
double accuracy_model(model m, data d)
{
    matrix p = forward_model(m, d.X);
    int i;
    int correct = 0;
    for(i = 0; i < d.y.rows; ++i){
        if(max_index(d.y.data[i], d.y.cols) == max_index(p.data[i], p.cols)) ++correct;
    }
    return (double)correct / d.y.rows;
}

// Calculate the cross-entropy loss for a set of predictions
// matrix y: the correct values
// matrix p: the predictions
// returns: average cross-entropy loss over data points, 1/n Σ(-ylog(p))
double cross_entropy_loss(matrix y, matrix p)
{
    int i, j;
    double sum = 0;
    for(i = 0; i < y.rows; ++i){
        for(j = 0; j < y.cols; ++j){
            sum += -y.data[i][j]*log(p.data[i][j]);
        }
    }
    return sum/y.rows;
}


// Train a model on a dataset using SGD
// model m: model to train
// data d: dataset to train on
// int batch: batch size for SGD
// int iters: number of iterations of SGD to run (i.e. how many batches)
// double rate: learning rate
// double momentum: momentum
// double decay: weight decay
void train_model(model m, data d, int batch, int iters, double rate, double momentum, double decay)
{
    int e;
    for(e = 0; e < iters; ++e){
        data b = random_batch(d, batch);
        matrix p = forward_model(m, b.X);
        fprintf(stderr, "%06d: Loss: %f\n", e, cross_entropy_loss(b.y, p));
        matrix dL = axpy_matrix(-1, p, b.y); // partial derivative of loss dL/dy
        backward_model(m, dL);
        update_model(m, rate/batch, momentum, decay);
        free_matrix(dL);
        free_data(b);
    }
}


// Questions
//
// 5.2.2.1 Why might we be interested in both training accuracy and testing accuracy? What do these two numbers tell us about our current model?
// Training accuracy tells us how accurately the model can predict seen data and testing accuracy tells us how the model performs on new data.
// High training accuracy paired with low testing accuracy means that the model is overfitting.

// 5.2.2.2 Try varying the model parameter for learning rate to different powers of 10 (i.e. 10^1, 10^0, 10^-1, 10^-2, 10^-3) and training the model. What patterns do you see and how does the choice of learning rate affect both the loss during training and the final model accuracy?
// If the learning rate is >= 1, the accuracy of the model in both training and test sets is awful as it overshoots the local minima.
// However, once the rate is < 0.1, the accuracy of the model shoots up considerably.
// The learning rate determines how much impact the gradient has on the weights at a given point.

// 5.2.2.3 Try varying the parameter for weight decay to different powers of 10: (10^0, 10^-1, 10^-2, 10^-3, 10^-4, 10^-5). How does weight decay affect the final model training and test accuracy?
// Changing the weight decay didn't have much of a impact on the accuracy of the model

// 5.2.3.1 Currently the model uses a logistic activation for the first layer. Try using a the different activation functions we programmed. How well do they perform? What's best?
// ReLU and LReLU performed near identically with ReLU being marginally better.
// Linear did surprisingly well followed by Logistic.
// Softmax did very poorly.

// 5.2.3.2 Using the same activation, find the best (power of 10) learning rate for your model. What is the training accuracy and testing accuracy?
// Using ReLU, the learning rate of 0.1 did the best when run with iters=1000

// 5.2.3.3 Right now the regularization parameter `decay` is set to 0. Try adding some decay to your model. What happens, does it help? Why or why not may this be?
// Decay helps in regularizing the weight updates and helps in preventing models overfitting to the training data.
// Too large (10, 1) of a decay hurts the accuracy of the model but anything < 0.1, seems to have minimal effect on the accuracy (+/- 1% accuracy).
//
// 5.2.3.4 Modify your model so it has 3 layers instead of two. The layers should be `inputs -> 64`, `64 -> 32`, and `32 -> outputs`. Also modify your model to train for 3000 iterations instead of 1000. Look at the training and testing error for different values of decay (powers of 10, 10^-4 -> 10^0). Which is best? Why?
// The model uses ReLU for the first layer, LReLU for the second, and softmax for the last layer. Learning rate is 0.1
// The best decay value is 0.01 with training accuracy of ~0.983 and test accuracy of ~0.971.
// This seems right as it balances the model between too powerful and overfitting

// 5.3.2.1 How well does your network perform on the CIFAR dataset?
// The best test set accuracy was about 46.9%