#!/bin/bash
kill $(ps aux | grep '[p]lanserv' | awk '{print $2}')
kill $(ps aux | grep '[l]t-mdpsim' | awk '{print $2}')
kill $(ps aux | grep '[f]ftmpfile' | awk '{print $2}')
rm -f last_id
