// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2016.
//
// This software is released under a three-clause BSD license:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of any author or any participating institution
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
// For a full list of authors, refer to the file AUTHORS.
// --------------------------------------------------------------------------
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL ANY OF THE AUTHORS OR THE CONTRIBUTING
// INSTITUTIONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// --------------------------------------------------------------------------
// $Maintainer: Hendrik Weisser $
// $Authors: Hendrik Weisser $
// --------------------------------------------------------------------------

#ifndef OPENMS_ANALYSIS_SVM_SIMPLESVM_H
#define OPENMS_ANALYSIS_SVM_SIMPLESVM_H

#include <OpenMS/DATASTRUCTURES/DefaultParamHandler.h>

#include <svm.h>

#include <map>
#include <vector>
#include <utility> // for "pair"

namespace OpenMS
{
  /**
     @brief Simple interface to support vector machines for classification (via LIBSVM).

     This class supports (multi-class) classification with a linear or RBF kernel.
     It uses cross-validation to optimize the SVM parameters @e C and (RBF kernel only) @e gamma.

     SVM models are generated by the the setup() method.
     To simplify the scaling of input data (predictors), the data for both the training and the test set together are passed in as parameter @p predictors.
     Given @e N observations of @e M predictors, the data are coded as a map of predictors (size @e M), each a numeric vector of values for different observations (size @e N).

     The parameter @p labels of setup() defines the training set; it contains the indexes of observations (corresponding to positions in the vectors in @p predictors) together with the class labels for training.

     To predict class labels based on a model, use the predict() method.
     The parameter @p indexes of predict() takes a vector of indexes corresponding to the observations for which predictions should be made.
     (With an empty vector, the default, predictions are made for all observations, including those used for training.)

     @htmlinclude OpenMS_SimpleSVM.parameters
  */
  class OPENMS_DLLAPI SimpleSVM :
    public DefaultParamHandler
  {

  public:
    /// Mapping from predictor name to vector of predictor values
    typedef std::map<String, std::vector<double> > PredictorMap;

    /// SVM prediction result
    struct Prediction
    {
      /// Predicted class label
      Int label;

      /// Predicted probabilities for different classes
      std::map<Int, double> probabilities;
    };

    /// Default constructor
    SimpleSVM();

    /// Destructor
    virtual ~SimpleSVM();

    /**
       @brief Load data and train a model.

       @param predictors Mapping from predictor name to vector of predictor values (for different observations). All vectors should have the same length; values will be changed by scaling.
       @param labels Mapping from observation index to class label in the training set.

       @throw Exception::IllegalArgument if @p predictors is empty
       @throw Exception::InvalidValue if an invalid index is used in @p labels
       @throw Exception::MissingInformation if there are fewer than two class labels in @p labels, or if there are not enough observations for cross-validation
    */
    void setup(PredictorMap& predictors, const std::map<Size, Int>& labels);

    /**
       @brief Predict class labels (and probabilities).

       @param predictions Output vector of prediction results (same order as @p indexes).
       @param indexes Vector of observation indexes for which predictions are desired. If empty (default), predictions are made for all observations.

       @throw Exception::Precondition if no model has been trained
       @throw Exception::InvalidValue if an invalid index is used in @p indexes
    */
    void predict(std::vector<Prediction>& predictions,
                 std::vector<Size> indexes = std::vector<Size>()) const;

    /**
       @brief Get the weights used for features (predictors) in the SVM model

       Currently only supported for two-class classification.
       If a linear kernel is used, the weights are informative for ranking features.

       @throw Exception::Precondition if no model has been trained, or if the classification involves more than two classes
    */
    void getFeatureWeights(std::map<String, double>& feature_weights) const;

    /// Write cross-validation (parameter optimization) results to a CSV file
    void writeXvalResults(const String& path) const;

  protected:
    /// Classification performance for different param. combinations (C/gamma):
    typedef std::vector<std::vector<double> > SVMPerformance;

    /// Values of predictors (LIBSVM format)
    std::vector<std::vector<struct svm_node> > nodes_;

    /// SVM training data (LIBSVM format)
    struct svm_problem data_;

    /// SVM parameters (LIBSVM format)
    struct svm_parameter svm_params_;

    /// Pointer to SVM model (LIBSVM format)
    struct svm_model* model_;

    /// Names of predictors in the model (excluding uninformative ones)
    std::vector<String> predictor_names_;

    /// Number of partitions for cross-validation
    Size n_parts_;

    /// Parameter values to try during optimization
    std::vector<double> log2_C_, log2_gamma_;

    /// Cross-validation results
    SVMPerformance performance_;

    /// Dummy function to suppress LIBSVM output
    static void printNull_(const char*) {}

    /// Scale predictor values to range 0-1
    void scaleData_(PredictorMap& predictors) const;

    /// Convert predictors to LIBSVM format
    void convertData_(const PredictorMap& predictors);

    /// Choose best SVM parameters based on cross-validation results
    std::pair<double, double> chooseBestParameters_() const;

    /// Run cross-validation to optimize SVM parameters
    void optimizeParameters_();
  };
}

#endif // #ifndef OPENMS_ANALYSIS_SVM_SIMPLESVM_H
