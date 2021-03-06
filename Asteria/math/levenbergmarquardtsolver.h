#ifndef LEVENBERGMARQUARDTSOLVER_H
#define LEVENBERGMARQUARDTSOLVER_H

/**
 * Note on the usage of virtual functions:
 * virtual keyword indicates a function that MAY be overridden in a derived class.
 * The =0 syntax indicates a 'pure virtual' function that MUST be overridden in derived classes.
 *
 *
 * @brief The LevenbergMarquardtSolver class
 * This class provides a general purpose implementation of the Levenberg-Marquardt
 * algorithm for nonlinear least squares. Applications that want to use this should
 * subclass it and implement/override the following methods:
 *
 * In order to use this class you must extend it and implement the getModel(Matrix params)
 * method and optionally the getJacobian(double[] params) method. If you would like the
 * fitter to use a numerical approximation to the Jacobian instead then override the
 * {@link #useFiniteDifferencesJacobian()} to return true and also override {@link #finiteDifferencesStepSizePerParam()}
 * to provide an appropriate step size for each parameter.
 *
 * Normal usage:
 *
 * TODO: improve this example usage! This is Java.
 *
 *	LevenbergMarquardt lma = new LevenbergMarquardt() {
 *
 *		@Override
 *		public void getModel(double * model) {
 *
 *          // Array 'model' has length N and we write the model values to this
 *          // Array 'params' has length M and contains the current values of the parameters
 *
 *			// Implement the parameters -> model function
 *          model[0] = f(params, x[0]);
 *          model[1] = f(params, x[1]);
 *
 *          // ....etc
 *
 *			return;
 *		}
 *
 *		@Override
 * 		public void getJacobian(double * jac) {
 *
 *          // Array 'jac' has length N*M and we write the Jacobian values to this
 *          // Array 'params' has length M and contains the current values of the parameters
 *
 *			// Compute the Jacobian elements
 *
 *			return;
 *		}
 *	}
 *
 *	lma.setData(electronSamples);
 *	lma.setCovariance(cov);
 *	lma.setInitialGuessParameters(params);
 *	// Perform the optimization
 *	lma.fit(500, true);
 *
 *	// Extract the solution
 *	double[] solution = lma.getParametersSolution();
 *
 * // Get the covariance matrix of the parameters:
 *
 *
 */

#include <Eigen/Dense>

using namespace Eigen;

class LevenbergMarquardtSolver
{

public:

    LevenbergMarquardtSolver(unsigned int M, unsigned int N);
    ~LevenbergMarquardtSolver();

    /**
     * @brief Set the Nx1 column vector of observed values
     * @param data
     * 	Pointer to an N-element array of observed values
     */
    void setData(const double *data);

    /**
     * @brief Set the Mx1 column vector of initial-guess parameters.
     *
     * @param params
     * 	Pointer to an M-element array of initial-guess parameters.
     */
    void setParameters(const double * params);

    /**
     * @brief Get the Mx1 column vector containing the fitted parameters.
     * @param params
     *  Pointer to an M-element array; on exit this will contain the solution
     */
    void getParameters(double * params);

    /**
     * @brief Set the NxN covariance matrix of the data points.
     *
     * @param covariance
     * 	NxN matrix of covariance of the data points, packed in a one dimensional array in
     * row-major order.
     */
    void setCovariance(const double *covariance);

    /**
     * @brief Set the Nx1 variance array of the data points. This is for applications where the covariance
     * matrix is diagonal, i.e. no covariance terms.
     *
     * @param variance
     * 	Nx1 array of variance values for each data point.
     */
    void setVariance(const double *variance);

    /**
     * @brief Get f(X,P): column vector of model values given x points and current
     * parameters set.
     *
     * f(X,P) = [f(x_1, P), f(x_2, P), ... , f(x_N, P)]^T
     *
     * This method MUST be overridden in the derived class.
     *
     * @param model
     *  Pointer to an N-element array that on exit will contain the model values
     */
    virtual void getModel(double * model) =0;

    /**
     * @brief Get the Jacobian matrix -> the matrix of partial derivatives of the
     * model values with respect to the parameters, given the current parameter set.
     *
     * J = [ df(x_0, P)/dp_0  df(x_0, P)/dp_1  df(x_0, p)/dp_2  ...  df(x_0, P)/dp_{M-1} ]
     *     [ df(x_1, P)/dp_0  df(x_1, P)/dp_1  df(x_1, p)/dp_2  ...  df(x_1, P)/dp_{M-1} ]
     *
     *
     * Leading dimension is number of rows, trailing dimension is number of columns, i.e.
     * A[r][c] has r rows and c columns.
     *
     * This function MAY be overridden in the derived class if an analytic Jacobian is possible.
     * A default implementation based on finite differences is provided.
     *
     * @param jac
     *  NxM element array that on exit will contain the Jacobian values, packed in a one
     * dimensional array in row-major order.
     */
    virtual void getJacobian(double * jac);

    /**
     * @brief Implementing classes should override this to provide appropriate step sizes per parameter for use
     * in the finite differences Jacobian approximation, if they intend to use that.
     *
     * This function MAY be overridden in the derived class: if an analytic Jacobian is not possible and the
     * getJacobian(double[] params) method is not overridden, then this function should be overridden to
     * provide appropriate step sizes for each parameter in the numerical Jacobian approximation.
     *
     * @param steps
     *  Pointer to an M-element array; on exit this contains appropriate finite-difference step sizes for each
     * parameter.
     */
    virtual void finiteDifferencesStepSizePerParam(double *steps);

    /**
     * @brief Method called whenever the algorithm updates the parameters (such as during regular iterations, and
     * during estimation of the Jacobian using finite differences). The default implementation does nothing, however
     * the user can override this in their derived class in case they need to perform any post-processing on the
     * adjusted parameters for example to enforce normalisation on unit vector or quaternion elements.
     */
    virtual void postParameterUpdateCallback();

