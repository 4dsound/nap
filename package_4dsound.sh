# Edit these values locally
code_signature="4DSOUND Technologies BV" # MacOS code signature
notary_profile="NotaryProfile" # Notarization credentials
entitlements_file="apps/4dsound/macos/Entitlements.entitlements" # Entitlements file
upload_folder_id="1ZhnHgBQzki1stOrQZRNnF_kbSN3tbBVK" # Google drive build folder ID

# Delete settings file
rm apps/4dsound/data/settings.json

# Default settings file
cp "apps/4dsound/releasesettings.json" "apps/4dsound/data/settings.json"

# Read legacy option
if [ "$#" -gt "0" ]; then
  if [ "$1" = "-l" ]; then
    echo "Building legacy version."
    cp "apps/4dsound/legacysettings.json" "apps/4dsound/data/settings.json"
  fi
fi

if [ "$(uname)" = "Darwin" ]; then
  # MacOS

  # Check if create-dmg tool is installed
  if ! [ -x "$(command -v create-dmg)" ]; then
    echo create-dmg utility not found. To install from homebrew run:
    echo brew install create-dmg
    exit 0
  fi

  # Package and codesign
  sh package_app.sh 4dsound -b build -s "${code_signature}" -n "${notary_profile}" -E "${entitlements_file}"

  # Grab the app title and version number
  title=`jq -r '.Title' apps/4dsound/app.json`
  version=`jq -r '.Version' apps/4dsound/app.json`

  cd install

  installer_name="${title} ${version} Installer"
  dmg_filename="${title} ${version} Installer.dmg"
  app_bundle="${title}.app"

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

  echo "Uploading ${dmg_filename}..."
  sh apps/4dsound/upload.sh "install/${dmg_filename}" $upload_folder_id

elif [ "$(uname)" = "Linux" ]; then
  # Linux
  sh package_app.sh 4dsound -b build -z

  # Grab the app title and version number
  title=`jq -r '.Title' apps/4dsound/app.json`
  version=`jq -r '.Version' apps/4dsound/app.json`

  app_zip="${title} ${version} Linux.zip"
  echo "Uploading ${app_zip}..."
  sh apps/4dsound/upload.sh "install/${app_zip}" $upload_folder_id

else
  # Windows
  sh package_app.sh 4dsound -b build -z

  # Use bundled jq.exe to grab title and version
  title=`./thirdparty/jq/msvc/x86_64/jq.exe -r '.Title' apps/4dsound/app.json`
  version=`./thirdparty/jq/msvc/x86_64/jq.exe -r '.Version' apps/4dsound/app.json`

  app_zip="${title} ${version} Win.zip"
  echo "Uploading ${app_zip}..."
  sh apps/4dsound/upload.sh "install/${app_zip}" $upload_folder_id

fi

# Remove temporary settings file, if it exists
rm apps/4dsound/data/settings.json

