#!/bin/sh
ASSET_DIR=$1
TARGET_FILE=$2
echo "Packing static resources (html, css, js, etc) of imcomp as compiled C++ asset"
echo "Source: ${ASSET_DIR}"
echo "Target: ${TARGET_FILE}"

echo "// automatically exported imcomp assets" >  "${TARGET_FILE}"
echo "// source: ${ASSET_DIR}"                 >> "${TARGET_FILE}"
echo "#include \"imcomp/imcomp_asset.h\""      >> "${TARGET_FILE}"

echo "const std::map<std::string, std::string> imcomp::asset::files_ = {" >> "${TARGET_FILE}"

for file in $(find ${ASSET_DIR} -maxdepth 3 -type f -printf '%P\n')
do
  echo "Processing file "$file
  printf "\n{\"/%s\", R\"(" $file >> "${TARGET_FILE}"
  cat "${ASSET_DIR}/$file" >> "${TARGET_FILE}"
  printf ")\" }, // end of file : %s" $file >> "${TARGET_FILE}"
done
printf "\n};" >> "${TARGET_FILE}"
