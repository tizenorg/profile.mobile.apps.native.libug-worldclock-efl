Name:       libug-worldclock-efl
Summary:    Time Zone setup UI gadget
Version:    0.2
Release:    1
Group:      Applications/Core Applications
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz


%if "%{?tizen_profile_name}" == "wearable"
ExcludeArch: %{arm} %ix86 x86_64
%endif

%if "%{?tizen_profile_name}"=="tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(db-util)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(ui-gadget-1)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(icu-i18n)
BuildRequires:  cmake
BuildRequires:  edje-tools
BuildRequires:  gettext-tools
BuildRequires:  pkgconfig(efl-extension)
BuildRequires:  pkgconfig(capi-system-system-settings)
BuildRequires:  pkgconfig(tapi)
BuildRequires:  pkgconfig(libtzplatform-config)
BuildRequires:  pkgconfig(notification)

%description
This is UI gadget for configuration time zone of device

%prep
%setup -q

%build
%define PREFIX "%{TZ_SYS_RO_UG}"

#TODO: Use macros TZ_USER_DATA when it will work
cmake . -DCMAKE_INSTALL_PREFIX=%{PREFIX} \
		-DTZ_SYS_RO_PACKAGES=%{TZ_SYS_RO_PACKAGES} \
		-DTZ_SYS_DATA=%{TZ_SYS_DATA} \
		-DTZ_USER_DATA=/opt/usr/data \
		-DTZ_SYS_DB=%{TZ_SYS_DB}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp LICENSE %{buildroot}/usr/share/license/%{name}
%make_install

mkdir -p %{buildroot}/opt/usr/data/clock

%post
/sbin/ldconfig

mkdir -p %{TZ_SYS_RO_UG}/bin/
ln -sf /usr/bin/ug-client %{TZ_SYS_RO_UG}/bin/worldclock-efl

%postun -p /sbin/ldconfig

%files
%manifest libug-worldclock-efl.manifest
%defattr(-,root,root,-)
%{TZ_SYS_RO_UG}/lib/libug-worldclock-efl.so
%{TZ_SYS_RO_UG}/res/edje/ug-worldclock-efl/ug_worldclock.edj
%{TZ_SYS_RO_UG}/res/edje/ug-worldclock-efl/ug_worldclock_button.edj
%{TZ_SYS_RO_UG}/res/locale/*/LC_MESSAGES/ug-worldclock-efl.mo
%{TZ_SYS_RO_UG}/res/images/ug-worldclock-efl/*
%{TZ_SYS_SHARE}/license/%{name}
%{TZ_SYS_RO_PACKAGES}/worldclock-efl.xml
%{TZ_SYS_DATA}/setting/tzlist.ini
%{TZ_SYS_DB}/.worldclock.db
%{TZ_SYS_DB}/.worldclock.db-journal

#TODO: Use macros TZ_USER_DATA when it will work
/opt/usr/data/clock/tzlist.ini
