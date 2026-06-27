<!-- Read Me of Nectar -->

<a href="LICENSE"><img src="https://img.shields.io/badge/LICENSE-Apache--2.0-blue.svg?style=for-the-badge" alt="License"></a>
![GitHub Stars](https://img.shields.io/github/stars/ne-app-eu/ncc?style=for-the-badge)

![CI](https://github.com/ne-app-eu/ncc/actions/workflows/nectar-asan-dev.yml/badge.svg)
![CI](https://github.com/ne-app-eu/ncc/actions/workflows/nectar-dev.yml/badge.svg)

## Getting Started

### Quick Install (POSIX)

```sh
curl -fsSL http://install.nectar.nekernel.org | sh
```

### Requirements

- [Clang](https://clang.llvm.org/)
- [Git](https://git-scm.com/)
- [Boost](https://boost.org/) (1.90.0+)
- [NeBuild](https://github.com/ne-app-eu/nebuild)
- [Doxygen](https://www.doxygen.nl/)
- GNU CoreUtils
- [Git](https://git-scm.com/)
- [OCL.TProc](https://github.com/ocl-foss/tproc) (1.61.0+)

### Building

Run the following:

```sh
git clone -j8 git@github.com:ne-app-eu/ncc.git
cd nectar
# Either build the debugger or compiler libraries/tools using nebuild.
```

### Community

Join Ne.app's community [discord](https://discord.gg/uD76Qweght) to chat with contributors.

---

<div align="center">
  <sub>
    &copy; 2023-2026 Amlal El Mahrouss. Licensed under the Apache 2.0 license.
  </sub>
</div>
