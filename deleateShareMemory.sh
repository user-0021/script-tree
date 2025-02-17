#!/bin/bash
shm_list=$(ipcs -m | grep " 0 ")
sem_list=$(ipcs -s | grep 0)

while read line
do
	id=$(cut -d' ' -f 2 <<<${line})
	(echo $id | xargs -i ipcrm --shmem-id {}) 
done <<END
$shm_list
END

while read line
do
	id=$(cut -d' ' -f 2 <<<${line})
	(echo $id | xargs -i ipcrm --semaphore-id {}) 
done <<END
$sem_list
END