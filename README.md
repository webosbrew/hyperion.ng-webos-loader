# Hyperion.NG for webOS

Binaries are [available](https://repo.webosbrew.org/apps/org.webosbrew.hyperion.ng.loader) through [Homebrew Channel](https://github.com/webosbrew/webos-homebrew-channel).

## Building

### Requirements

* Linux host system
* [buildroot-nc4](https://github.com/openlgtv/buildroot-nc4/releases), a toolchain targeting webOS
* Git
* CMake
* npm

### Instructions

Build Hyperion.NG:
```
./build_hyperion_ng.sh
```

Build webOS frontend/service:
```
./build.sh
```

Both scripts take the environment variable `TOOLCHAIN_DIR`, defaulting to `$HOME/arm-webos-linux-gnueabi_sdk-buildroot`.

To provide a different path, run `export TOOLCHAIN_DIR=/your/toolchain/path` before executing the respective scripts.

Two other environment variables can also be used to configure `build_hyperion_ng.sh`:
- `HYPERION_NG_REPO` - Repository from which to obtain Hyperion.NG. Defaults to `https://github.com/hyperion-project/hyperion.ng`.
- `HYPERION_NG_BRANCH` - Which branch to check out from the above repo. Defaults to `master`.

## Related projects

- [Hyperion.NG](https://github.com/hyperion-project/hyperion.ng) - Ambient lighting service/daemon
- [hyperion-webos](https://github.com/webosbrew/hyperion-webos) - Video grabber for webOS
- [PicCap](https://github.com/TBSniller/piccap) - Frontend for hyperion-webos video grabber

## Credits

* @Smx-smx
* @TBSniller
* @Informatic
* @Mariotaku
* @Lord-Grey
* @Paulchen-Panther
* @chbartsch
* [throwaway96](https://github.com/throwaway96)
