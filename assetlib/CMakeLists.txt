set(CMAKE_CXX_STANDARD 17)
# Add source to this project's executable.
add_library (assetlib  
"mesh_asset.cpp"
"mesh_asset.h"
"texture_asset.h"
"texture_asset.cpp"
"asset_loader.h"
"asset_loader.cpp"
)

target_include_directories(assetlib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(assetlib PUBLIC json lz4)

#target_link_libraries(baker PUBLIC tinyobjloader stb_image json lz4 assetlib glm)