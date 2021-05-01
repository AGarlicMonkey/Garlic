set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source)

#Library
add_library(
    BulbUI SHARED

    ${SOURCE}/ViewModel.cs
)

#Target properties
set_target_properties(BulbUI PROPERTIES 
        VS_DOTNET_REFERENCES "Microsoft.CSharp;System;"
)