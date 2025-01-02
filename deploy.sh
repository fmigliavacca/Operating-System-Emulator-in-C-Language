#!/bin/bash

xfce4-terminal --command "bash -c 'cd memoria; make clean; make; cd bin; clear; ./memoria /home/utnso/Desktop/tp-2024-1c-BCM/memoria/memoria1.0.config'" --hold &

xfce4-terminal --command "bash -c 'cd cpu; make clean; make; cd bin; clear; ./cpu /home/utnso/Desktop/tp-2024-1c-BCM/cpu/cpu1.0.config'" --hold &

xfce4-terminal --command "bash -c 'cd kernel; make clean; make; cd bin; clear; ./kernel /home/utnso/Desktop/tp-2024-1c-BCM/kernel/kernel1.0.config'" --hold &

xfce4-terminal --command "bash -c 'cd entradasalida; make clean; make; cd bin; clear; ./entradasalida /home/utnso/Desktop/tp-2024-1c-BCM/entradasalida'" --hold & 