template: gem,git,git-flow,github
lib_name: apex.rb
class_name: Apex
gem_title: apex-ruby

# Apex Ruby Gem

Ruby bindings for Apex

## File Structure

Standard Gem structure with `git flow` (Git template)

## Test Gem

@run(rake test)

## Deploy (increment:patch)

Moves `ext/apex_ext/apex_src` to the newest Apex `v*` tag before bumping
(pass `apex_tag=vX.Y.Z` to bundle a specific release). The submodule sync is
needed because Apex's cmark-gfm submodule URL can change between tags.

```run
#!/bin/bash
set -e

APEX_SRC=ext/apex_ext/apex_src
APEX_TAG="${apex_tag:latest}"

if [ -n "$(git -C "$APEX_SRC" status --porcelain --ignore-submodules=none)" ]; then
  echo "The bundled Apex source ($APEX_SRC) has local changes:" >&2
  git -C "$APEX_SRC" status --short --ignore-submodules=none >&2
  git -C "$APEX_SRC" submodule foreach --quiet --recursive \
    'git status --short --ignore-submodules=none | sed "s|^|  $displaypath: |"' >&2
  exit 1
fi

git -C "$APEX_SRC" fetch --tags --quiet origin
if [ "$APEX_TAG" = "latest" ]; then
  APEX_TAG=$(git -C "$APEX_SRC" tag --list 'v*' --sort=-v:refname | head -1)
fi
git -C "$APEX_SRC" checkout --quiet "$APEX_TAG"
git -C "$APEX_SRC" submodule sync --recursive --quiet
git -C "$APEX_SRC" submodule update --init --recursive --quiet
echo "Bundling Apex $APEX_TAG"

rake clobber test

VER=`rake bump:${increment} | sed -e 's/Bumped version to //'`
git add "$APEX_SRC"
git commit -am "v${VER} (Apex ${APEX_TAG})"
git pull
git push
```

@include(release gem)