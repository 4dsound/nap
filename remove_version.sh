# Removes version tags from the repository

# Check if version is specified
if [ "$#" -lt "1" ]; then
  echo "Specify a version."
  exit 0
fi

version=$1
sh git.sh "tag -d ${version}"
sh git.sh "push origin --delete #{version}"
