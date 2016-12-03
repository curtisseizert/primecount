Release process
===============

Clone or fork primecount:
```sh
$ git clone git://github.com/kimwalisch/primecount.git
```

Build primecount using:
```sh
$ ./build.sh
```

### Adding a new prime counting function implementation

* Add new function signature e.g. ```pi_lmo(x)``` to [include/primecount.hpp](include/primecount-internal.hpp)
* Create source file ```src/lmo/pi_lmo.cpp``` with the new function
* Add ```src/lmo/pi_lmo.cpp``` to [Makefile.am](Makefile.am)
* Add ```src\lmo\pi_lmo.obj``` to [Makefile.msvc](Makefile.msvc)
* Add ```pi_lmo(x)``` to [src/app/main.cpp](src/app/main.cpp)
* Add ```pi_lmo(x)``` to ```bool test()``` in [src/test.cpp](src/test.cpp)
* Add ```--lmo``` command-line option summary to [src/app/help.cpp](src/app/help.cpp)
* Add ```OPTION_LMO``` to [src/app/cmdoptions.hpp](src/app/cmdoptions.hpp)
* Add ```OPTION_LMO``` to optionMap in [src/app/cmdoptions.cpp](src/app/cmdoptions.cpp)

### Versioning

Increase the primecount version and the primecount libtool version before
each new release. The ```update_version.sh``` script automatically
updates the version (and release date) in all files.

```sh
cd scripts

# Usage example: update primecount version to 1.2.3 and 
# update primecount libtool version to 7:0:0
./update_version.sh 1.2.3 7:0:0
```
[This page](https://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html)
explains libtool versioning.

### Release process

* Run tests using ```make check```
* Increase version number (see <a href="#versioning">Versioning</a>)
* Build statically linked primecout binaries and upload them to [https://bintray.com/kimwalisch/primecount](https://bintray.com/kimwalisch/primecount)
* Update _**Precompiled binaries**_ section in [README.md](README.md)
* Update [ChangeLog](ChangeLog)
* Tag the new release in git
* Create new release tarball using ```make dist``` and upload it to [https://bintray.com/kimwalisch/primecount](https://bintray.com/kimwalisch/primecount)
