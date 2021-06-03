# Comm

## Dependencies
- opencv: not a specific version necessary (tested with >3.2 and >4.3), no opencv_contrib modules needed
- gcc: toolchain (for Linux and Windows (mingw)) <br/> 
Note: possibly working with the Visual Studio compiler as is... Build opencv (or use prebuild distributed files) for the OS and compiler you are using.

## Building
- Step 0:
Install Opencv like presented in the [official documentation](https://docs.opencv.org/master/df/d65/tutorial_table_of_content_introduction.html)

- Step 1:
```
cd <your_development_directory>
git clone https://github.com/AndreiCostinescu/Comm.git
cd Comm
mkdir build-release
cd build-release
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```