    /**
     * @brief Perform LM iteration loop until parameters cannot be improved.
     * @param maxIterations  Maximum number of allowed iteration before convergence.
     * @param verbose        Enables verbose logging
     */
    void fit(unsigned int maxIterations, bool verbose);

    /**
     * @brief Chi-square statistic, (x - f(x))^T*C^{-1}*(x - f(x))
     */
    double getChi2();

    /**
     * @brief Reduced Chi-square statistic.
     */
    double getReducedChi2();

    /**
     * @brief Degrees of freedom of fit.
     */
    double getDOF();

    /**
     * @brief Set the absolute step size used in the finite difference Jacobian estimation for the
     * calculation of the rate of change of parameters as a function of the data. This step is applied
     * to the data values, so should be of the appropriate order of magnitude.
     * @param H
     * 	The finite step size to be applied to the data
     */
    void setH(double h);

    /**
     * @brief Set the exit tolerance - if the (absolute value of the) relative change in the chi-square
     * from one iteration to the next is lower than this, then we're at the minimum and the fit
     * is halted.
     * @param exitTolerance
     * 	The exit tolerance to set
     */
    void setExitTolerance(double exitTolerance);

    /**
     * @brief Set the maximum damping factor. If the damping factor becomes larger than this during the fit,
     * then we're stuck and cannot reach a better solution.
     *
     * @param maxDamping
     * 	The max damping factor to set
     */
    void setMaxDamping(double maxDamping);

    /**
     * @brief Set the factor by which the Levenberg-Marquardt step is inflated or deflated in order
     * to find a good parameter step.
     * @param boostShrinkFactor
     *  The boost/shrink factor
     */
    void setBoostShrinkFactor(double boostShrinkFactor);

    /**
     * @brief This method estimates parameter covariance by propagating data
     * covariance through the system using the following equation:
     *
     * S_p = (dp/dx)^T S_x (dp/dx)
     *
     * It uses a fourth order central difference approximation for the
     * parameter/data Jacobian.
     *
     * Notes:
     * 1) This method fails for functions that are significantly
     * non-linear within a STDDEV or two of current solution.
     * 2) This method gives results that are basically identical to the
     * getParameterCovariance() function, scaled by the inverse of the
     * reduced chi-square.
     */
    MatrixXd getFourthOrderCovariance();

    /**
     * @brief Get the covariance matrix for parameters. This method has been tested
     * against Gnuplot 'fit' function and provides the same asymptotic
     * standard error and parameter correlation.
     */
    MatrixXd getParameterCovariance();

    /**
     * @brief Get the asymptotic standard error for the parameters
     * @param errors
     *  Pointer to an M-element array fo doubles; on exit this will contain the asymptotic
     * standard error for each parameter.
     */
    void getAsymptoticStandardError(double * errors);

    /**
     * @brief Get the correlation matrix for the parameters.
     */
    MatrixXd getParameterCorrelation();

protected:

    /**
     * Number of free parameters.
     */
    unsigned int M;

    /**
     * Number of data points.
     */
    unsigned int N;

    /**
     * @brief Absolute step size used in finite difference Jacobian approximation,
     * for Jacobian of parameter solution with respect to data. This step is
     * applied to the data.
     */
    double h = 1E-2;

    /**
     * Exit tolerance
     */
    double exitTolerance = 1E-32;

    /**
     * @brief Max damping scale factor. This is multiplied by automatically selected
     * starting value of damping parameter, which is 10^{-3}
     * times the average of the diagonal elements of J^T*J
     */
    double maxDamping = 1E32;

    /**
     * @brief The factor by which the Levenberg-Marquardt step is inflated or deflated in order
     * to find a good parameter step.
     */
    double boostShrinkFactor = 10;

    /**
     * @brief Nx1 column vector of observed values
     *
     * Y = [y_0, y_1, y_2, ..., y_{N-1}]^T
     *
     */
    double * data;

    /**
     * @brief The current model values. After the fit is complete this contains the final fitted model; during the
     * fit it is used to store temporary values.
     */
    double * model;

    /**
     * @brief Covariance matrix for the observed values. This is either of size NxN if the full covariance matrix
     * is specified, or Nx1 if only variance terms are given.
     */
    double *  covariance;

    /**
     * @brief Flag that indicates a diagonal covariance matrix, i.e. no covariance terms, only variance. This means we can
     * store the terms in a Nx1 array and do efficient inversion of the covariance in the linear algebra.
     */
    bool covarianceIsDiagonal;

    /**
     * @brief Mx1 column vector of parameters
     *
     * P = [p_0, p_1, p_2, ..., p_{M-1}]^T
     *
     */
    double * params;

    /**
     * @brief Each call performs one iteration of parameters.
     *
     * @param lambda Current value of the damping parameter
     * @param maxLambda Maximum allowed value of the dampling parameter
     *
     * @return bool  States whether another iteration would be appropriate, or
     *                  if change in residuals and/or damping thresholds have
     *                  been reached
     */
    bool iteration(double &lambda, const double &maxLambda, bool verbose);

    /**
     * @brief Get the residuals (x - f(x)) for the current model values.
     */
    void getResiduals(double *residuals);

    /**
     * @brief Finite difference Jacobian approximation. This is the derivative of the
     * parameters solution with respect to the data, useful in estimating the
     * parameters covariance with respect to the data.
     *
     * Jacobian is N rows by M columns.
     *
     * This uses fourth-order central difference approximation for the first
     * derivative.
     */
    MatrixXd getJacobian_dpdx();

};

#endif // LEVENBERGMARQUARDTSOLVER_H
