#!/bin/csh

set baseDir=`pwd`

setenv HOME /sphenix/u/$LOGNAME
source /etc/csh.login
foreach i (/etc/profile.d/*.csh)
source $i
end
#source $HOME/.login

source /opt/sphenix/core/bin/sphenix_setup.csh ana.415
setenv MYINSTALL /sphenix/u/pnietomar/install
setenv LD_LIBRARY_PATH $MYINSTALL/lib:$LD_LIBRARY_PATH
setenv ROOT_INCLUDE_PATH $MYINSTALL/include:$ROOT_INCLUDE_PATH
source /opt/sphenix/core/bin/setup_local.csh $MYINSTALL

set runnumber = $1
set nevents = $2
set CALOFILE = $3
set TRACKFILES = $4
set SEEDFILES = $5

set segment = `echo $TRACKFILES | sed 's/.*-\([0-9]\+\)\.root/\1/' | awk '{printf "%05d", $0}'`

set OUTPUTFILE = "outputs/Fun4All_PhotConv_${runnumber}_${nevents}_${segment}.root"

echo "Initializing" 
echo "Run number: " ${runnumber}
echo "Events: " ${nevents}
echo "Calo file: " ${CALOFILE}
echo "Trackin tracking files: " ${TRACKFILES}
echo "Tracking seed files: " ${SEEDFILES}
echo "Output file: $OUTPUTFILE"

cd ${baseDir}
pwd

root -b -q 'Fun4All_Kshorts.C('${runnumber}', '${segment}', '${nevents}', "'${CALOFILE}'", "'${TRACKFILES}'", "'${SEEDFILES}'", "'${OUTPUTFILE}'")'




