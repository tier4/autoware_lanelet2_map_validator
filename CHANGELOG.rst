^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package autoware_lanelet2_map_validator
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Forthcoming
-----------
* Moved stuff in the autoware_lanelet2_map_validator directory to the base
* refactor: extract mapping_issues to main (`#13 <https://github.com/tier4/autoware_lanelet2_map_validator/issues/13>`_)
* Contributors: TaikiYamada4

1.0.0 (2025-02-10)
------------------
* chore: bump version to 1.0.0 (`#12 <https://github.com/tier4/autoware_lanelet2_map_validator/issues/12>`_)
* ci: removed check-directory-changes and added summarize-jobs (`#11 <https://github.com/tier4/autoware_lanelet2_map_validator/issues/11>`_)
* docs: improve README and create a Japanese version (`#9 <https://github.com/tier4/autoware_lanelet2_map_validator/issues/9>`_)
* Merge pull request `#6 <https://github.com/tier4/autoware_lanelet2_map_validator/issues/6>`_ from tier4/feat/manage_version_control
  feat: apply version control
* Added PRs to CHANGELOG.rst
* add build_depends.repos
* Fixed CI algorithm
* Merge branch 'main' into feat/manage_version_control
* Merge pull request `#8 <https://github.com/tier4/autoware_lanelet2_map_validator/issues/8>`_ from tier4/feat/explictly_check_prevent-no-label-execution
  ci: explictly check prevent no label execution
* Changed conditions
* check and output an error if label is wrong
* Merge branch 'main' into feat/manage_version_control
* fixed typo
* Merge pull request `#7 <https://github.com/tier4/autoware_lanelet2_map_validator/issues/7>`_ from tier4/ci/move_no_label_execution
  ci: change build-and-test-differential algorithm
* changed statements
* moved to an upper level
* Added debug lines
* changed GITHUB_ENV to GITHUB_OUTPUT
* move prevent-no-label-execution into build-and-test-differential
* updated documents
* remove CHANGELOG.rst brought from autoware_tools and make a new one
* added test code for version control functions
* add information of version control in documents
* apply verions to package and autoware_requirement_set.json
* add feature of version control
* Merge pull request `#4 <https://github.com/tier4/autoware_lanelet2_map_validator/issues/4>`_ from tier4/ci/refine_workflows
  ci: refine some workflows
* Fixed CODEOWNERS
* downgrade pr-labeler
* removed pull_request: paths
* Let build-and-test-differential to skip if there is no change in autoware_lanelet2_map_validator directory
* refine some workflows
* Merge pull request `#1 <https://github.com/tier4/autoware_lanelet2_map_validator/issues/1>`_ from tier4/dependabot/github_actions/actions/labeler-5
  chore(deps): bump actions/labeler from 4 to 5
* Merge pull request `#2 <https://github.com/tier4/autoware_lanelet2_map_validator/issues/2>`_ from tier4/chore/change_structure
  chore: extract documents from package
* Fixed pre-commit-config-optional.yaml
* Extract documents from pakcage
* Merge pull request `#3 <https://github.com/tier4/autoware_lanelet2_map_validator/issues/3>`_ from tier4/ci/initial_settings
  ci: set CI related files
