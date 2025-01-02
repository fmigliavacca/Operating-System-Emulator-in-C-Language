#!/bin/bash

gnome-terminal -- bash -c 'cd memoria; make clean; make ; cd bin; clear; ./memoria'

gnome-terminal -- bash -c 'cd cpu; make clean; make ; cd bin; clear; ./cpu'

gnome-terminal -- bash -c 'cd kernel; make clean; make ; cd bin; clear; ./kernel'

gnome-terminal -- bash -c 'cd entradasalida; make clean; make ; cd bin; clear; ./entradasalida'
