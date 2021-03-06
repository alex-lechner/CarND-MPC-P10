#include "MPC.h"
#include <cppad/cppad.hpp>
#include <cppad/ipopt/solve.hpp>
#include "Eigen-3.3/Eigen/Core"

using CppAD::AD;

const size_t N = 12;
const double dt = 0.04, Lf = 2.67, ref_v = 40;

// This value assumes the model presented in the classroom is used.
//
// It was obtained by measuring the radius formed by running the vehicle in the
// simulator around in a circle with a constant steering angle and velocity on a
// flat terrain.
//
// Lf was tuned until the the radius formed by the simulating the model
// presented in the classroom matched the previous radius.
//
// This is the length from front to CoG that has a similar radius.

size_t x_start = 0;
size_t y_start = x_start + N;
size_t psi_start = y_start + N;
size_t v_start = psi_start + N;
size_t cte_start = v_start + N;
size_t epsi_start = cte_start + N;
size_t delta_start = epsi_start + N;
size_t a_start = delta_start + N - 1;

class FG_eval {
public:
    // Fitted polynomial coefficients
    Eigen::VectorXd coeffs;

    FG_eval(Eigen::VectorXd coeffs) { this->coeffs = coeffs; }

    typedef CPPAD_TESTVECTOR(AD<double>) ADvector;

    void operator()(ADvector &fg, const ADvector &vars) {
        // `fg` a vector of the cost constraints, `vars` is a vector of variable values (state & actuators)
        // NOTE: You'll probably go back and forth between this function and
        // the Solver function below.
        fg[0] = 0;


        //Reference State
        for (size_t i = 0; i < N; ++i) {
            fg[0] += CppAD::pow(vars[cte_start + i], 2);
            fg[0] += CppAD::pow(vars[epsi_start + i], 2);
            fg[0] += CppAD::pow(vars[v_start + i] - ref_v, 2);
        }

        //minimize use of actuators/change-rate for a smooth turn
        for (size_t i = 0; i < N - 1; ++i) {
            fg[0] += CppAD::pow(vars[delta_start + i], 2);
            fg[0] += CppAD::pow(vars[a_start + i], 2);
        }

        //minimize value gap
        for (size_t i = 0; i < N - 2; ++i) {
            fg[0] += 1000 * CppAD::pow(vars[delta_start + i + 1] - vars[delta_start + i], 2);
            fg[0] += 1000 * CppAD::pow(vars[a_start + i + 1] - vars[a_start + i], 2);
        }


        // Initial constraints
        //
        // We add 1 to each of the starting indices due to cost being located at
        // index 0 of `fg`.
        // This bumps up the position of all the other values.
        fg[1 + x_start] = vars[x_start];
        fg[1 + y_start] = vars[y_start];
        fg[1 + psi_start] = vars[psi_start];
        fg[1 + v_start] = vars[v_start];
        fg[1 + cte_start] = vars[cte_start];
        fg[1 + epsi_start] = vars[epsi_start];

        // The rest of the constraints
        for (size_t t = 1; t < N; t++) {
            AD<double> x1, y1, psi1, v1, cte1, epsi1, x0, y0, psi0, v0, cte0, epsi0, delta0, a0, f0, psides0;

            x1 = vars[x_start + t];
            y1 = vars[y_start + t];
            psi1 = vars[psi_start + t];
            v1 = vars[v_start + t];
            cte1 = vars[cte_start + t];
            epsi1 = vars[epsi_start + t];


            x0 = vars[x_start + t - 1];
            y0 = vars[y_start + t - 1];
            psi0 = vars[psi_start + t - 1];
            v0 = vars[v_start + t - 1];
            cte0 = vars[cte_start + t - 1];
            epsi0 = vars[epsi_start + t - 1];

            delta0 = vars[delta_start + t - 1];
            a0 = vars[a_start + t - 1];
            f0 = coeffs[0] + coeffs[1] * x0 + coeffs[2] * CppAD::pow(x0, 2) + coeffs[3] * CppAD::pow(x0, 3);
            psides0 = CppAD::atan(coeffs[1] + 2 * coeffs[2] * x0 + 3 * coeffs[3] * CppAD::pow(x0, 2));

            // Here's `x` to get you started.
            // The idea here is to constraint this value to be 0.
            //
            // NOTE: The use of `AD<double>` and use of `CppAD`!
            // This is also CppAD can compute derivatives and pass
            // these to the solver.
            fg[1 + x_start + t] = x1 - (x0 + v0 * CppAD::cos(psi0) * dt);
            fg[1 + y_start + t] = y1 - (y0 + v0 * CppAD::sin(psi0) * dt);
            fg[1 + psi_start + t] = psi1 - (psi0 + v0 * delta0 / Lf * dt);
            fg[1 + v_start + t] = v1 - (v0 + a0 * dt);
            fg[1 + cte_start + t] = cte1 - ((f0 - y0) + (v0 * CppAD::sin(epsi0) * dt));
            fg[1 + epsi_start + t] = epsi1 - ((psi0 - psides0) + v0 * delta0 / Lf * dt);
        }
    }
};

