# Docker で autoware_lanelet2_map_validator を使う

English: [how_to_use.md](./how_to_use.md)

Docker Desktop (Windows・macOS) または Docker Engine 23.0 以降 (Linux) が必要です．
コマンドは PowerShell (Windows) またはターミナル (macOS) で実行してください．

## ビルド

`Dockerfile` のあるフォルダで実行します．`1.8.2` はリリースのバージョンに置き換えてください．

```shell
docker build --build-arg VERSION=1.8.2 -t autoware_lanelet2_map_validator .
```

## 実行

地図のあるフォルダで実行します．そのフォルダがコンテナ内の `/data` になります．

GUI: 次のコマンドを実行し，ウェブブラウザで <http://localhost:6080> を開きます．

```shell
docker run --rm -it -p 127.0.0.1:6080:6080 -v "${PWD}:/data" autoware_lanelet2_map_validator
```

CLI: イメージ名の後ろのオプションは `autoware_lanelet2_map_validator` に渡されます．

```shell
docker run --rm -v "${PWD}:/data" autoware_lanelet2_map_validator -p mgrs -m /data/lanelet2_map.osm -i /opt/autoware_lanelet2_map_validator/map_requirements/autoware_requirement_set.json -o /data
```

### Linux: GUI を通常のウィンドウで表示する

```shell
xhost +SI:localuser:"$(id -un)"
docker run --rm -it --user "$(id -u):$(id -g)" -e DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix -v "${PWD}:/data" autoware_lanelet2_map_validator
```
