with import <nixpkgs> {};
mkShell {
  nativeBuildInputs = [ pkg-config ];
  LOCALE_ARCHIVE = "${glibcLocales}/lib/locale/locale-archive";
  buildInputs = [ gnumake
                  gcc12
                  clang_16 
                  lld_16
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
