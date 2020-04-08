# Jetbrains API

You can find the Dolphin plugin here:
https://github.com/alex1701c/JetBrainsDolphinPlugin

And the KRunner plugin here:
https://github.com/alex1701c/JetBrainsRunner

### Usage for other projects:

CMake config:
```cmake
add_subdirectory(jetbrains-api)
target_link_libraries(my_target jetbrains_api_static)
```
And in your code:
```c++
#include "jetbrains-api/export.h"
...
 QList<JetbrainsApplication *> apps = JetbrainsAPI::fetchApplications();
```

Feedback and pull requests are appreciated 😃.