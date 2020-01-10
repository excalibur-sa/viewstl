# [Go here for development!](https://gitlab.com/andrewhall649/viewstl)


# viewstl
### An STL (3D Model) viewer originally made by Doug Dingus
This modified version is mostly just to get the GNU/Linux copy into a better state. There are better STL viewers out there that use more modern OpenGL versions but are bloated. Use those instead.

Originally made by Doug Dingus (potatohead on SourceForge). Original copyright ~2004 to him under the GPLv2.
https://sourceforge.net/projects/viewstl/

If for whatever silly reason you're on Windows or IRIX in $CURRENT_YEAR you can find the code and build files for those versions in [viewstl_original.tar.gz](viewstl_original.tar.gz).

---

#### Planned Features/Changes
- [ ] Move from GLUT to SDL2.
- [ ] Add support for OpenGL3 rendering.
- [x] Add support for automatically detecting and displaying changes to the currently viewed STL file.
- [ ] Extend auto-reload to support FreeBSD.
- [ ] Possibly a name change with support for more formats.
- [x] Support for binary STL files.

*Note: Reloading is only supported on Linux. FreeBSD auto-reload will be added in the future. It's also worth noting that there may still be some bugs in the auto-reload feature, namely race conditions between the file being fully written and viewstl reading it.*
