# Runs a git command on all repositories

# Check if command is specified
if [ "$#" -lt "1" ]; then
  echo "Specify a branch."
  exit 0
fi

command=$1

git $command # nap
cd modules
git $command # 4dsound_modules
cd napaudioadvanced # napaudioadvanced
git $command
cd ../napvban
git $command # napvban
cd thirdparty/vban
git $command # vban
cd ../../../..
cd apps/4dsound # 4dsound
git $command