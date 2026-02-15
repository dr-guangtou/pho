# Pho - Lightweight Image Viewer

Pho is a lightweight image viewer written in C using GTK3.

More information on pho, and the latest version, is available at:
http://www.shallowsky.com/software/pho/

See the man page or the web page for more details on how to use pho.

## macOS Compilation

### Prerequisites

Pho requires GTK3 development libraries. On macOS, install them via Homebrew:

```bash
brew install gtk+3
```

### Building

After installing the dependencies, simply run:

```bash
make
```

If `pkg-config` cannot find the libraries, you may need to set the `PKG_CONFIG_PATH`:

```bash
export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:$PKG_CONFIG_PATH"
make
```

### Testing

Pho includes a comprehensive test suite using the Unity testing framework:

```bash
make test        # Run all tests (50 tests)
make test-unit   # Run unit tests only
make test-regression  # Run regression tests only
```

Tests are located in the `tests/` directory.

### Installation

To install system-wide (requires root):

```bash
sudo make install
```

Or, to install for the current user only (recommended), create a symlink to a directory in your PATH:

```bash
mkdir -p ~/.local/bin
ln -s "$(pwd)/pho" ~/.local/bin/pho
```

Then you can run `pho` from anywhere.

---

## Recent Changes

### 2026-02-15: GTK3 Migration Complete
- Successfully migrated from GTK2 to GTK3
- Full Cairo drawing system implementation
- Updated all deprecated APIs (signals, widgets, monitor detection)
- All 50 tests passing
- Runtime verified with actual images

### Security & Stability
- Fixed several security issues (buffer overflows, uninitialized variables)
- Added automated test suite (50 tests)
- Improved memory safety with NULL checks throughout

## Migration Notes

Pho has been successfully migrated from GTK2 to GTK3. The migration included:

- **Drawing System**: Replaced `gdk_pixbuf_render_to_drawable` with Cairo
- **Event Handling**: Updated `expose_event` to `draw` signal
- **Widget Accessors**: Replaced direct struct access with accessor functions
- **Monitor API**: Updated to use `GdkDisplay` and `GdkMonitor`
- **Input Handling**: Migrated to device-based input APIs

The codebase is now fully GTK3 compliant with zero deprecation warnings.

## License

Pho is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Pho is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with Pho (see COPYING); if not, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
