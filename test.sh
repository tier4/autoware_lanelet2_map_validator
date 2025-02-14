#!/bin/bash

# Run the awk script
awk -v start_word="https://github.com/tier4/autoware_lanelet2_map_validator/issues/6" '
BEGIN { found=0 }
{
  if (found) {
    gsub("tier4/autoware_lanelet2_map_validator", "autowarefoundation/autoware_tools")
  }
  if ($0 ~ start_word) { found=1 }
  print $0
}' autoware_lanelet2_map_validator/CHANGELOG.rst >tmpfile.rst && mv tmpfile.rst autoware_lanelet2_map_validator/CHANGELOG.rst

echo "Updated CHANGELOG:"
