Name:       org.tizen.worldclock-efl
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
%define PREFIX "%{TZ_SYS_RO_APP}/%{name}"

#TODO: Use macros TZ_USER_DATA when it will work
cmake . -DCMAKE_INSTALL_PREFIX=%{PREFIX} \
		-DTZ_SYS_RO_PACKAGES=%{TZ_SYS_RO_PACKAGES}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install



%files
%manifest org.tizen.worldclock-efl.manifest
%defattr(-,root,root,-)
%{TZ_SYS_RO_APP}/%{name}/*
%{TZ_SYS_RO_PACKAGES}/*
%license LICENSE


