#!/bin/sh

tr -dc "A-B" < /dev/urandom | fold -w100|head -n 100000
