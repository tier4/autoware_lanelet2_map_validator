# How to use autoware_lanelet2_map_validator with Docker

You can use autoware_lanelet2_map_validator in a docker image.

## How to build

Follow these instructions to build a docker image with autoware_lanelet2_map_validator in it.

**(Assumption) Please install docker to your machine first. Installation methods will vary depending to your operation system.**

1. Go to the base directory of this repository.

   ```bash
   cd <DIRECTORY_TO_THIS_REPOSITORY>
   ```

2. Build the docker image

   ```bash
   docker build -t <IMAGE_NAME> -f docker/Dockerfile .
   ```

## How to use

### Use Case A: Use the GUI once

1. Configure display settings **(This may vary depending to your operation system!! This example demonstrates in the Ubuntu way**). Grant permission of X11 display to the local user

   ```bash
   xhost +local:root
   ```

2. Prepare a folder to pass to the map file (`.osm file`) (and map requirements `.json file` if needed) to the docker container.

   ```bash
   mkdir <FOLDER_NAME>
   mv <MAP_FILE> <FOLDERNAME>
   ```

3. Run a temporary docker container

   ```bash
   docker run -it --rm -e DISPLAY=$DISPLAY \
   -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
   -v <FOLDER_NAME>:/home/autoware_lanelet2_map_validator \
   <IMAGE_NAME> \
   ros2 run autoware_lanelet2_map_validator gui.py
   ```

   By this command, your files are shared to the docker container in the `/home/autoware_lanelet2_map_validator` directory.

4. Validate your map through the GUI. Note that the map requirements are already in the image, but you can bring your own files with your map by including them in the folder you've prepared before. You should not forget to set the `output_directory` to `/home/autoware_lanelet2_map_validator` to bring the results to your local machine.

5. Closing the GUI will automatically exit the docker container.

### Use Case B: Use it as a CLI

1. Prepare a folder to pass to the map file (`.osm file`) (and map requirements `.json file` if needed) to the docker container.

   ```bash
   mkdir <FOLDER_NAME>
   mv <MAP_FILE> <FOLDERNAME>
   ```

2. You can simply enter to the container by the following command. But do not forget to mount the folder to your map or you cannot pass the map file to the container.

   ```bash
   docker run -it --rm -v <FOLDER_NAME>:/home/autoware_lanelet2_map_validator <IMAGE_NAME>
   ```
