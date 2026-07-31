# Third-party notices

Chess is licensed under the MIT License; see `LICENSE.txt` in the macOS app
resources or `copyright` in the Debian documentation directory.

The macOS application redistributes the dynamic libraries listed below.
The Debian package dynamically links compatible system packages instead of
copying these libraries, but ships the same notices for convenient review.
Versions are those audited for the v0.1.0 Apple Silicon release build.
Complete license texts are in the adjacent `licenses/` directory.
Exact upstream sources, checksums, Homebrew formula provenance, and the dylib
mapping are recorded in `macos-arm64-v0.1.0-sources.tsv`. The release also
provides a corresponding-source archive and the local relinking instructions
described in `LGPL_RELINKING.md`.

The license column describes the library files actually bundled in the App.
Some upstream source archives also contain command-line tools under other
licenses; those unbundled tools do not change the license of the listed dylib.

| Component | Version | Bundled macOS libraries | Distributed library license | Upstream | License files |
|---|---:|---|---|---|---|
| cairo | 1.18.4 | `libcairo*` | MPL-1.1 (selected) | https://cairographics.org/ | `cairo/` |
| cairomm | 1.18.1 | `libcairomm*` | LGPL-2.0-or-later | https://cairographics.org/cairomm/ | `cairomm/` |
| fontconfig | 2.18.2 | `libfontconfig*` | HPND-sell-variant AND Unicode-3.0 AND MIT-Modern-Variant AND MIT AND public-domain | https://www.freedesktop.org/wiki/Software/fontconfig/ | `fontconfig/` |
| FreeType | 2.14.3 | `libfreetype*` | FTL | https://freetype.org/ | `freetype/` |
| FriBidi | 1.0.16 | `libfribidi*` | LGPL-2.1-or-later | https://github.com/fribidi/fribidi | `fribidi/` |
| gdk-pixbuf | 2.44.7 | `libgdk_pixbuf*` | LGPL-2.1-or-later | https://gitlab.gnome.org/GNOME/gdk-pixbuf | `gdk-pixbuf/` |
| gettext (libintl only) | 1.0 | `libintl*` | LGPL-2.1-or-later | https://www.gnu.org/software/gettext/ | `gettext/` |
| GLib | 2.88.2 | `libglib*`, `libgio*`, `libgmodule*`, `libgobject*` | LGPL-2.1-or-later | https://docs.gtk.org/glib/ | `glib/` |
| glibmm | 2.88.1 | `libglibmm*`, `libgiomm*` | LGPL-2.1-or-later | https://gtkmm.org/ | `glibmm/` |
| Graphene | 1.10.8 | `libgraphene*` | MIT | https://ebassi.github.io/graphene/ | `graphene/` |
| Graphite2 | 1.3.15 | `libgraphite2*` | MIT (selected) | https://graphite.sil.org/ | `graphite2/` |
| GTK | 4.22.4 | `libgtk-4*` | LGPL-2.1-or-later | https://www.gtk.org/ | `gtk4/` |
| gtkmm | 4.22.0 | `libgtkmm*` | LGPL-2.1-or-later | https://gtkmm.org/ | `gtkmm4/` |
| HarfBuzz | 14.2.1 | `libharfbuzz*` | MIT | https://harfbuzz.github.io/ | `harfbuzz/` |
| libjpeg-turbo | 3.2.0 | `libjpeg*` | IJG AND Zlib AND BSD-3-Clause | https://libjpeg-turbo.org/ | `jpeg-turbo/` |
| libdatrie | 0.2.14 | `libdatrie*` | LGPL-2.1-or-later | https://linux.thai.net/projects/libthai | `libdatrie/` |
| libepoxy | 1.5.10 | `libepoxy*` | MIT | https://github.com/anholt/libepoxy | `libepoxy/` |
| libpng | 1.6.58 | `libpng16*` | libpng-2.0 | http://www.libpng.org/pub/png/libpng.html | `libpng/` |
| libsigc++ | 3.8.1 | `libsigc-3.0*` | LGPL-3.0-or-later | https://libsigcplusplus.github.io/libsigcplusplus/ | `libsigc++/` |
| libthai | 0.1.30 | `libthai*` | LGPL-2.1-or-later | https://linux.thai.net/projects/libthai | `libthai/` |
| LibTIFF | 4.7.2 | `libtiff*` | libtiff | https://libtiff.gitlab.io/libtiff/ | `libtiff/` |
| libX11 | 1.8.13 | `libX11*` | MIT | https://gitlab.freedesktop.org/xorg/lib/libx11 | `libx11/` |
| libXau | 1.0.12 | `libXau*` | MIT | https://gitlab.freedesktop.org/xorg/lib/libxau | `libxau/` |
| libxcb | 1.17.0 | `libxcb*` | MIT | https://gitlab.freedesktop.org/xorg/lib/libxcb | `libxcb/` |
| libXdmcp | 1.1.5 | `libXdmcp*` | MIT | https://gitlab.freedesktop.org/xorg/lib/libxdmcp | `libxdmcp/` |
| libXext | 1.3.7 | `libXext*` | MIT | https://gitlab.freedesktop.org/xorg/lib/libxext | `libxext/` |
| libXrender | 0.9.12 | `libXrender*` | MIT | https://gitlab.freedesktop.org/xorg/lib/libxrender | `libxrender/` |
| Pango | 1.58.0 | `libpango*` | LGPL-2.0-or-later | https://www.pango.org/ | `pango/` |
| pangomm | 2.56.2 | `libpangomm*` | LGPL-2.1-only | https://gtkmm.org/ | `pangomm/` |
| PCRE2 | 10.47 | `libpcre2-8*` | BSD-3-Clause WITH PCRE2-exception | https://www.pcre.org/ | `pcre2/` |
| Pixman | 0.46.4 | `libpixman-1*` | MIT | https://www.pixman.org/ | `pixman/` |
| WebP | 1.6.0 | `libwebp*`, `libsharpyuv*` | BSD-3-Clause | https://developers.google.com/speed/webp | `webp/` |
| XZ Utils (liblzma only) | 5.8.3 | `liblzma*` | 0BSD | https://tukaani.org/xz/ | `xz/` |
| Zstandard | 1.5.7_1 | `libzstd*` | BSD-3-Clause (selected) | https://facebook.github.io/zstd/ | `zstd/` |

This file is an inventory of redistributed software, not legal advice. If the
set of bundled libraries changes, regenerate and review the inventory before
publishing another release.

The official Developer ID App keeps Hardened Runtime Library Validation
enabled. A separate local-only relinking workflow lets a recipient replace an
interface-compatible LGPL dylib and ad-hoc sign their modified copy without
the release owner's certificate or private key. The official App does not use
the Disable Library Validation entitlement.

The macOS Cairo libraries are built from the audited Cairo 1.18.4 source with
its LZO feature explicitly disabled. The release bundle must not contain a
`liblzo2` library or an LZO Mach-O load command.
