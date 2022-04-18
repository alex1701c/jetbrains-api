# Jetbrains API

This project contains methods/classes to detect Jetbrains IDEs that are installed on the system and parse the recently used projects from them.

You can find the Dolphin plugin here:
https://github.com/alex1701c/JetBrainsDolphinPlugin

And the KRunner plugin here:
https://github.com/alex1701c/JetBrainsRunner

### Usage for other projects:

It is recommended to include this project as a git-submodule.

You can include it in the CMake code using:
```cmake
add_subdirectory(jetbrains-api)
target_link_libraries(my_target jetbrains_api_static)
```
And in your C++-code:
```c++
#include "jetbrains-api/export.h"
...
 QList<JetbrainsApplication *> apps = JetbrainsAPI::fetchApplications();
```

Feedback and pull requests are appreciated 😃.
