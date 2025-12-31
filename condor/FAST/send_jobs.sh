#!/bin/bash
#for run in $(cat 42_tracking_list.txt); do
#  condor_submit -append "RunNumber=$run" condor.job
#done

for run in $(head -n 21 42_tracking_list.txt | tail -n 5); do
  condor_submit -append "RunNumber=$run" condor.job
done

