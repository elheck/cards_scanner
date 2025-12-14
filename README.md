# Card Scanner

A C++17 application for detecting and processing Magic: The Gathering (MTG) cards using OpenCV. Designed to run on Raspberry Pi for automated card scanning and cataloging.

---

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Project Structure](#project-structure)
- [Dependencies](#dependencies)
- [Building](#building)
- [Usage](#usage)
- [Testing](#testing)
- [Raspberry Pi Deployment](#raspberry-pi-deployment)
- [Contract / Communication Protocol](#contract--communication-protocol)

---

## Overview

The Card Scanner application captures images of MTG cards, detects card boundaries using computer vision techniques, applies perspective correction (warping), and outputs normalized card images ready for OCR or database matching.

### Key Features

- **Card Detection**: Automatically detects card boundaries in images using contour analysis
- **Perspective Correction**: Warps detected cards to a standardized 480x680 pixel format
- **Tilt Correction**: Framework for correcting rotated/tilted cards (extensible)
- **Cross-Platform**: Build and test on desktop, deploy to Raspberry Pi

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         Main Application                         │
│                           (main.cpp)                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                   │
│  ┌──────────────────────┐     ┌──────────────────────────────┐  │
│  │    CardDetector      │     │       PicHelper              │  │
│  │   (detection/)       │     │       (misc/)                │  │
│  │                      │     │                              │  │
│  │  - loadImage()       │     │  - displayResults()          │  │
│  │  - processCards()    │     │  - saveResults()             │  │
│  │  - detectCards()     │     │                              │  │
│  │  - warpCard()        │     └──────────────────────────────┘  │
│  │  - sortCorners()     │                                        │
│  └──────────────────────┘                                        │
│                                                                   │
│  ┌──────────────────────┐                                        │
│  │   TiltCorrector      │                                        │
│  │   (detection/)       │                                        │
│  │                      │                                        │
│  │  - correctCardTilt() │                                        │
│  └──────────────────────┘                                        │
│                                                                   │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
                    ┌──────────────────┐
                    │     OpenCV       │
                    │  (Image Processing)│
                    └──────────────────┘
```

### Processing Pipeline

1. **Image Loading** – Load source image from file
2. **Undistortion** – Apply camera calibration (placeholder for future implementation)
3. **Card Detection** – Convert to grayscale, threshold, find contours, identify quadrilaterals
4. **Corner Sorting** – Order corners as: top-left, top-right, bottom-right, bottom-left
5. **Perspective Warp** – Transform to normalized 480×680 pixel output
6. **Output** – Display or save the processed card image

---

## Project Structure

```
cards_scanner/
├── CMakeLists.txt              # Main CMake configuration
├── build.sh                    # Incremental build script
├── rebuild.sh                  # Clean rebuild script  
├── run_test.sh                 # Test execution script
├── run_ansible.sh              # Raspberry Pi deployment script
│
├── include/                    # Header files
│   ├── detection/
│   │   ├── card_detector.hpp   # CardDetector class definition
│   │   └── tilt_corrector.hpp  # Tilt correction function declaration
│   └── misc/
│       └── pic_helper.hpp      # Image display/save utilities
│
├── src/                        # Source files
│   ├── main.cpp                # Application entry point
│   ├── detection/
│   │   ├── card_detector.cpp   # Card detection implementation
│   │   └── tilt_corrector.cpp  # Tilt correction (stub)
│   └── misc/
│       └── pic_helper.cpp      # Image utilities implementation
│
├── tests/                      # Test suite
│   ├── integration/
│   │   ├── CMakeLists.txt      # Test build configuration
│   │   └── test_card_detection.cpp  # Integration tests
│   └── sample_cards/           # Test images (JPG)
│
├── ansible/                    # Raspberry Pi provisioning
│   ├── set_up_raspi.yml        # Main playbook
│   ├── inventory.ini           # Target hosts
│   ├── example_inventory.ini   # Inventory template
│   └── plays/
│       ├── apt_update_upgrade.yml
│       ├── install_opencv_dependencies.yml
│       └── download_and_install_opencv.yml
│
└── build/                      # Build output (generated)
    ├── card_scanner            # Executable
    └── ...                     # CMake/Conan generated files
```

### Core Components

| Component | File | Description |
|-----------|------|-------------|
| **CardDetector** | `detection/card_detector.hpp/cpp` | Main class for loading images and detecting cards. Uses OpenCV contour detection to find quadrilaterals, then applies perspective transform to normalize card dimensions. |
| **TiltCorrector** | `detection/tilt_corrector.hpp/cpp` | Utility for correcting card rotation (placeholder for future implementation). |
| **PicHelper** | `misc/pic_helper.hpp/cpp` | Utilities for displaying results (`displayResults`) and saving processed cards to timestamped directories (`saveResults`). |

---

## Dependencies

| Dependency | Version | Purpose |
|------------|---------|---------|
| **CMake** | ≥ 3.18 | Build system |
| **C++ Compiler** | C++17 | GCC/Clang with C++17 support |
| **OpenCV** | Latest | Image processing and computer vision |
| **Google Test** | 1.13.0 | Unit/Integration testing |
| **Conan** | 2.x | Dependency management |

### Installing Dependencies (Desktop/Linux)

```bash
# Install Conan (if not present)
pip install conan

# OpenCV (system package)
sudo apt install libopencv-dev

# CMake
sudo apt install cmake
```

---

## Building

### Quick Build (Incremental)

```bash
./build.sh
```

Options:
- `-v` : Verbose output

### Clean Rebuild

```bash
./rebuild.sh
```

This removes the `build/` directory and performs a fresh build.

### Manual Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

The compiled executable will be at `build/card_scanner`.

---

## Usage

### Command Line Interface

```bash
./build/card_scanner [options]
```

### Options

| Option | Description |
|--------|-------------|
| `-f, --file <path>` | Process a card from an image file |
| `-h, --help` | Show help message |

### Examples

```bash
# Process a single card image
./build/card_scanner -f /path/to/card_image.jpg

# Show help
./build/card_scanner --help
```

### Output

When processing an image, the application will:
1. Load the image
2. Detect the card boundary
3. Apply perspective correction
4. Display the normalized card image in a window (press any key to close)

---

## Testing

### Run Integration Tests

```bash
./run_test.sh
```

Or manually:

```bash
cd build
ctest -V
```

### Test Structure

- **Integration Tests** (`tests/integration/test_card_detection.cpp`):
  - Loads all sample card images from `tests/sample_cards/`
  - Processes each image through the detection pipeline
  - Saves results to timestamped subdirectories

### Adding Test Images

Place new test images in `tests/sample_cards/` (JPG format recommended).

---

## Raspberry Pi Deployment

### Prerequisites

1. SD card with Raspbian OS installed and expanded
2. SSH enabled on the Raspberry Pi
3. SSH key copied for passwordless access:
   ```bash
   ssh-copy-id pi@your_raspi.local
   ```

### Configure Inventory

1. Copy the example inventory:
   ```bash
   cp ansible/example_inventory.ini ansible/inventory.ini
   ```

2. Edit `ansible/inventory.ini` with your Raspberry Pi hostname:
   ```ini
   [raspi]
   your_raspi.local ansible_user=pi
   ```

### Run Deployment

```bash
# Full deployment (installs OpenCV and dependencies)
./run_ansible.sh

# Lint playbooks only
./run_ansible.sh lint
```

### What Gets Installed

The Ansible playbooks will:
1. Update and upgrade system packages
2. Install OpenCV build dependencies
3. Download and compile OpenCV from source

---

## Contract / Communication Protocol

> **Note**: This section describes the intended production behavior for the Raspberry Pi deployment.

### Startup Configuration

On startup, the device should receive a configuration from a PC containing:
- **Bin numbers** to use
- **Bin fullness** status

### Post-OCR Communication

After processing a card (OCR), the device should send the following data via MQTT (or similar protocol) to the PC:
- Card **name**
- Card **set**
- **Collector's number**
- **Image hash**
- **Bin identifier**

This allows the PC to verify the card against a database.

---

## License

*Add license information here*

---

## Contributing

*Add contribution guidelines here*