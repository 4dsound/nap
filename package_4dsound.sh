# Edit these values locally
qt_dir="~/Qt/6.6.1/macos" # QT location
code_signature="4DSOUND Technologies BV" # MacOS code signature
notary_profile="NotaryProfile" # Notarization credentials

# Check if create-dmg tool is installed
if ! [ -x "$(command -v create-dmg)" ]; then
  echo create-dmg utility not found. To install from homebrew run:
  echo brew install create-dmg
  exit 0
fi

# Delete settings file
rm apps/4dsound/data/settings.json

if [ "$(uname)" = "Darwin" ]; then
  # MacOS

  # Package and codesign
  export QT_DIR=${qt_dir}
  sh package_app.sh 4dsound build "${code_signature}" "${notary_profile}"

  # Grab the app title and version number
  title=`jq -r '.Title' apps/4dsound/app.json`
  version=`jq -r '.Version' apps/4dsound/app.json`

  cd install

  app_zip="${title} ${version} MacOS.zip"
  installer_name="${title} ${version} Installer"
  dmg_filename="${title} ${version} Installer.dmg"
  app_bundle="${title}.app"

  # Remove the zip (we will replace it with the dmg)
  rm "${app_zip}"

  # Cleaning previous disk image, if any
  echo Cleaning previous disk image...
  rm -f "${dmg_filename}"

  # Create dmg
  echo ${dmg_filename}
  echo ${app_bundle}
  create-dmg \
  --volname "${installer_name}" \
  --window-pos 200 120 \
  --window-size 800 400 \
  --icon-size 100 \
  --icon "${app_bundle}" 200 190 \
  --hide-extension "${app_bundle}" \
  --app-drop-link 600 185 \
  --codesign "${code_signature}" \
  --notarize "${notary_profile}" \
  "${dmg_filename}" \
  "${app_bundle}"
  cd ..

else
  # Windows and Linux
  set QT_DIR=${QT_DIR}
  sh package_app.sh 4dsound build
fi
