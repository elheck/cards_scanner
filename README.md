# cards_scanner


# Build and run
```bash
./rebuild.sh        # full clean configure + build using architecture-specific lockfile
./update_lock.sh    # refresh conan-<arch>.lock with current profiles (default profile unless specified)
./run_test.sh       # run integration tests
./build.sh          # incremental build (if provided)
```

## Dependency Management (Conan)
This project uses Conan 2 with architecture-specific lockfiles for reproducible builds.

Artifacts:
- `conanfile.txt`: Declares high-level requirements and options.
- `conan-x86_64.lock`: Generated lockfile for x86_64 systems (CI/servers).
- `conan-armv8.lock`: Generated lockfile for ARM64 systems (Apple Silicon, Raspberry Pi).
- `update_lock.sh`: Regenerates the architecture-specific lockfile.

Normal workflow:
1. Build (will reuse pinned revisions from appropriate arch lockfile):
	```bash
	./rebuild.sh
	```
2. Update dependencies (intentional upgrade):
	```bash
	./update_lock.sh            # generates conan-<arch>.lock for current system
	git add conan-*.lock
	git commit -m "chore(deps): update lockfile"
	```

CI validates the lockfile by attempting a locked install; if a dependency drift occurs without updating the lockfile, the build fails. The system automatically selects the appropriate lockfile based on the target architecture.

# Contract:
This is running on a raspi. 
- On start up a bin config (bin numbers to use, fullness of bins, card kind) should be sent to the device from a pc
- After ocr the name, set, collectors number, image hash, bin identifier should be sent by mqtt or similar to pc to check with database.


# Set up Raspi using ansible
### Prerequisite:
- SD card with recent raspian
- SD card is set up and expanded
- SSH Key was added for passwordless access using ssh-copy-id
- LC_ALL locale was set to utf-8


# TODO for isntall ansible
install conan
add to path
new profile detect


# Notes for HW build
- We want most likely Off-axis bright field lighting with an 18-20 degree tilt in the light to rmeove hot spots