* Removed .cspell-partial.json
* Set CI related files
* chore(deps): bump actions/labeler from 4 to 5
  Bumps [actions/labeler](https://github.com/actions/labeler) from 4 to 5.
  - [Release notes](https://github.com/actions/labeler/releases)
  - [Commits](https://github.com/actions/labeler/compare/v4...v5)
  ---
  updated-dependencies:
  - dependency-name: actions/labeler
  dependency-type: direct:production
  update-type: version-update:semver-major
  ...
* chore: bump version to 0.1.0 (`#209 <https://github.com/autowarefoundation/autoware_tools/issues/209>`_)
  * remove COLCON_IGNORE
  * unify version to 0.0.0
  * add changelog
  * 0.1.0
  * remove stop_accel_evaluator
  ---------
* chore: sync files (`#182 <https://github.com/autowarefoundation/autoware_tools/issues/182>`_)
  Co-authored-by: github-actions <github-actions@github.com>
* feat(lanelet2_map_validator): check local coordinates declaration (`#194 <https://github.com/autowarefoundation/autoware_tools/issues/194>`_)
  * Implement mapping.lane.local_coordinates_declaration
  * Added test for mapping.lane.local_coordinates_declaration
  * Added documents for mapping.lane.local_coordinates_declaration
  ---------
* feat(lanelet2_map_validator): add validator to check whether intersection lanelets have valid turn_direction tags (`#186 <https://github.com/autowarefoundation/autoware_tools/issues/186>`_)
  * Added validator which checks the "turn_direction" tag in intersection_areas
  * Added test code for mapping.intersection.turn_direction_tagging
  * Added document for mapping.intersection.turn_direction_tagging
  * Added mapping.intersection.turn_direction_tagging to autoware_requirement_set.json
  * Fixed spelling errors
  * Removed crosswalk boundaries from test map
  ---------
* docs(lanelet2_map_validator): update README (`#193 <https://github.com/autowarefoundation/autoware_tools/issues/193>`_)
  * Updated document
  * Added explanation about issue_code
  * Removed back ticks in the title
  * Fixed spelling issues
  ---------
* fix(lanelet2_map_validator): restore missing intersection lane and removed unnecessary linestrings from intersection test maps (`#188 <https://github.com/autowarefoundation/autoware_tools/issues/188>`_)
  * Restore missing linestring 197 and lanelet 49
  * Removed crosswalk remainings from intersection category test maps
  ---------
* docs(autoware_lanelet2_map_validator): update usage (`#191 <https://github.com/autowarefoundation/autoware_tools/issues/191>`_)
* feat(lanelet2_map_validator): generation script for new validators (`#180 <https://github.com/autowarefoundation/autoware_tools/issues/180>`_)
  * temporary commit
  * Added python script
  * Finished except docs
  * Added documents related stuff
  * Moved stuff to templates
  * Revised files to suit the current directory
  * Added arguments
  * Added prints
  * added #include <string> to test code
  * Fixed typo
  * Write explanation of create_new_validator.py
  * Added back slashes to example command
  * Enable the generation script to be run by `ros2 run`
  ---------
* chore(lanelet2_map_validator): automate test code compilation and categorize test codes (`#183 <https://github.com/autowarefoundation/autoware_tools/issues/183>`_)
  * Categorize test codes
  * Rewrite CMakeLists.txt so that contributors doesn't have to write the test code name in it
  ---------
* feat(autoware_lanelet_map_validator): add dangling reference checker to non existing intersection_area (`#177 <https://github.com/autowarefoundation/autoware_tools/issues/177>`_)
* chore(deps): bump tibdex/github-app-token from 1 to 2 (`#176 <https://github.com/autowarefoundation/autoware_tools/issues/176>`_)
  Bumps [tibdex/github-app-token](https://github.com/tibdex/github-app-token) from 1 to 2.
  - [Release notes](https://github.com/tibdex/github-app-token/releases)
  - [Commits](https://github.com/tibdex/github-app-token/compare/v1...v2)
  ---
  updated-dependencies:
  - dependency-name: tibdex/github-app-token
  dependency-type: direct:production
  update-type: version-update:semver-major
  ...
  Co-authored-by: dependabot[bot] <49699333+dependabot[bot]@users.noreply.github.com>
* ci(sync-files): utilize the sync-file-templates repository (`#174 <https://github.com/autowarefoundation/autoware_tools/issues/174>`_)
* chore: sync files (`#11 <https://github.com/autowarefoundation/autoware_tools/issues/11>`_)
  Co-authored-by: github-actions <github-actions@github.com>
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* feat(lanelet2_map_validator): check whether intersection_area satisfies vm-03-08 (`#171 <https://github.com/autowarefoundation/autoware_tools/issues/171>`_)
  * Create the framework for intersection_area_validity.
  * Made is_valid checker in intersection_area_validity
  * Split and create a new validator intersection_area_segement_type.
  * Completed intersection_area_segment_type
  * Added `vm-03-08` to autoware_requirement_set.json
  * Added `vm-03-08` to autoware_requirement_set.json
  * Added documents for intersection_area validators
  * Added `intersection_area` type polygons to sample_map.osm
  * Added test codes
  * Fixed spelling error
  * Removed original bbox calculation and use the one in the Lanelet2 library
  * Added explanation of functions
  ---------
* docs(lanelet2_map_validator): add a new document how_to_contribute.md (`#170 <https://github.com/autowarefoundation/autoware_tools/issues/170>`_)
  * Added a document how_to_contribute.md to lanelet2_map_validator
  * Added information about CMakeLists in tests.
  * Added figure illustrating the input output
  * Add a link to how_to_contribute.md to README.md
  * Fixed spelling and grammar mistakes
  * Fixed some sentences
  * Fixed spelling errors
  * Fixed link URLs
  * Quit using .. to direct to README.md
  * Fixed link mistakes
  ---------
* Fixed issue that invalid prerequisites are not reflected to the results (`#169 <https://github.com/autowarefoundation/autoware_tools/issues/169>`_)
* feat(lanelet2_map_validator): add validator to check traffic light facing (`#165 <https://github.com/autowarefoundation/autoware_tools/issues/165>`_)
  * Added valdiator missing_referrers_for_traffic_lights
  * Added validator traffic_light_facing
  * Added traffic_light_facing and missing_referrers_for_traffic_lights
  * Added new validators to README.md
  * Added test codes for traffic_light_facing and missing_referrers_for_traffic_lights
  * feat(lanelet2_map_validator): added issue codes  (`#163 <https://github.com/autowarefoundation/autoware_tools/issues/163>`_)
  * Added issue code processing
  * Revised tests for json processing
  * Added issue codes for missing_regulatory_elements_for_crosswalks
  * Added issue codes for regulatory_element_details_for_crosswalks
  * Added issue codes for missing_regulatory_elements_for_stop_lines
  * Added issue codes for missing_regulatory_elements_for_traffic_lights
  * Added issue codes for regulatory_element_details_for_traffic_lights
  * Added issue codes to docs
  * Change issue_code_prefix to append_issue_code_prefix
  * Fixed merging mistake
  ---------
  * Changed to append_issue_code_prefix.
  Added test for TrafficLight.CorrectFacing-001
  * Rearrange code structure
  * Fixed spelling mistakes
  * Fixed traffic_light_facing_procedure.svg
  * Fixed mistakes in document
  * Refine algorithm in traffic_light_facing.cpp
  * Shorten the data collection process, and updated the procedure svg
  * Use findUsages in missing_referrers_for_traffic_lights
  * Update map/autoware_lanelet2_map_validator/docs/traffic_light/missing_referrers_for_traffic_lights.md
  Co-authored-by: Mamoru Sobue <hilo.soblin@gmail.com>
  * style(pre-commit): autofix
  * Quit stocking all lanelets to collect traffic light regulatory elements
  ---------
  Co-authored-by: Mamoru Sobue <hilo.soblin@gmail.com>
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* ci(spell-check): use awf dict (`#168 <https://github.com/autowarefoundation/autoware_tools/issues/168>`_)
* feat(lanelet2_map_validator): added issue codes  (`#163 <https://github.com/autowarefoundation/autoware_tools/issues/163>`_)
  * Added issue code processing
  * Revised tests for json processing
  * Added issue codes for missing_regulatory_elements_for_crosswalks
  * Added issue codes for regulatory_element_details_for_crosswalks
  * Added issue codes for missing_regulatory_elements_for_stop_lines
  * Added issue codes for missing_regulatory_elements_for_traffic_lights
  * Added issue codes for regulatory_element_details_for_traffic_lights
  * Added issue codes to docs
  * Change issue_code_prefix to append_issue_code_prefix
  * Fixed merging mistake
  ---------
* chore: update CODEOWNERS (`#164 <https://github.com/autowarefoundation/autoware_tools/issues/164>`_)
  Co-authored-by: github-actions <github-actions@github.com>
* feat(lanelet2_map_validator): add test codes for existing validators (`#150 <https://github.com/autowarefoundation/autoware_tools/issues/150>`_)
  * Added small maps for testing.
  Added test codes using these maps.
  * Rearrange architecture of test directory.
  Added regulatory_elements_details_for_crosswalks test
  * Removed old test programs
  * Removed test_regulatory_elements_details.cpp
  * Revised test (TestRegulatoryElementDetailsForTrafficLights, MissingRefers) to look into the loading errors
  * Added sample_map for testing a normal map
  * Reflect PR comments
  * Fixed detection area in sample_map.osm
  * Added autoware namespace to test codes
  ---------
* refactor(lalenet2_map_validator): divide map loading process (`#153 <https://github.com/autowarefoundation/autoware_tools/issues/153>`_)
* refactor(lanelet2_map_validator): move custom implementation to lanelet::autoware::validation (`#152 <https://github.com/autowarefoundation/autoware_tools/issues/152>`_)
* fix(lanelet2_map_validator): change validation order in regulatory_elements_details (`#151 <https://github.com/autowarefoundation/autoware_tools/issues/151>`_)
  * Changed the order to validate in regulatory_element_details
  * Revised test code
  ---------
* Removed redundant appendIssues (`#148 <https://github.com/autowarefoundation/autoware_tools/issues/148>`_)
* feat(autoware_lanelet2_map_validator): allow prerequisites attribute for input (`#147 <https://github.com/autowarefoundation/autoware_tools/issues/147>`_)
  * Added prerequisites tag to input.
  Moved process_requirements to validation.cpp
  * Added prerequisites to autoware_requirement_set.json
  * Redefine ValidatorInfo
  * Changed check_prerequisite_completion not to read the json_data
  * Added two tests CreateValidationQueueNoCycles CreateValidationQueueWithCycles
  * Added test CheckPrerequisiteCompletionSuccess CheckPrerequisiteCompletionFailure
  * Change how to load json files in tests.
  * Added test DescriptUnusedValidatorsToJson and SummarizeValidatorResults
  * Revised README.md to the current status
  * Fixed typo and unknown words
  * Reflect PR comments
  * Fixed typo
  ---------
* chore: update CODEOWNERS (`#76 <https://github.com/autowarefoundation/autoware_tools/issues/76>`_)
  Co-authored-by: github-actions <github-actions@github.com>
* refactor(lanelet2_map_validator): move headers to include/ (`#144 <https://github.com/autowarefoundation/autoware_tools/issues/144>`_)
* chore(autoware_lanelet2_map_validator): add requirement vm-02-02 to autoware_requirement_set (`#143 <https://github.com/autowarefoundation/autoware_tools/issues/143>`_)
  * Add Sobue-san as maintainer of autoware_lanelet2_map_validator
  * Added maintainers to autoware_lanelet2_map_validator
  * Added vm-02-02 to autoware_requirement_set.json
  * Fixed error of autoware_lanelet2_map_validator template
  * Detect stop lines that are referred as `refers` role.
  ---------
* chore(autoware_lanelet2_map_validator): add maintainers (`#141 <https://github.com/autowarefoundation/autoware_tools/issues/141>`_)
  * Add Sobue-san as maintainer of autoware_lanelet2_map_validator
  * Added maintainers to autoware_lanelet2_map_validator
  ---------
* feat(autoware_lanelet2_map_validator): introduce autoware_lanelet2_map_validator (`#118 <https://github.com/autowarefoundation/autoware_tools/issues/118>`_)
  * introduce autoware_lanelet2_map_validator to autoware_tools
  * wrote description a little to README.md
  * style(pre-commit): autofix
  * Restore commented out parts.
  Removed rclcpp which is unused.
  * style(pre-commit): autofix
  * Separate validation rules to samller pieces.
  Added validation template
  * Split the validation code into smaller pieces.
  Added yaml input/output for a set of validations
  * Fixed test codes to use the separated codes
  * Removed unused code which are already divided to smaller codes.
  * Rename new_main.cpp to main.cpp
  * style(pre-commit): autofix
  * Wrote detailed README.md
  * Fixed commit mistake
  * Renew input command option to `-i` from `-r`.
  Fixed mistakes in README.md
  * style(pre-commit): autofix
  * Fixed long to uint64_t
  * Fixed spelling
  * style(pre-commit): autofix
  * Fixed typo
  * Split long lines in the code
  * style(pre-commit): autofix
  * Changed the entire structure.
  Fixed pre-commit.ci related errors.
  * style(pre-commit): autofix
  * Fixed pre-commit.ci related stuff
  * Write more details about the relationship to lanelet2_validation.
  Rewrite misleading examples.
  * Added figure of the architecture
  * Change the input/output to JSON
  * Revised architecture image of autoware_lanelet2_map_validator
  * fixed typo
  * Renew year numbers
  * Fixed dependency
  * Fixed pointed out issues
  * Improve error handling
  Refactor code style
  * Avoid clang format
  Delete unused variables
  * Removed redundant process.
  Restrict input/output format.
  * Added approaches to the documents
  * Fixed typo
  * Removed catch and improve io error handling
  * Fixed grammatical error.
  Fixed explanation of issues
  * Added stop_line validator to the table in the main README.md
  * Renamed lib to common.
  Refined CMakeLists.txt
  * Removed redundant under score
  * Removed redundant underscore again
  * Changed years.
  Removed redundant else statement.
  Removed debug comments
  * Removed underscore from test_regulatory_element_details.cpp
  ---------
  Co-authored-by: pre-commit-ci[bot] <66853113+pre-commit-ci[bot]@users.noreply.github.com>
* change to ubuntu-22.04 (`#135 <https://github.com/autowarefoundation/autoware_tools/issues/135>`_)
* chore: update CODEOWNERS (`#61 <https://github.com/autowarefoundation/autoware_tools/issues/61>`_)
  Co-authored-by: github-actions <github-actions@github.com>
* chore: update CODEOWNERS (`#12 <https://github.com/autowarefoundation/autoware_tools/issues/12>`_)
  Co-authored-by: github-actions <github-actions@github.com>
* ci(build-and-test-differential): fix fetch depth (`#55 <https://github.com/autowarefoundation/autoware_tools/issues/55>`_)
* ci(build-and-test): and differential ci fixes (`#45 <https://github.com/autowarefoundation/autoware_tools/issues/45>`_)
* chore: add templates for pull requests and issues (`#2 <https://github.com/autowarefoundation/autoware_tools/issues/2>`_)
* ci: port workflows from autoware.universe (`#1 <https://github.com/autowarefoundation/autoware_tools/issues/1>`_)
  * ci: port workflows from autoware.universe
  * fix(build_depends.repos): remove unnecessary build dependencies
  ---------
* Initial commit
* Contributors: Go Sakayori, Kosuke Takeuchi, M. Fatih Cırıt, Mamoru Sobue, Ryohsuke Mitsudome, TaikiYamada4, Yutaka Kondo, awf-autoware-bot[bot], dependabot[bot]
