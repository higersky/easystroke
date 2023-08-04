# easystroke
Fork of thjaeger / easystroke with all of the known patches. Moreover,

- Fix deprecated usages of Gtk (GdkScreen-related APIs, deprecated properties, etc.)
- Fix most memory leaks, undefined behaviors and redundant allocations (AddressSanitizer checked)
- Use modern C++/C standards to compile
- Change the behavior of gesture feedback.
  - The feedback tooltip is always shown on the lower center of the screen. Because the original settings often wrongly place the tooltip on some random locations.
  - Use AppIndicators instead of deprecated GtkStatusIcon to get a better icon resolution on modern displays. Due to the limitations of AppIndicators, Feedback on the tray icon is removed.

## Build

You can use the following instructions to install on a Debian-based distribution:

```bash
sudo apt-get install -y help2man intltool libatkmm-1.6-dev libboost-serialization-dev libboost-serialization-dev libboost-dev libcairomm-1.0-dev libfontenc-dev libglibmm-2.4-dev libgtkmm-3.0-dev libpangomm-1.4-dev libsigc++-2.0-dev libxfont-dev libxkbfile-dev xserver-xorg-dev libappindicator-dev
wget https://github.com/higersky/easystroke/archive/master.tar.gz
tar zxf master.tar.gz
cd easystroke-master
make
sudo make install
# call ./build.sh if you want to create a deb package
```
