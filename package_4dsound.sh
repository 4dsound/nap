# Delete settings file
rm apps/4dsound/data/settings.json

if [ "$(uname)" = "Darwin" ]; then
  sh package_app.sh 4dsound build "4DSOUND Technologies BV" "NotaryProfile"
else
  sh package_app.sh 4dsound build
fi
