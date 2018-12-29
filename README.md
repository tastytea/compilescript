**cppscript** allows you to execute C++ files as scripts.

It compiles the source file, stores the binary in `${XDG_CACHE_HOME}/cppscript/`
and executes it.

It does compile the file every time at the moment.

## Usage

Use `#!/usr/bin/env cppscript` as shebang and write the compiler arguments in
the line below with the prefix: `//cppscript:`

### Example

```C++
#!/usr/bin/env cppscript
//cppscript: -Wall -pedantic -Wextra

#include <iostream>

using std::cout;

int main(int argc, char *argv[])
{
    cout << "Hello world!\n";
    cout << argv[1] << '\n';
}
```

## Configuration



## Install

### From source

#### Dependencies

* C++ compiler (tested: [gcc](https://gcc.gnu.org/) 6/7/8,
  [clang](https://llvm.org/) 3/5/6)
* [cmake](https://cmake.org/) (at least 3.2)
* [libxdg-basedir](http://repo.or.cz/w/libxdg-basedir.git) (tested: 1.2)
* [libconfig++](https://github.com/hyperrealm/libconfig) (tested: 1.5)

#### Get sourcecode

Download the current
[release](https://schlomp.space/tastytea/cppscript/releases) and copy
[xdgcfg](https://schlomp.space/tastytea/xdgcfg) into `xdgcfg/`.

If you clone from git, be sure to `git submodule init` and
`git submodule update` afterwards. See the [submodules article in the git book]
(https://git-scm.com/book/en/v2/Git-Tools-Submodules#_cloning_submodules) for
further info.

#### Compile

```SH
mkdir build
cd build
cmake ..
make
make install
```

## Contributing

Contributions are always welcome. You can submit them as pull requests or via
email to `tastytea`@`tastytea.de`.

## License & Copyright

```PLAIN
Copyright © 2018 tastytea <tastytea@tastytea.de>.
License GPLv3: GNU GPL version 3 <https://www.gnu.org/licenses/gpl-3.0.html>.
This program comes with ABSOLUTELY NO WARRANTY. This is free software,
and you are welcome to redistribute it under certain conditions.
```
