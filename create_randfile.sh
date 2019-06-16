#!/bin/bash


dd if=/dev/urandom bs=32KB count=1 | base64 > client-input.dat
