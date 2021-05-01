# Garlic

The goal of Garlic is to create an extremely modular engine where it's libraries can be used independently.

Please note that the engine is still in very early development and so the API will fluctuate wildly and without warning!

The Garlic game engine is comprised of the following components:

|Component|Language|Description|
|-|-|-|
|Clove|C++|Clove is the engine itself. This is where all the platform / graphics / rendering / systems etc. are|
|Bulb|C#|Bulb is the editor. It contains all of the view logic|

Note: Bulb is only currently supported on Windows.

Currently, **Clove** supports the following platforms:

|**Platform**|**API**|
|-|-|
|Windows|Vulkan|
|Linux|Vulkan|
|MacOS(WIP)|Metal(WIP)|

## Building

Garlic is built using CMake.  
Inside the project directory, use the following commands to generate and build the project:
```
mkdir build
cd build
cmake ..
cmake --build .
```

## Linking

If using Garlic as a `sub_directory` inside a CMakeLists then OpenAL will need to be placed into the same directory as your exe.  
This can be achieved by putting the following snippet inside the CMakeLists that builds your executable:
```
add_custom_command(TARGET yourprog POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:OpenAL> $<TARGET_FILE_DIR:yourprog>
)
```

If getting an error along the lines of '' when linking against Clove. Put the following command in your top most CMakeLists:
```
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
```