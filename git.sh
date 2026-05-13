# Runs a git command on all repositories

# Check if command is specified
if [ "$#" -lt "1" ]; then
  echo "Specify a branch."
  exit 0
fi

command=$1

echo "nap.."
git $command # nap
echo "modules.."
cd modules
git $command # 4dsound_modules
echo "napaudioadvanced.."
cd napaudioadvanced
git $command # napaudioadvanced
cd ..
echo "audiobridge.."
cd napaudiobridge/thirdparty/audiobridge
git $command # audiobridge
cd ../../../..
echo "4dsound.."
cd apps/4dsound # 4dsound
git $command