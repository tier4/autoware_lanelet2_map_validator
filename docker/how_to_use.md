# Running autoware_lanelet2_map_validator with Docker

日本語版: [how_to_use_ja.md](./how_to_use_ja.md)

Requires Docker Desktop (Windows/macOS) or Docker Engine 23.0+ (Linux).
Run the commands in PowerShell (Windows) or Terminal (macOS).

## Build

Run in the folder containing `Dockerfile`. Replace `1.8.2` with the release version.

```shell
docker build --build-arg VERSION=1.8.2 -t autoware_lanelet2_map_validator .
```

## Run

Run in the folder containing your maps. It becomes `/data` in the container.

GUI: run the command below, then open <http://localhost:6080> in your web browser.

```shell
docker run --rm -it -p 127.0.0.1:6080:6080 -v "${PWD}:/data" autoware_lanelet2_map_validator
```

CLI: options after the image name are passed to `autoware_lanelet2_map_validator`.

```shell
docker run --rm -v "${PWD}:/data" autoware_lanelet2_map_validator -p mgrs -m /data/lanelet2_map.osm -i /opt/autoware_lanelet2_map_validator/map_requirements/autoware_requirement_set.json -o /data
```

### Linux: show the GUI as a normal window

```shell
xhost +SI:localuser:"$(id -un)"
docker run --rm -it --user "$(id -u):$(id -g)" -e DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -v "${PWD}:/data" autoware_lanelet2_map_validator
```
