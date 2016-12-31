#!/usr/local/bin/bash
#
######################################
# Xtract & Standardize
# Files Recursively
#    feat. Camusensei irc.freenode.net
######################################
#
# USAGE
if (( $# != 2 ))
then
  printf "\nUsage: \n";
  printf "ARG1 - Directory to Extract\n";
  printf "ARG2 - Directory to Dump\n";
  exit 1;
fi
#
# ARGUMENTS
target=${1%/}
dump=${2%/}
#
# LOCATIONS
# $target = where to extract
# $dump = where to store
# $dump/meta/ = where to store file metadata
# $dump/xs_log_cp$d = log for copy errors
#
# CHECK DIR
[[ -d "$target" ]] || { echo >&2 "ERROR: you didn't specify a valid EXTRACT directory"; exit 1; }
[[ -d "$dump" ]] || { echo >&2 "ERROR: you didn't specify a valid DUMP directory"; exit 1; }
#
# BEGIN
shopt -s globstar;
for file in "$target"/**/*
do
  [[ -d "$file" ]] && continue # skipping directories
# EXTRACT METADATA
  d=$( date +%d%m%y_%H%M )
  dir=${file%/*}
  path=${dir//[ \/]/_}
  base=${file##*/}
  name=${base//[ \/]/.}
# SHA FILE
  sha="$( sha256 "$file" |  awk '{ print $NF }' )"
# EXTRACT FILE
  cp "$file" "$dump"/pool/"$sha" || echo "ERROR: couldn't cp '$file'" >> "$dump"/xs_log_cp.$d;
  printf "%s\n" "$name" >> "$dump"/meta/m"$sha";
  printf "%s\n" "$path" >> "$dump"/meta/m"$sha";
done
