#!/bin/bash

pushd `dirname $0` > /dev/null
SCRIPTPATH=`pwd -P`
popd > /dev/null

user_name=$1

if [ ! -n "$user_name" ]; then
	echo "\033[38;5;148m[FATAL]: User name is mandatory!"
	echo "\033[38;5;148mExiting script"
	exit 1
fi

module_path=$SCRIPTPATH
module_name=${SCRIPTPATH##*/}
repository_name="chaos-driver-$module_name"

echo "\033[38;5;148m!CHAOS github repository creation tools to LNF-Gerrit\033[39m"
echo "\033[38;5;148mModule path -> $module_path\033[39m"
echo "\033[38;5;148mModule name -> $module_name\033[39m"
echo "\033[38;5;148mRepository name -> $repository_name\033[39m"

echo "press any key to presist repo on github server"
read -n 1 -s

echo " User password:"
stty -echo
read user_pwd
stty echo

cd $module_path

echo "Create repo on git hub"
curl -u "$user_name:$user_pwd" https://api.github.com/user/repos -d "{\"name\":\"$repository_name\"}"
if [ $? -eq 0 ]; then
	git remote add github_ssh "git@github.com:$user_name/$repository_name"
fi
