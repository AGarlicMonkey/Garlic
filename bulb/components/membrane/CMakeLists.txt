set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/Membrane)

#Library
add_library(
    BulbMembrane SHARED
        ${INCLUDE}/Application.hpp
        ${SOURCE}/Application.cpp

        ${INCLUDE}/Log.hpp
        ${SOURCE}/Log.cpp
        ${INCLUDE}/MembraneLog.hpp

        ${INCLUDE}/EditorTypes.hpp

        ${INCLUDE}/MessageBase.hpp
        ${INCLUDE}/MessageHandler.hpp
        ${SOURCE}/MessageHandler.cpp
        ${INCLUDE}/Messages.hpp

        ${INCLUDE}/NameComponent.hpp

        ${INCLUDE}/EditorLayer.hpp
        ${SOURCE}/EditorLayer.cpp
        ${INCLUDE}/RuntimeLayer.hpp
        ${SOURCE}/RuntimeLayer.cpp
        
        ${INCLUDE}/ViewportSurface.hpp
        ${SOURCE}/ViewportSurface.cpp

        ${INCLUDE}/Scene.hpp
		${INCLUDE}/Scene.inl
		${SOURCE}/Scene.cpp
)

#Include
target_include_directories(
    BulbMembrane

    PUBLIC
        include   
)

#Libraries
target_link_libraries(
    BulbMembrane

    PRIVATE
        Clove
)

#Definitions
target_compile_definitions(
    BulbMembrane

    PRIVATE
        ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

#Properties
set_target_properties(
    BulbMembrane
    
    PROPERTIES 
        COMMON_LANGUAGE_RUNTIME ""
        VS_DOTNET_REFERENCES "System;PresentationCore;PresentationFramework;WindowsBase"
        CXX_STANDARD 17
)