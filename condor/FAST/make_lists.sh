#!/bin/bash
for run in $(cat 42_tracking_list.txt); do
  . createlists.sh ${run}
done

