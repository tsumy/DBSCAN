INCLUDE_DIRECTORIES( SYSTEM ${google-glog_BINARY_DIR} ${PROJECT_SOURCE_DIR}/glog/src/ )

add_executable(cluster_csv main_cluster_csv.cpp)
target_link_libraries(cluster_csv boost_program_options glog)

add_executable(knn_dist main_knn_dist.cpp)
target_link_libraries(knn_dist boost_program_options glog)