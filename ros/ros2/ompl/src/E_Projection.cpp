#include "PlanningCommon.h"
#include <ompl/base/ConstrainedSpaceInformation.h>
#include <ompl/base/ProjectionEvaluator.h>
#include <ompl/base/spaces/constraint/ConstrainedStateSpace.h>
#include <ompl/geometric/planners/kpiece/KPIECE1.h>

class MyProjection : public ob::ProjectionEvaluator {
public:
  MyProjection(const ob::StateSpacePtr &space)
      : ob::ProjectionEvaluator(space) {}

  virtual unsigned int getDimension(void) const { return 2; }

  virtual void defaultCellSizes(void) {
    cellSizes_.resize(2);
    cellSizes_[0] = 0.1;
    cellSizes_[1] = 0.25;
  }

  virtual void project(const ob::State *state,
                       Eigen::Ref<Eigen::VectorXd> projection) const {
    const double *values =
        state->as<ob::RealVectorStateSpace::StateType>()->values;
    projection(0) = (values[0] + values[1]) / 2.0;
    projection(1) = (values[2] + values[3]) / 2.0;
  }
};

bool obstacle(const ob::State *state) {
  const ob::RealVectorStateSpace::StateType &x =
      *state->as<ob::RealVectorStateSpace::StateType>();

  if (sqrt(x[0]*x[0]+x[1]*x[1]+x[2]*x[2])<1)
  {
      return false;
  }

  return true;
}

int main(int argc, char **argv) {
  // Create the ambient space state space for the problem.
  auto rvss = std::make_shared<ob::RealVectorStateSpace>(3);

  ob::RealVectorBounds bounds(3);
  bounds.setLow(-2);
  bounds.setHigh(2);
  rvss->setBounds(bounds);
  rvss->registerProjection("my_projection",
                           std::make_shared<MyProjection>(rvss));

  // Simple Setup
  auto ss = std::make_shared<og::SimpleSetup>(rvss);
  ss->setStateValidityChecker(obstacle);

  // Start and goal vectors.
  ob::ScopedState<> start(rvss);
  ob::ScopedState<> goal(rvss);
  start[0] = start[1] = 0.;
  start[2] = -1.1;
  goal[0] = goal[1] = 0.;
  goal[2] = 1.1;
  ss->setStartAndGoalStates(start, goal);

  if (argc < 2) {
    auto planner = std::make_shared<og::KPIECE1>(ss->getSpaceInformation());
    planner->setProjectionEvaluator("my_projection");
    ss->setPlanner(planner);
  } else {
    auto planner(std::make_shared<og::RRTConnect>(ss->getSpaceInformation()));
    ss->setPlanner(planner);
  }
  ss->setup();

  // IV. Solve the problem
  ob::PlannerStatus stat = ss->solve(5.);
  if (stat) {
    // Path simplification also works when using a constrained state space!
    ss->simplifySolution(5.);

    // Get solution path.
    auto path = ss->getSolutionPath();

    // Interpolation also works on constrained state spaces, and is generally
    // required.
    path.interpolate();

    // Then do whatever you want with the path, like normal!
    path.print(std::cout);

    std::cout << path.length() << std::endl;
  } else
    OMPL_WARN("No solution found!");
  return 0;
}
