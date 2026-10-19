# Bumps the 4DSOUND version number in app.json and adds version tags to all repositories

# Check if version is specified
if [ "$#" -lt "1" ]; then
  echo "Specify a version."
  exit 0
fi

version=$1

# Update the version number in app.json
cd apps/4dsound
jq --arg a "${version}" '.Version = $a' app.json > "tmp" && mv "tmp" app.json
git commit app.json -m $version # commit to git
git push origin
cd ../..

# Create tags
sh git.sh "tag -a ${version} -m ${version}"
sh git.sh "push origin refs/tags/${version}"

echo In order to remove all the created version tags run:
echo sh git.sh \"tag -d [version]\"
echo sh git.sh \"push origin --delete [version]\"


