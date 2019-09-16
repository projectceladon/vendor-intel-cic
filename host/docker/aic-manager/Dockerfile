FROM ubuntu-bionic-aic:1.0

ARG DEBIAN_FRONTEND=noninteractive

# copy MDC resources
COPY bin/graphic/liblxc_util.so /lib/liblxc_util.so
COPY bin/graphic/libutil_drm.so /lib/libutil_drm.so
COPY bin/graphic/libutil_x11.so /lib/libutil_x11.so
COPY bin/graphic/libutil_wayland.so /lib/libutil_wayland.so
COPY bin/graphic/display-daemon-drm /display-daemon-drm
COPY bin/graphic/display-daemon-x11 /display-daemon-x11
COPY bin/graphic/display-daemon-wld /display-daemon-wld

# copy mac80211_hwsim wifi resources
COPY scripts/mac80211_hwsim.sh /mac80211_hwsim.sh
COPY conf/simulated_hostapd.conf /simulated_hostapd.conf

# copy scripts
COPY scripts/CPU_info.py /CPU_info.py

# copy codes
COPY data /data

# copy entry process
COPY bin/aic-manager-entry /aic-manager-entry

ENTRYPOINT ["/aic-manager-entry"]

