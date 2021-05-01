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
        
	    ${SOURCE}/MainWindow.xaml
		${SOURCE}/MainWindow.xaml.cs

		${SOURCE}/Vector3Box.xaml
		${SOURCE}/Vector3Box.xaml.cs

		${SOURCE}/RelayCommand.cs
		
		${SOURCE}/EditorSessionViewModel.cs
		${SOURCE}/SceneViewModel.cs
		${SOURCE}/EntityViewModel.cs
		${SOURCE}/LogViewModel.cs

		${SOURCE}/ComponentViewModel.cs
		${SOURCE}/TransformComponentViewModel.cs
)

#Libraries
target_link_libraries(
    Bulb

	PUBLIC
		BulbUI
        Membrane   
)

#Cs properties
csharp_set_designer_cs_properties(
	${SOURCE}/AssemblyInfo.cs
)

csharp_set_xaml_cs_properties(
	${SOURCE}/EditorApp.xaml
    ${SOURCE}/EditorApp.xaml.cs
	
	${SOURCE}/Vector3Box.xaml
	${SOURCE}/Vector3Box.xaml.cs

	${SOURCE}/MainWindow.xaml
    ${SOURCE}/MainWindow.xaml.cs
)

#File properties
set_source_files_properties(${SOURCE}/EditorApp.xaml PROPERTIES VS_XAML_TYPE "ApplicationDefinition")

#Target properties
set_target_properties(Bulb PROPERTIES
        VS_DOTNET_REFERENCES "Microsoft.CSharp;PresentationCore;PresentationFramework;System;System.Core;System.Data;System.Xaml;System.Xml;WindowsBase"
)