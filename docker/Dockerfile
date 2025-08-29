# Start from Autoware core-devel (has ROS2 + Autoware dependencies pre-installed)
FROM ghcr.io/autowarefoundation/autoware:core-devel

# Install Qt X11 dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    libxcb-xinerama0=1.14-3ubuntu3 \
    libxkbcommon-x11-0=1.4.0-1 \
    libxcb-cursor0=0.1.1-4ubuntu1 \
    libxrender1=1:0.9.10-1build4 \
    libxext6=2:1.3.4-1build1 \
    libxi6=2:1.8-1build1 \
    libgl1-mesa-glx=23.0.4-0ubuntu1~22.04.1 \
    libgl1-mesa-dri=23.2.1-1ubuntu3.1~22.04.3 \
    && rm -rf /var/lib/apt/lists/*

# Run the same build-and-clean process Autoware uses
# hadolint ignore=SC1091
RUN --mount=type=bind,source=./autoware_lanelet2_map_validator,target=/autoware/src/autoware_lanelet2_map_validator \
    --mount=type=cache,target=/ccache \
    source /opt/autoware/setup.bash && \
    rosdep update && \
    rosdep install -y --from-paths src --ignore-src --rosdistro "$ROS_DISTRO" && \
    /autoware/build_and_clean.sh /ccache /opt/autoware "--packages-up-to autoware_lanelet2_map_validator"

ENTRYPOINT ["/ros_entrypoint.sh"]
CMD ["bash"]
