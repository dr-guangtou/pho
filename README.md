# Pho - Lightweight Image Viewer

Pho is a lightweight image viewer written in C using GTK2.

I hope you enjoy pho, the lightweight image viewer!

More information on pho, and the latest version, is available at:
http://www.shallowsky.com/software/pho/

See the man page or the web page for more details on how to use pho.

## macOS Compilation

### Prerequisites

Pho requires GTK2 development libraries. On macOS, install them via Homebrew:

```bash
brew install gtk+
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

## Warning

Pho uses the GTK2 toolkit, which is obsolete. I have not been
successful in rewriting Pho to use either GTK3 or GTK4, due to
apparently removed functionality and poor documentation.

As of mid-2025, GTK2 is still available for most platforms. But if it
becomes unavailable, I have been working on a replacement written in
Python using TkInter instead of GTK. If you want to help test, it's
currently available as tkpho in my metapho repository (and I've also
rewritten metapho to use Tk instead of GTK).
Please file issue reports on any problems you find.

-- Akkana

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
