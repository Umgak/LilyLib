# LilyLib
A bunch of of helper functions for my mods
### Features:
- One-line hooks
- Extensive error logger
- A few handy utility functions
### Usage:
Clone the project, and add it and its dependency refs to your project. Sadly, Visual Studio doesn't have a good way to automate this.
Then, you can call the functions like this:
```cpp
#include "LilyLib.hpp"
try {
  LilyLib::Memory::Initialize();
  LilyLib::Memory::Hook("80 f9 03 73 79", &DetourFunction);
  LilyLib::Memory::Patch("83 ff 03 b9 03 00 00 00", "?? ?? 06 ?? 06");
  LilyLib::Memory::Apply();
} catch (...) {
  // add your own error handler here, or use LilyLib::DetailedException
}
```
### Dependencies:
- [Pattern16](https://github.com/Dasaav-dsv/Pattern16/)
- [minhook](https://github.com/TsudaKageyu/minhook)
- [BytePatch](github.com/Umgak/BytePatch)
