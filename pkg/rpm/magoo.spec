summary: Image analysis and density quantification
name: magoo
version: 0
release: 1
license: GPL3
group: Applications/Science
source: https://github.com/BehaviorEnterprises/Magoo.git
url: https://wiki.BehaviorEnterprises.com
vendor: Behavior Enterprises
packager: Jesse McClure jesse [at] mccluresk9 [dot] com
requires: cairo, desktop-file-utils
buildrequires: git, cairo-devel, gcc, libX11-devel, pkgconfig
prefix: /usr

%description
TODO: gtk2 deps

%prep
%setup -c

%build
make

%install
make "DESTDIR=${RPM_BUILD_ROOT}" install

%clean
rm -rf "${RPM_BUILD_ROOT}"

%post
update-desktop-database -q

%postun
update-desktop-database -q

%files
TODO
