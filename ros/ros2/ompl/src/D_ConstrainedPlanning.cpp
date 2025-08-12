#include <memory>

#include <ompl/geometric/SimpleSetup.h>
#include <ompl/geometric/PathGeometric.h>
#include <ompl/geometric/planners/rrt/RRT.h>

#include <ompl/base/Constraint.h>
#include <ompl/base/ConstrainedSpaceInformation.h>
#include <ompl/base/spaces/constraint/ConstrainedStateSpace.h>
#include <ompl/base/spaces/constraint/ProjectedStateSpace.h>
#include <ompl/base/spaces/RealVectorStateSpace.h>

namespace ob = ompl::base;
namespace og = ompl::geometric;
namespace om = ompl::magic;
// namespace ot = ompl::tools;

// Constraints must inherit from the constraint base class. By default, a
// numerical approximation to the Jacobian of the constraint function is computed
// using a central finite difference.
class SphereConstraint : public ob::Constraint
{
public:
    // ob::Constraint's constructor takes in two parameters, the dimension of
    // the ambient state space, and the dimension of the real vector space the
    // constraint maps into. For our sphere example, as we are planning in R^3, the
    // dimension of the ambient space is 3, and as our constraint outputs one real
    // value the second parameter is one (this is also the co-dimension of the
    // constraint manifold).
    SphereConstraint() : ob::Constraint(3, 1)
    {
    }

    // Here we define the actual constraint function, which takes in some state "x"
    // (from the ambient space) and sets the values of "out" to the result of the
    // constraint function. Note that we are implementing `function` which has this
    // function signature, not the one that takes in ompl::base::State.
    void function(const Eigen::Ref<const Eigen::VectorXd> &x, Eigen::Ref<Eigen::VectorXd> out) const override
    {
        // The function that defines a sphere is f(q) = || q || - 1, as discussed
        // above. Eigen makes this easy to express:
        out[0] = x.norm() - 1;
    }

    // Implement the Jacobian of the constraint function. The Jacobian for the
    // constraint function is an matrix of dimension equal to the co-dimension of the
    // constraint by the ambient dimension (in this case, 1 by 3). Similar to
    // `function` above, we implement the `jacobian` method with the following
    // function signature.
    void jacobian(const Eigen::Ref<const Eigen::VectorXd> &x, Eigen::Ref<Eigen::MatrixXd> out) const override
    {
        out = x.transpose().normalized();
    }
};

bool obstacle(const ob::State *state)
{
    // As ob::ConstrainedStateSpace::StateType inherits from
    // Eigen::Map<Eigen::VectorXd>, we can grab a reference to it for some easier
    // state access.
    const Eigen::Map<Eigen::VectorXd> &x = *state->as<ob::ConstrainedStateSpace::StateType>();

    // Alternatively, we could access the underlying real vector state with the
    // following incantation:
    //   auto x = state->as<ob::ConstrainedStateSpace::StateType>()->getState()->as<ob::RealVectorStateSpace::StateType>();
    // Note the use of "getState()" on the constrained state. This accesss the
    // underlying state that was allocated by the ambient state space.

    // Define a narrow band obstacle with a small hole on one side.
    if (-0.1 < x[2] && x[2] < 0.1)
    {
        if (-0.05 < x[0] && x[0] < 0.05)
            return x[1] < 0;

        return false;
    }

    return true;
}

int main(int argc, char **argv)
{
  // I. Create the ambient space state space for the problem.
  auto rvss = std::make_shared<ob::RealVectorStateSpace>(3);
  ob::RealVectorBounds bounds(3);
  bounds.setLow(-2);
  bounds.setHigh(2);
  rvss->setBounds(bounds);

  // II. Define the constrained space information for this constrained state space.
  auto constraint = std::make_shared<SphereConstraint>();
  auto css = std::make_shared<ob::ProjectedStateSpace>(rvss, constraint);
  auto csi = std::make_shared<ob::ConstrainedSpaceInformation>(css);

  // III. Simple Setup
  auto ss = std::make_shared<og::SimpleSetup>(csi);
  ss->setStateValidityChecker(obstacle);
  // Start and goal vectors.
  Eigen::VectorXd sv(3), gv(3);
  sv << 0, 0, -1;
  gv << 0, 0,  1;

  // Scoped states that we will add to simple setup.
  ob::ScopedState<> start(css);
  ob::ScopedState<> goal(css);

  // Copy the values from the vectors into the start and goal states.
  start->as<ob::ConstrainedStateSpace::StateType>()->copy(sv);
  goal->as<ob::ConstrainedStateSpace::StateType>()->copy(gv);

  // If we were using an Atlas or TangentBundleStateSpace, we would also have to anchor these states to charts:
  //   css->anchorChart(start.get());
  //   css->anchorChart(goal.get());
  // Which gives a starting point for the atlas to grow.

  ss->setStartAndGoalStates(start, goal);

  // Planned
  auto pp = std::make_shared<og::RRT>(csi);
ss->setPlanner(pp);

  ss->setup();

  // IV. Solve the problem
  ob::PlannerStatus stat = ss->solve(5.);
  if (stat)
  {
      // Path simplification also works when using a constrained state space!
      ss->simplifySolution(5.);

      // Get solution path.
      auto path = ss->getSolutionPath();

      // Interpolation also works on constrained state spaces, and is generally required.
      path.interpolate();

      // Then do whatever you want with the path, like normal!
      path.print(std::cout);
  }
  else
      OMPL_WARN("No solution found!");
  return 0;
}
