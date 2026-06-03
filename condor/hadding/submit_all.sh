#!/bin/bash

samples=(
    "overlay_jet10"
    "overlay_jet20"
)
for sample in "${samples[@]}"; do   
    condor_submit hadding.job \
        -a "batch_name=${sample}" 
done