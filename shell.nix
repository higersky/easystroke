with import <nixpkgs> {};
mkShell {
  nativeBuildInputs = [ pkg-config ];
  buildInputs = [ gnumake 
                  clang_16 
                  boost 
                  pkgconfig 
                  gtkmm3 
                  dbus-glib 
                  xorg.libX11 
                  xorg.libXtst 
                  xorg.libXi 
                  xorg.xorgserver 
                  xorg.libXfixes 
                  vala
                  gettext 
                  locale
                  iconv
                  glibcLocales
                  glibcLocalesUtf8
                ];
}
