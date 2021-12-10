// #include <string>
// #include <vector>

// #include "ortools/constraint_solver/routing.h"
// #include "ortools/constraint_solver/routing_enums.pb.h"
// #include "ortools/constraint_solver/routing_index_manager.h"
// #include "ortools/constraint_solver/routing_parameters.h"

// namespace operations_research {
// struct DataModel {
//     std::vector<std::vector<int64>> time_matrix;
//     std::vector<std::pair<int64, int64>> time_windows;
//     const int num_vehicles = 4;
//     const RoutingIndexManager::NodeIndex depot{0};
// };

// struct VRPResult {
//     VRPResult(std::vector<std::vector<int>> routes_, 
//         std::vector<std::vector<std::pair<int64, int64>>> cumulData_) {
//         routes = routes_;
//         cumulData = cumulData_;
//     }
//     std::vector<std::vector<int>> routes;
//     std::vector<std::vector<std::pair<int64, int64>>> cumulData;
// }


// VRPResult VrpTimeWindows(DataModel data) {

//     // Create Routing Index Manager
//     RoutingIndexManager manager(data.time_matrix.size(), data.num_vehicles,
//                                 data.depot);

//     // Create Routing Model.
//     RoutingModel routing(manager);

//     // Create and register a transit callback.
//     const int transit_callback_index = routing.RegisterTransitCallback(
//         [&data, &manager](int64 from_index, int64 to_index) -> int64 {
//         // Convert from routing variable Index to time matrix NodeIndex.
//         auto from_node = manager.IndexToNode(from_index).value();
//         auto to_node = manager.IndexToNode(to_index).value();
//         return data.time_matrix[from_node][to_node];
//         });

//     // Define cost of each arc.
//     routing.SetArcCostEvaluatorOfAllVehicles(transit_callback_index);

//     // Add Time constraint.
//     std::string time{"Time"};
//     routing.AddDimension(transit_callback_index,  // transit callback index
//                         int64{30},               // allow waiting time
//                         int64{30},               // maximum time per vehicle
//                         false,  // Don't force start cumul to zero
//                         time);
//     const RoutingDimension& time_dimension = routing.GetDimensionOrDie(time);
//     // Add time window constraints for each location except depot.
//     for (int i = 1; i < data.time_windows.size(); ++i) {
//     int64 index = manager.NodeToIndex(RoutingIndexManager::NodeIndex(i));
//     time_dimension.CumulVar(index)->SetRange(data.time_windows[i].first,
//                                                 data.time_windows[i].second);
//     }
//     // Add time window constraints for each vehicle start node.
//     for (int i = 0; i < data.num_vehicles; ++i) {
//     int64 index = routing.Start(i);
//     time_dimension.CumulVar(index)->SetRange(data.time_windows[0].first,
//                                                 data.time_windows[0].second);
//     }

//     // Instantiate route start and end times to produce feasible times.
//     for (int i = 0; i < data.num_vehicles; ++i) {
//     routing.AddVariableMinimizedByFinalizer(
//         time_dimension.CumulVar(routing.Start(i)));
//     routing.AddVariableMinimizedByFinalizer(
//         time_dimension.CumulVar(routing.End(i)));
//     }

//     // Setting first solution heuristic.
//     RoutingSearchParameters searchParameters = DefaultRoutingSearchParameters();
//     searchParameters.set_first_solution_strategy(
//         FirstSolutionStrategy::PATH_CHEAPEST_ARC);

//     // Solve the problem.
//     const Assignment* solution = routing.SolveWithParameters(searchParameters);

//     // Print solution on console.
//     auto routes = GetRoutes(solution, routing, manager);
//     auto cumulData = GetCumulData(solution, routing, dimension);
//     return VRPResult(routes, cumulData);
// }

// std::vector<std::vector<int>> GetRoutes(const Assignment& solution,
//                                         const RoutingModel& routing,
//                                         const RoutingIndexManager& manager) {
//     // Get vehicle routes and store them in a two dimensional array, whose
//     // i, j entry is the node for the jth visit of vehicle i.
//     std::vector<std::vector<int>> routes(manager.num_vehicles());
//     // Get routes.
//     for (int vehicle_id = 0; vehicle_id < manager.num_vehicles(); ++vehicle_id) {
//         int64 index = routing.Start(vehicle_id);
//         routes[vehicle_id].push_back(manager.IndexToNode(index).value());
//         while (!routing.IsEnd(index)) {
//             index = solution.Value(routing.NextVar(index));
//             routes[vehicle_id].push_back(manager.IndexToNode(index).value());
//         }
//     }
//     return routes;
// }

// std::vector<std::vector<std::pair<int64, int64>>> GetCumulData(
//     const Assignment& solution, const RoutingModel& routing,
//     const RoutingDimension& dimension) {
//     // Returns an array cumul_data, whose i, j entry is a pair containing
//     // the minimum and maximum of CumulVar for the dimension.:
//     // - cumul_data[i][j].first is the minimum.
//     // - cumul_data[i][j].second is the maximum.
//     std::vector<std::vector<std::pair<int64, int64>>> cumul_data(
//         routing.vehicles());

//     for (int vehicle_id = 0; vehicle_id < routing.vehicles(); ++vehicle_id) {
//         int64 index = routing.Start(vehicle_id);
//         IntVar* dim_var = dimension.CumulVar(index);
//         cumul_data[vehicle_id].emplace_back(solution.Min(dim_var),
//                                             solution.Max(dim_var));
//         while (!routing.IsEnd(index)) {
//             index = solution.Value(routing.NextVar(index));
//             IntVar* dim_var = dimension.CumulVar(index);
//             cumul_data[vehicle_id].emplace_back(solution.Min(dim_var),
//                                                 solution.Max(dim_var));
//         }
//     }
//     return cumul_data;
// }

// DataModel parseDataModel() {

// }

// VRPResult callVrpSolver(vector<int> landmarks, int numUAVs, double maxDelay) {
//     DataModel data = parseDataModel(landmarks, numUAVs, maxDelay);

//     return VrpTimeWindows(data);
// }
// }  // namespace operations_research