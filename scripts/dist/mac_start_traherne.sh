#!/bin/sh
# Script to invoke the Traherne executable located inside Content/Resources/
# Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
# Date: March 9 2018
#
TRAHERNE_BUNDLE="`echo "$0" | sed -e 's/\/Contents\/MacOS\/Traherne//'`"
TRAHERNE_RESOURCES="$TRAHERNE_BUNDLE/Contents/Resources"

exec "$TRAHERNE_RESOURCES/Traherne"