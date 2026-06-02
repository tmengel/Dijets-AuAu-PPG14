#!/bin/bash

# Usage:
#   source setup.sh
#   source setup.sh --install=/path/to/install
#   source setup.sh --build=new
#   source setup.sh --clean
#   source setup.sh --force-build

export BUILDVER="${BUILDVER:-new}"
export INSTALLDIR="$(pwd)/install"

export sPHENIX_SETUP="/opt/sphenix/core/bin/sphenix_setup.sh"
export LOCAL_SETUP="/opt/sphenix/core/bin/setup_local.sh"

export PPG14_SRC="$(pwd)/src/dijetana"

DO_CLEAN=0
FORCE_BUILD=0
BUILDTAG="-n"

setBuild() {
  if [ -n "$1" ]; then
    export BUILDVER="$1"
  elif [ -z "$BUILDVER" ]; then
    export BUILDVER="new"
  fi

  echo "BUILDVER=$BUILDVER"
}

setInstall() {
  if [ -n "$1" ]; then
    export INSTALLDIR="$1"
  elif [ -z "$INSTALLDIR" ]; then
    echo "Error: no install directory provided"
    return 1
  fi

  export INSTALLDIR
  echo "INSTALLDIR=$INSTALLDIR"
}

srcCore() {
  if [ ! -f "$sPHENIX_SETUP" ]; then
    echo "Error: cannot find $sPHENIX_SETUP"
    return 1
  fi

  source "$sPHENIX_SETUP" -n "$BUILDVER"
}

srcLocal() {
  if [ -z "$OPT_SPHENIX" ]; then
    echo "Error: OPT_SPHENIX is not set. Did sphenix_setup.sh fail?"
    return 1
  fi

  local setup_local="$OPT_SPHENIX/bin/setup_local.sh"

  if [ ! -f "$setup_local" ]; then
    echo "Error: cannot find $setup_local"
    return 1
  fi

  if [ ! -d "$INSTALLDIR" ]; then
    echo "Error: install directory does not exist: $INSTALLDIR"
    return 1
  fi

  source "$setup_local" "$INSTALLDIR"
}

dijetanaInstalled() {
  if [ ! -d "$INSTALLDIR" ]; then
    return 1
  fi

  # Check for common install products.
  # Adjust these names if dijetana installs a differently named library/binary.
  find "$INSTALLDIR" \
    \( -name "libdijetana*.so" \
       -o -name "libdijetana*.a" \
       -o -name "*dijetana*" \) \
    -print -quit | grep -q .
}

cleanModule() {
  local modulepath="${1:-$PWD}"

  if [ ! -d "$modulepath" ]; then
    echo "Error: invalid module path: $modulepath"
    return 1
  fi

  if [ -d "$modulepath/build" ]; then
    pushd "$modulepath/build" > /dev/null || return 1
    make distclean >/dev/null 2>&1 || true
    popd > /dev/null || return 1
    rm -rf "$modulepath/build"
  fi

  local tmpfiles=(
    aclocal.m4
    autom4te.cache
    config.sub
    config.guess
    Makefile
    configure
    depcomp
    install-sh
    ltmain.sh
    Makefile.in
    missing
    compile
  )

  for file in "${tmpfiles[@]}"; do
    rm -rf "$modulepath/$file"
  done

  echo "Module cleaned in $modulepath"
}

buildModule() {
  local modulepath="${1:-$PPG14_SRC}"

  if [ ! -d "$modulepath" ]; then
    echo "Error: invalid module path: $modulepath"
    return 1
  fi

  modulepath="$(realpath "$modulepath")"

  local autogenpath="$modulepath/autogen.sh"
  if [ ! -f "$autogenpath" ]; then
    echo "Error: no autogen.sh found in $modulepath"
    return 1
  fi

  mkdir -p "$INSTALLDIR" || return 1

  if [ "$DO_CLEAN" -eq 1 ]; then
    cleanModule "$modulepath" || return 1
  fi

  srcCore || return 1

  # Source local before building if the install dir already contains dependencies.
  if [ -d "$INSTALLDIR" ]; then
    srcLocal || true
  fi

  mkdir -p "$modulepath/build" || return 1

  pushd "$modulepath/build" > /dev/null || return 1

  "$autogenpath" --prefix="$INSTALLDIR" || {
    echo "Error: autogen.sh failed"
    popd > /dev/null
    return 1
  }

  make -j20 || {
    echo "Error: make failed"
    popd > /dev/null
    return 1
  }

  make install || {
    echo "Error: make install failed"
    popd > /dev/null
    return 1
  }

  popd > /dev/null || return 1

  srcCore || return 1
  srcLocal || return 1

  echo "Built dijetana from $modulepath"
  echo "Installed to $INSTALLDIR"
}

srcPPG14() {
  setInstall "$1" || return 1
  setBuild "$2" || return 1

  srcCore || return 1

  if dijetanaInstalled && [ "$FORCE_BUILD" -eq 0 ]; then
    echo "Found existing dijetana install in $INSTALLDIR"
    srcLocal || return 1
    echo "Sourced existing dijetana installation"
    return 0
  fi

  if [ ! -d "$PPG14_SRC" ]; then
    echo "Error: dijetana source directory does not exist: $PPG14_SRC"
    return 1
  fi

  echo "No existing dijetana install found in $INSTALLDIR"
  echo "Building dijetana from $PPG14_SRC"

  buildModule "$PPG14_SRC" || return 1
}

for arg in "$@"; do
  case "$arg" in
    --install=*)
      setInstall "${arg#*=}" || return 1 2>/dev/null || exit 1
      ;;
    --build=*)
      setBuild "${arg#*=}" || return 1 2>/dev/null || exit 1
      ;;
    --clean)
      DO_CLEAN=1
      ;;
    --force-build)
      FORCE_BUILD=1
      ;;
    -n)
      BUILDTAG="-n"
      ;;
    *)
      echo "Warning: unknown option ignored: $arg"
      ;;
  esac
done

srcPPG14 "$INSTALLDIR" "$BUILDVER" || {
  echo "Error: failed to set up dijetana"
  return 1
}
