### For bulding mdal_dhi_dfsu_driver.dll :

Requirement :
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

* Open the solution with Visual Studio
* Install the nuget package DHI.EUM and DHI.DFS from "Manage NuGet Packages ..."
or from the console (menu "Tools"->"Nugget Package Manager"->"Package Manager Console") enter :
```
Install-Package DHI.EUM -ProjectName mdal_dhi_dfsu_driver
Install-Package DHI.DFS -ProjectName mdal_dhi_dfsu_driver
```

* To build the DHI driver, the MFC needs to be installed and you need to change project configuration :
"Properties"->"Configuration Properties"->"Advanced"-> "Use of MFC" -> set "Use MFC in a Shared DLL"

* Start building ->"Build"

----

### For testing under Visual Studio IDE:

Set the following environment for the "mdal_dhi_driver_test" project (in the project Properties : "Configuration Properties"->"Debugging"-> "Environment")
```
PATH=%PATH%;"Path to the dhi *.dll"
MDAL_DRIVER_PATH="Path to the mdal_dhi_dfsu_driver.dll"
```

Those two paths can be the path where was built the driver.


### Install the dll

To make MDAL available for using the driver, the file mdal_dhi_dfsu_driver.dll has to be in the folder defined by the environment variable MDAL_DRIVER_PATH 
***but also*** you need to add to the environment PATH the path to all the dhi *.dll files (that can be found in the output directory of the build).






