#!/usr/bin/env bash
set -e

add-apt-repository ppa:cs50/ppa -y # for astyle 3.x
apt-get -qq update
apt-get install -y --allow-unauthenticated astyle
