include(CSharpUtilities)

#Create another project to enable C#
project(Garlic-Editor LANGUAGES CSharp)

set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source)

add_subdirectory(components)

#Executable
add_executable(
    Bulb
	    ${SOURCE}/EditorApp.xaml
        ${SOURCE}/EditorApp.xaml.cs

	    ${SOURCE}/AssemblyInfo.cs
        
	    ${SOURCE}/Views/MainWindow.xaml
		${SOURCE}/Views/MainWindow.xaml.cs
		${SOURCE}/Views/Vector3Box.xaml
		${SOURCE}/Views/Vector3Box.xaml.cs
		${SOURCE}/Views/Viewport.xaml
		${SOURCE}/Views/Viewport.xaml.cs
		
		${SOURCE}/ViewModels/ComponentMenuItemViewModel.cs
		${SOURCE}/ViewModels/EditorSessionViewModel.cs
		${SOURCE}/ViewModels/SceneViewModel.cs
		${SOURCE}/ViewModels/EntityViewModel.cs
		${SOURCE}/ViewModels/LogViewModel.cs
		${SOURCE}/ViewModels/ComponentViewModel.cs
		${SOURCE}/ViewModels/CollisionShapeComponentViewModel.cs
		${SOURCE}/ViewModels/RigidBodyComponentViewModel.cs
		${SOURCE}/ViewModels/TransformComponentViewModel.cs
)

#Libraries
target_link_libraries(
    Bulb

	PUBLIC
		BulbUI
        BulbMembrane   
)

#Cs properties
csharp_set_designer_cs_properties(
	${SOURCE}/AssemblyInfo.cs
)

csharp_set_xaml_cs_properties(
	${SOURCE}/Views/EditorApp.xaml
    ${SOURCE}/Views/EditorApp.xaml.cs

	${SOURCE}/Views/MainWindow.xaml
    ${SOURCE}/Views/MainWindow.xaml.cs
	
	${SOURCE}/Views/Vector3Box.xaml
	${SOURCE}/Views/Vector3Box.xaml.cs

	${SOURCE}/Views/Viewport.xaml
	${SOURCE}/Views/Viewport.xaml.cs
)

#File properties
set_source_files_properties(${SOURCE}/EditorApp.xaml PROPERTIES VS_XAML_TYPE "ApplicationDefinition")

#Target properties
set_target_properties(Bulb PROPERTIES
        VS_DOTNET_REFERENCES "Microsoft.CSharp;PresentationCore;PresentationFramework;System;System.Core;System.Data;System.Xaml;System.Xml;WindowsBase"
)