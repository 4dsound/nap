# Delete settings file
rm apps/4dsound/data/settings.json

if [ "$(uname)" = "Darwin" ]; then
  export QT_DIR="~/Qt/6.6.1/macos"
  sh package_app.sh 4dsound build "4DSOUND Technologies BV" "NotaryProfile"
else
  sh package_app.sh 4dsound build
fi
