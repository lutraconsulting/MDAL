### For bulding mdal_dhi_dfsu_driver.dll and mdal_dhi_dfs2_driver:

Requirements:
* OS : Windows
* Visual Studio 16 Community

Follow this steps:
* Create and enter build directory:
```
mkdir build
cd build
```

* Generate the solution with CMAKE
```
cmake -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Rel ..
```

## For Dfsu driver

* Open the solution with Visual Studio
* Install the Nuget packages DHI.EUM and DHI.DFS from "Manage NuGet Packages ..."
or from the console (menu "Tools"->"Nugget Package Manager"->"Package Manager Console") enter :
```
Install-Package DHI.EUM -ProjectName mdal_dhi_dfsu_driver
Install-Package DHI.DFS -ProjectName mdal_dhi_dfsu_driver
```

* To build the DHI driver, the MFC needs to be installed and you need to change project configuration :
"Properties" -> "Configuration Properties"->"Advanced" -> "Use of MFC" -> set "Use MFC in a Shared DLL"

* Start building ->"Build"

## For Dfs2 driver

In Dfs2 format, the origin and the orientation of the mesh is always provided in Geograhic coordinates system while the coordinate system of 
the mesh could be a projected system. To convert origin and orientation in projected system, we need to use the .Net Framework Dhi.Projections 
also available as Nuget package.

First, you need to go to change the project configuration:
"Properties" -> "Configuration Properties" -> "Advanced" -> "Use of MFC" -> set "Use MFC in a Shared DLL"
"Properties" -> "Configuration Properties" -> "Advanced"-> "C++/CLI Properties" -> "Common Language Runtime Support" -> set "Common Language Runtime Support (/clr)"
"Properties" -> "Configuration Properties" -> "Advanced"-> "C++/CLI Properties" -> ".Net Target Framework Version" -> set the version as vX.X.X, for example "v4.7.2"
"Properties" -> "Configuration Properties" -> "C/C++"-> "Code Generation" -> "Enable C++ Exceptions" -> set "yes with SEH Exceptions (/EHa)"
"Properties" -> "Configuration Properties" -> "C/C++"-> "Code Generation" -> "Basic Run Time Checks" -> set "Default"

Then, for Dfs2 driver, you need to install Nuget packages DHI.EUM, DHI.DFS **and** DHI.Projections:

```
Install-Package DHI.EUM -ProjectName mdal_dhi_dfs2_driver
Install-Package DHI.DFS -ProjectName mdal_dhi_dfs2_driver
Install-Package DHI.Projections mdal_dhi_dfs2_driver
```

As Visual Studio do not configure automaticly the .Net assembly references for C++ project, in the file "packages.config" (in Solution Explorer, 
in the folder of the "project mdal_dhi_dfs2_driver"), you need to replace the following line:

```
<package id="DHI.Projections" version="19.1.0" targetFramework="native" />
```
by this one:
```
<package id="DHI.Projections" version="19.1.0" targetFramework="net472" />
```
'version' and 'targetFramework' could change depending of your configuration.

To finish, you must specify where the assembly dll is:
In the "Solution Explorer", in the "mdal_dhi_dfs2_driver" folder, right click on "References" -> "Add Reference" -> "Browse..." -> search and select "DHI.Projections.dll", 
that could be in the build directory under ".\packages\DHI.Projections.19.1.0\lib\netstandart2.0\".

Then you can build the project...
----

### For testing under Visual Studio IDE:

Set the following environment for the "mdal_dhi_driver_test" project (in the project Properties : "Configuration Properties"->"Debugging"-> "Environment")
```
PATH=%PATH%;"Path to the dhi *.dll"
MDAL_DRIVER_PATH="Path to the mdal_dhi_dfsX_driver.dll"
```

Those two paths can be the path where was built the driver.


### Install the dll

To make MDAL available for using the driver, the file mdal_dhi_dfsu_driver.dll has to be in the folder defined by the environment variable MDAL_DRIVER_PATH 
***but also*** you need to add to the environment PATH the path to all the dhi *.dll files (that can be found in the output directory of the build).

For Dfs2, the "DHI.Projections.dll" and "DHI.Projections.xml" have to be in the executable folder (with "mdal_dhi_driver_test.exe" for testing).