//
// MPC class definition implementation.
//
MPC::MPC() {}

MPC::~MPC() {}

vector<double> MPC::Solve(Eigen::VectorXd state, Eigen::VectorXd coeffs) {
    bool ok = true;
    typedef CPPAD_TESTVECTOR(double) Dvector;

    x = state[0];
    y = state[1];
    psi = state[2];
    v = state[3];
    cte = state[4];
    epsi = state[5];

    // For example: If the state is a 4 element vector, the actuators is a 2
    // element vector and there are 10 timesteps. The number of variables is:
    //
    // 4 * 10 + 2 * 9
    size_t n_vars =  6 * N + 2 * (N - 1);
    size_t n_constraints = N * 6;

    // Initial value of the independent variables.
    // SHOULD BE 0 besides initial state.
    Dvector vars(n_vars);
    for (size_t i = 0; i < n_vars; i++) {
        vars[i] = 0;
    }

    Dvector vars_lowerbound(n_vars);
    Dvector vars_upperbound(n_vars);

    // Set all non-actuators upper and lowerlimits
    // to the max negative and positive values.
    for (size_t j = 0; j < delta_start; ++j) {
        vars_lowerbound[j] = -1.0e19;
        vars_upperbound[j] = 1.0e19;
    }

    // The upper and lower limits of delta are set to -25 and 25
    // degrees (values in radians).
    for (size_t k = delta_start; k < a_start; ++k) {
        vars_lowerbound[k] = -0.436332;
        vars_upperbound[k] = 0.436332;
    }

    // Acceleration/decceleration upper and lower limits.
    for (size_t l = a_start; l < n_vars; ++l) {
        vars_lowerbound[l] = -1.0;
        vars_upperbound[l] = 1.0;
    }

    // Lower and upper limits for the constraints
    // Should be 0 besides initial state.
    Dvector constraints_lowerbound(n_constraints);
    Dvector constraints_upperbound(n_constraints);
    for (size_t m = 0; m < n_constraints; ++m) {
        constraints_lowerbound[m] = 0;
        constraints_upperbound[m] = 0;
    }

    constraints_lowerbound[x_start] = constraints_upperbound[x_start] = x;
    constraints_lowerbound[y_start] = constraints_upperbound[y_start] = y;
    constraints_lowerbound[psi_start] = constraints_upperbound[psi_start] = psi;
    constraints_lowerbound[v_start] = constraints_upperbound[v_start] = v;
    constraints_lowerbound[cte_start] = constraints_upperbound[cte_start] = cte;
    constraints_lowerbound[epsi_start] = constraints_upperbound[epsi_start] = epsi;

    // object that computes objective and constraints
    FG_eval fg_eval(coeffs);

    //
    // NOTE: You don't have to worry about these options
    //
    // options for IPOPT solver
    std::string options;
    // Uncomment this if you'd like more print information
    options += "Integer print_level  0\n";
    // NOTE: Setting sparse to true allows the solver to take advantage
    // of sparse routines, this makes the computation MUCH FASTER. If you
    // can uncomment 1 of these and see if it makes a difference or not but
    // if you uncomment both the computation time should go up in orders of
    // magnitude.
    options += "Sparse  true        forward\n";
    options += "Sparse  true        reverse\n";
    // NOTE: Currently the solver has a maximum time limit of 0.5 seconds.
    // Change this as you see fit.
    options += "Numeric max_cpu_time          0.5\n";

    // place to return solution
    CppAD::ipopt::solve_result <Dvector> solution;

    // solve the problem
    CppAD::ipopt::solve<Dvector, FG_eval>(
            options, vars, vars_lowerbound, vars_upperbound, constraints_lowerbound,
            constraints_upperbound, fg_eval, solution);

    // Check some of the solution values
    ok &= solution.status == CppAD::ipopt::solve_result<Dvector>::success;

    // Cost
    auto cost = solution.obj_value;
    std::cout << "Cost " << cost << std::endl;

    // Return the first actuator values. The variables can be accessed with
    // `solution.x[i]`.
    //
    // {...} is shorthand for creating a vector, so auto x1 = {1.0,2.0}
    // creates a 2 element double vector.
    vector<double> solution_vals;

    solution_vals.push_back(solution.x[delta_start]);
    solution_vals.push_back(solution.x[a_start]);

    for (size_t n = 0; n < N - 1; ++n) {
        solution_vals.push_back(solution.x[x_start + n + 1]);
        solution_vals.push_back(solution.x[y_start + n + 1]);
    }

    return solution_vals;
}
